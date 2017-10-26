#include "sphere.h"

using namespace Eigen;

Sphere::Sphere(float radius, int nU, int nV) :
    _radius(radius)
{
    int nVertices  = (nU + 1) * (nV + 1);
    int nTriangles =  nU * nV * 2;

    _vertices.resize(nVertices);
    _normals.resize(nVertices);
    _tangents.resize(nVertices);
    _texCoords.resize(nVertices);
    _colors.resize(nVertices);
    _indices.resize(3*nTriangles);

    for(int v=0;v<=nV;++v)
    {
        for(int u=0;u<=nU;++u)
        {

            float theta = u / float(nU) * M_PI;
            float phi 	= v / float(nV) * M_PI * 2;

            int index 	= u +(nU+1)*v;

            Vector3f vertex, tangent, normal;
            Vector2f texCoord;

            // normal
            normal[0] = sin(theta) * cos(phi);
            normal[1] = sin(theta) * sin(phi);
            normal[2] = cos(theta);
            normal.normalize();

            // position
            vertex = normal * _radius;

            // tangent
            theta += M_PI_2;
            tangent[0] = sin(theta) * cos(phi);
            tangent[1] = sin(theta) * sin(phi);
            tangent[2] = cos(theta);
            tangent.normalize();

            // texture coordinates
            texCoord[1] = u / float(nU);
            texCoord[0] = v / float(nV);

            _vertices[index] = vertex;
            _normals[index] = normal;
            _tangents[index] = tangent;
            _texCoords [index] = texCoord;
            _colors[index] = Vector3f(0.6f,0.2f,0.8f);
            _bbox.extend(vertex);
        }
    }

    int index = 0;
    for(int v=0;v<nV;++v)
    {
        for(int u=0;u<nU;++u)
        {
            int vindex 	= u + (nU+1)*v;

            _indices[index+0] = vindex;
            _indices[index+1] = vindex+1 ;
            _indices[index+2] = vindex+1 + (nU+1);

            _indices[index+3] = vindex;
            _indices[index+4] = vindex+1 + (nU+1);
            _indices[index+5] = vindex   + (nU+1);

            index += 6;
        }
    }
}

Sphere::~Sphere()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(6, _vbo);
}

void Sphere::init()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(6, _vbo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _indices.size(), _indices.data(),  GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * _colors.size(), _colors.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * _normals.size(), _normals.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f) * _tangents.size(), _tangents.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2f) * _texCoords.size(), _texCoords.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    _ready = true;
}

void Sphere::display(Shader *shader)
{
    if (!_ready)
        init();

    glBindVertexArray(_vao);

     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);

    int vertex_loc = shader->getAttribLocation("vtx_position");
    glEnableVertexAttribArray(vertex_loc);
    if(vertex_loc>=0){
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int color_loc = shader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glEnableVertexAttribArray(color_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
        glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int normal_loc = shader->getAttribLocation("vtx_normal");
    if(normal_loc>=0){
        glEnableVertexAttribArray(normal_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[3]);
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int tangent_loc = shader->getAttribLocation("vtx_tangent");
    if(tangent_loc>=0){
        glEnableVertexAttribArray(tangent_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[4]);
        glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int texCoord_loc = shader->getAttribLocation("vtx_texcoord");
    if(texCoord_loc>=0){
        glEnableVertexAttribArray(texCoord_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[5]);
        glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawElements(GL_TRIANGLES, _indices.size(),  GL_UNSIGNED_INT, 0);

    if(vertex_loc>=0)
        glDisableVertexAttribArray(vertex_loc);
    if(color_loc>=0)
        glDisableVertexAttribArray(color_loc);
    if(normal_loc>=0)
        glDisableVertexAttribArray(normal_loc);
    if(tangent_loc>=0)
        glDisableVertexAttribArray(tangent_loc);
    if(texCoord_loc>=0)
        glDisableVertexAttribArray(texCoord_loc);


    glBindVertexArray(0);
}
