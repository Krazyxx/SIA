
#include "mesh.h"
#include "laplacian.h"
#include <Eigen/SparseCholesky>

using namespace Eigen;
using namespace surface_mesh;

typedef SparseMatrix<float> SpMat;
typedef PermutationMatrix<Dynamic> Permutation;
typedef Eigen::Triplet<double> T;

double cotan_weight(const Surface_mesh& mesh, Surface_mesh::Halfedge he)
{
    auto points = mesh.get_vertex_property<Point>("v:point");

    Surface_mesh::Vertex pi  = mesh.from_vertex(he);                          // p(i)
    Surface_mesh::Vertex pi1 = mesh.to_vertex(mesh.ccw_rotated_halfedge(he)); // p(j-1)
    Surface_mesh::Vertex pj  = mesh.to_vertex(he);                            // p(j)
    Surface_mesh::Vertex pj1 = mesh.to_vertex(mesh.cw_rotated_halfedge(he));  // p(j+1)

    Vector3f vi1_i = (points[pi] - points[pi1]).normalized();
    Vector3f vi1_j = (points[pj] - points[pi1]).normalized();
    Vector3f vj1_i = (points[pi] - points[pj1]).normalized();
    Vector3f vj1_j = (points[pj] - points[pj1]).normalized();

    float cos_alpha = vi1_j.dot(vi1_j);
    float sin_alpha = vi1_j.dot(vi1_j);
    float cos_beta  = vi1_j.dot(vi1_j);
    float sin_beta  = vi1_j.dot(vi1_j);

    float cotan_alpha = cos_alpha / sin_alpha;
    float cotan_beta = cos_beta / sin_beta;

    return (cotan_alpha + cotan_beta) / 2.f;
}

/// Computes the Laplacian matrix in matrix \a L using cotangent weights or the graph Laplacian if useCotWeights==false.
void create_laplacian_matrix(const Surface_mesh& mesh, SpMat& L, bool useCotWeights)
{
    // number of vertices in mesh
    int n = mesh.n_vertices();
    std::vector<T> tripletList;
    tripletList.reserve(n);

    Surface_mesh::Vertex_iterator vit;
    for (vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit) {
        // halfedge circulator
        float wii = 0.f;
        Surface_mesh::Vertex vi = *vit;

        Surface_mesh::Vertex_around_vertex_circulator vj, vj_end;
        vj = mesh.vertices(*vit);
        vj_end = vj;
        do {
            float wij = useCotWeights ? cotan_weight(mesh, vj.halfedge()) : 1.0; // need to compute the weight
            tripletList.push_back(T(vi.idx(), (*vj).idx(), wij));
            wii += wij;
        } while(++vj != vj_end);
        tripletList.push_back(T(vi.idx(), vi.idx(), -wii));
    }

    L.setFromTriplets(tripletList.begin(), tripletList.end());
}

/// Computes the permutation putting selected vertices (mask==1) first, and the others at the end.
/// It returns the number of selected vertices.
int create_permutation(const Surface_mesh& mesh, Permutation& perm)
{
    auto masks = mesh.get_vertex_property<int>("v:mask");

    // number of vertices in mesh
    int n = mesh.n_vertices();
    perm.resize(n);

    int nb_unknowns = 0, nb_knowns = n-1;
    Surface_mesh::Vertex_iterator vit;
    for (vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit) {
        Surface_mesh::Vertex vi = *vit;
        if (masks[vi] == 1) {
            perm.indices()[vi.idx()] = nb_unknowns;
            nb_unknowns++;
        } else {
            perm.indices()[vi.idx()] = nb_knowns;
            nb_knowns--;
        }
    }

    return nb_unknowns;
}


/// Performs the poly-harmonic interpolation (order k) over the selected vertices (mask==1) of the vertex attributes u.
/// For each vertex V of index i,
///     if  mask[V]!=1 then u.col(i) is used as input constraints,
///     otherwise, mask[V}==1, and u.col(i) is replaced by the poly-harmonic interpolation of the fixed values.
void poly_harmonic_interpolation(const Surface_mesh& mesh, Ref<MatrixXf> u, int k)
{
    // Number of vertices in the mesh
    int n = mesh.n_vertices();

    // 1 - Create the sparse Laplacian matrix
    SpMat L(n,n);
    create_laplacian_matrix(mesh, L, true);
    SpMat Ltmp = SpMat(L);
    for (int i = 1; i < k; ++i) {
        L = L * Ltmp;
    }

    // 2 - Create the permutation matrix putting the fixed values at the end,
    //     and the true unknown at the beginning
    Permutation perm;
    int nb_unknowns = create_permutation(mesh, perm);

    // 3 - Apply the permutation to both rows (equations) and columns (unknowns),
    //     i.e., L = P * L * P^-1
    // multiplication à gauche par P : modification des colonnes
    //                  droite       : modification des lignes
    L = L.twistedBy(perm); // Equivalent to L = perm * L * perm.inverse() but faster

    SpMat L00 = L.topLeftCorner(nb_unknowns, nb_unknowns);
    SpMat L01 = L.topRightCorner(nb_unknowns, n - nb_unknowns);

    // 4 - solve L * [x^T u^T]^T = 0, i.e., L00 x = - L01 * u
    MatrixXf u_prime = perm * u.transpose();


    SimplicialLDLT<SpMat> solver;
    solver.compute(L00);
    u_prime.topRows(nb_unknowns) = solver.solve(- L01 * u_prime.bottomRows(n - nb_unknowns));

    // 5 - Copy back the results to u
    u_prime  = perm.inverse() * u_prime;
    u = u_prime.transpose();
}
