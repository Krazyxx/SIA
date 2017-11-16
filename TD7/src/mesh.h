#ifndef MESH_H
#define MESH_H

#include "ray.h"

#include <iostream>
#include <surface_mesh/surface_mesh.h>

#include <string>
#include <vector>
#include <Eigen/Core>
#include "opengl.h"

class BVH;
class Shader;

class Mesh : public surface_mesh::Surface_mesh
{
    using Vector3f = Eigen::Vector3f;
    using Vector2f = Eigen::Vector2f;

public:
    Mesh() : _ready(false), _bvh(0) {}
    ~Mesh();

    /// Loads the mesh from file
    void load(const std::string& filename);

    /// Creates a mesh representing a uniform paramteric grids with m x n vertices over the [0,1]^2 range.
    void createGrid(int m, int n);

    void init();

    /// Draws the mesh using OpenGL
    void draw(const Shader &shader);

    /// returns face indices as a 3xN matrix of integers
    Eigen::Map<Eigen::Matrix<int,3,Eigen::Dynamic> > faceIndices()
    { return Eigen::Map<Eigen::Matrix<int,3,Eigen::Dynamic> >(_indices.data(), 3, n_faces()); }

    Eigen::Map<const Eigen::Matrix<int,3,Eigen::Dynamic> > faceIndices() const
    { return Eigen::Map<const Eigen::Matrix<int,3,Eigen::Dynamic> >(_indices.data(), 3, n_faces()); }

    // Accessors to vertex attributes as Eigen's matrices:
    Eigen::Map<Eigen::Matrix3Xf> positions()
    {
      auto& vertices = get_vertex_property<Vector3f>("v:point").vector();
      return Eigen::Map<Eigen::Matrix3Xf>(vertices[0].data(), 3, vertices.size());
    }

    Eigen::Map<Eigen::Matrix3Xf> colors()
    {
      auto& vcolors = get_vertex_property<Vector3f>("v:color").vector();
      return Eigen::Map<Eigen::Matrix3Xf>(vcolors[0].data(), 3, vcolors.size());
    }

    Eigen::Map<Eigen::Matrix3Xf> normals()
    {
      auto& vnormals = get_vertex_property<Vector3f>("v:normal").vector();
      return Eigen::Map<Eigen::Matrix3Xf>(vnormals[0].data(), 3, vnormals.size());
    }

    Eigen::Map<Eigen::Matrix2Xf> texcoords()
    {
      auto& texcoords = get_vertex_property<Vector2f>("v:texcoord").vector();
      return Eigen::Map<Eigen::Matrix2Xf>(texcoords[0].data(), 2, texcoords.size());
    }

    Eigen::Map<Eigen::VectorXi> masks()
    {
      auto& masks = get_vertex_property<int>("v:mask").vector();
      return Eigen::Map<Eigen::VectorXi>(masks.data(), masks.size());
    }

    /// Call all update* functions below.
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

    /** compute the intersection between a ray and a given triangular face */
    bool intersectFace(const Ray& ray, Hit& hit, int faceId) const;

    const Eigen::AlignedBox3f& boundingBox() const { return _bbox; }


private:

    bool _ready;
    Eigen::AlignedBox3f _bbox;

    std::vector<int> _indices;

    BVH *_bvh;

    GLuint _vao;
    const int VBO_IDX_FACE = 0;
    const int VBO_IDX_POSITION = 1;
    const int VBO_IDX_NORMAL = 2;
    const int VBO_IDX_COLOR = 3;
    const int VBO_IDX_TEXCOORD = 4;
    const int VBO_IDX_MASK = 5;
    GLuint _vbo[6];
};


#endif // MESH_H
