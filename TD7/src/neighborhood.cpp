
#include "mesh.h"
#include "neighborhood.h"
#include <stack>
#include <vector>

using namespace surface_mesh;
using namespace Eigen;
using namespace std;

std::vector<Surface_mesh::Vertex> select_neighbors(const Surface_mesh& mesh, Surface_mesh::Vertex v, float dist)
{
  std::vector<Surface_mesh::Vertex> neighbors;

  neighbors.push_back(v);

  Vector3f q = mesh.position(v);
  std::vector<bool> visited(mesh.n_vertices(), false);
  visited[v.idx()] = true;
  std::stack<Surface_mesh::Vertex> stack;
  stack.push(v);
  while(!stack.empty())
  {
    Surface_mesh::Vertex vj = stack.top();
    stack.pop();

    // pour tous les voisins vi de vj
    Surface_mesh::Halfedge h = mesh.halfedge(vj), h0 = h;
    do
    {
      Surface_mesh::Vertex vi = mesh.to_vertex(h);
      if(!visited[vi.idx()]) // si on ne l'a pas déjà rencontré
      {
        if( (mesh.position(vi)-q).norm() < dist ) // si on n'est pas sortie de la boule
        {
          neighbors.push_back(vi);
          stack.push(vi);
        }
      }
      visited[vi.idx()] = true;
      h = mesh.next_halfedge(mesh.opposite_halfedge(h));
    } while (h!=h0);
  }

  return neighbors;
}
