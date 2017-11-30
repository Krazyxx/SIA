#ifndef MESH_H
#define MESH_H

#include "shape.h"
#include "ray.h"
#include "quad.h"

#include <surface_mesh/surface_mesh.h>

#include <string>
#include <vector>
#include <Eigen/SparseCore>

class BVH;

class Mesh : public Shape
{
    /** Represents a vertex of the mesh */
    struct Vertex
    {
        Vertex()
            : position(Eigen::Vector3f::Constant(0)), color(Eigen::Vector3f::Constant(0)), normal(Eigen::Vector3f::Constant(0)),
              texcoord(Eigen::Vector2f::Constant(0)), mask(0)
        {}
        Vertex(const Eigen::Vector3f& pos)
            : position(pos), color(Eigen::Vector3f::Constant(0)), normal(Eigen::Vector3f::Constant(0)),
              texcoord(Eigen::Vector2f::Constant(0)), mask(0)
        {}
        Vertex(const Eigen::Vector3f& pos, const Eigen::Vector3f& c, const Eigen::Vector3f& n, const Eigen::Vector2f& tex)
            : position(pos), color(c), normal(n), texcoord(tex), mask(0)
        {}

        Eigen::Vector3f position;
        Eigen::Vector3f color;
        Eigen::Vector3f normal;
        Eigen::Vector2f texcoord;
        int mask;
    };

public:
    Mesh() : _bvh(0) {}
    virtual ~Mesh();
    void load(const std::string& filename);
    void init();
    void display(Shader *shader);

    /// returns a reference to the surface_mesh data structure
    surface_mesh::Surface_mesh& halfEdgeMesh() { return _halfEdgeMesh; }

    /// returns a const reference to the surface_mesh data structure
    const surface_mesh::Surface_mesh& halfEdgeMesh() const { return _halfEdgeMesh; }

    /// returns face indices as a 3xN matrix of integers
    Eigen::Map<Eigen::Matrix<unsigned int,3,Eigen::Dynamic> > faceIndices()
    { return Eigen::Map<Eigen::Matrix<unsigned int,3,Eigen::Dynamic> >(_indices.data(), 3, nbFaces()); }

    // Accessors to vertex attributes as Eigen's matrices:
    Eigen::Map<Eigen::Matrix3Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> > positions()
    { return Eigen::Map<Eigen::Matrix3Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> >(_vertices[0].position.data(), 3, _vertices.size()); }

    Eigen::Map<Eigen::Matrix3Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> > colors()
    { return Eigen::Map<Eigen::Matrix3Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> >(_vertices[0].color.data(), 3, _vertices.size()); }

    Eigen::Map<Eigen::Matrix3Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> > normals()
    { return Eigen::Map<Eigen::Matrix3Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> >(_vertices[0].normal.data(), 3, _vertices.size()); }

    Eigen::Map<Eigen::Matrix2Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> > texcoords()
    { return Eigen::Map<Eigen::Matrix2Xf,0,Eigen::OuterStride<sizeof(Vertex)/sizeof(float)> >(_vertices[0].texcoord.data(), 2, _vertices.size()); }

    Eigen::Map<Eigen::VectorXi,0,Eigen::InnerStride<sizeof(Vertex)/sizeof(float)> > masks()
    { return Eigen::Map<Eigen::VectorXi,0,Eigen::InnerStride<sizeof(Vertex)/sizeof(float)> >(&_vertices[0].mask, _vertices.size()); }

    /// Computes the Laplacian matrix in matrix \a L using cotangent weights or the graph Laplacian if useCotWeights==false.
    void create_laplacian_matrix(Eigen::SparseMatrix<float>& L, bool useCotWeights = true) const;

    /// Computes the permutation putting selected vertices (mask==1) first, and the others at the end.
    /// It returns the number of selected vertices.
    int create_permutation(Eigen::PermutationMatrix<Eigen::Dynamic> & perm) const;

    /// Performs the poly-harmonic interpolation (order k) over the selected vertices (mask==1) of the vertex attributes u.
    /// For each vertex V of index i,
    ///     if  V.mask!=1 then u.col(i) is used as input constraints,
    ///     otherwise, V.mask==1, and u.col(i) is replaced by the poly-harmonic interpolation of the fixed values.
    void poly_harmonic_interpolation(Eigen::Ref<Eigen::MatrixXf> u, int k=1) const;

    void updateAll();

    /// Re-compute vertex normals (needs to be called after editing vertex positions)
    void updateNormals();

    /// Re-compute the aligned bounding box (needs to be called after editing vertex positions)
    void updateBoundingBox();

    /// Re-compute the BVH for fast ray-mesh intersections (needs to be called after editing vertex positions)
    void updateBVH();

    /// Copy vertex attributes from the CPU to GPU memory (needs to be called after editing any vertex attributes: positions, normals, texcoords, masks, etc.)
    void updateVBO();


    /// computes the first intersection between the ray and the mesh in hit (if any)
    bool intersect(const Ray& ray, Hit& hit) const;

    // For BVH:
    /// \returns  the number of faces
    int nbFaces() const { return _halfEdgeMesh.n_faces(); }

    /// \returns a const references to the \a vertexId -th vertex of the \a faceId -th face. vertexId must be between 0 and 2 !!
    const Vertex& vertexOfFace(int faceId, int vertexId) const { return _vertices[_indices[faceId*3+vertexId]]; }

    /** compute the intersection between a ray and a given triangular face */
    bool intersectFace(const Ray& ray, Hit& hit, int faceId) const;




private:

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;

    surface_mesh::Surface_mesh _halfEdgeMesh;

    BVH *_bvh;

    GLuint _vao;
    GLuint _vbo[2];
};


#endif // MESH_H
