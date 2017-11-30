#ifndef _SPHERE_H
#define _SPHERE_H

#include "shape.h"
#include <vector>

class Sphere : public Shape {

public:
    Sphere(float radius=1.f, int nU=40, int nV=40);
    virtual ~Sphere();
    void init();
    void display(Shader *shader);
    float radius() const { return _radius; }

private:
    GLuint _vao;
    GLuint _vbo[6];

    std::vector<int>                _indices;   /** vertex indices */
    std::vector<Eigen::Vector3f>	_vertices;  /** 3D positions */
    std::vector<Eigen::Vector3f>	_colors;    /** colors */
    std::vector<Eigen::Vector3f>	_normals;   /** 3D normals */
    std::vector<Eigen::Vector3f>	_tangents;  /** 3D tangent to surface */
    std::vector<Eigen::Vector2f>	_texCoords; /** 2D texture coordinates */

    float _radius;
};

#endif
