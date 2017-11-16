

#ifndef TD2_H
#define TD2_H

#include <surface_mesh/surface_mesh.h>

// Returns the list of all vertices connected to v within a distance 'dist'
std::vector<surface_mesh::Surface_mesh::Vertex> select_neighbors(const surface_mesh::Surface_mesh& mesh, surface_mesh::Surface_mesh::Vertex v, float dist);


#endif // TD2_H
