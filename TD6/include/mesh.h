#ifndef MESH_H
#define MESH_H

#include "shape.h"
#include "quad.h"

#include <surface_mesh/surface_mesh.h>

#include <string>
#include <vector>

class Mesh : public Shape
{
public:
    Mesh() {}
    virtual ~Mesh();
    void load(const std::string& filename);
    void init();
    void display(Shader *shader);

    Mesh* computeShadowVolume(const Eigen::Vector3f &lightPos);

protected:
    void specifyVertexData(Shader *shader);
    void addQuad(Mesh* mesh, const Eigen::Vector4f &p0, const Eigen::Vector4f &p1, const Eigen::Vector4f &p2, const Eigen::Vector4f &p3);

private:
    /** Represents a vertex of the mesh */
    struct Vertex
    {
        Vertex()
            : position(Eigen::Vector4f::Constant(0)), color(Eigen::Vector4f::Constant(0)), normal(Eigen::Vector3f::Constant(0)),
              texcoord(Eigen::Vector2f::Constant(0))
        {}
        Vertex(const Eigen::Vector4f& pos)
            : position(pos), color(Eigen::Vector4f::Constant(0)), normal(Eigen::Vector3f::Constant(0)),
              texcoord(Eigen::Vector2f::Constant(0))
        {}
        Vertex(const Eigen::Vector4f& pos, const Eigen::Vector4f& c, const Eigen::Vector3f& n, const Eigen::Vector2f& tex)
            : position(pos), color(c), normal(n), texcoord(tex)
        {}
        Vertex(const Eigen::Vector4f& pos, const Eigen::Vector4f& c, const Eigen::Vector3f& n,
               const Eigen::Vector3f& t, const Eigen::Vector3f& b, const Eigen::Vector2f& tex)
            : position(pos), color(c), normal(n), tangent(t), bitangent(b), texcoord(tex)
        {}
        Eigen::Vector4f position;
        Eigen::Vector4f color;
        Eigen::Vector3f normal;
        Eigen::Vector3f tangent;
        Eigen::Vector3f bitangent;
        Eigen::Vector2f texcoord;
    };

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;

    surface_mesh::Surface_mesh _halfEdgeMesh;

    GLuint _vao;
    GLuint _vbo[2];
};


#endif // MESH_H
