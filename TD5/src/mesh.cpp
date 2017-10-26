#include "mesh.h"
#include "meshloader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

using namespace std;
using namespace Eigen;

Mesh::~Mesh()
{
    if(_ready){
        glDeleteBuffers(2, _vbo);
        glDeleteVertexArrays(1,&_vao);
    }
}


void Mesh::load(const string& filename)
{
    vector<Vector3f> positionTmp;
    vector<Vector3i> faceId;
    vector<Vector3f> normalTmp;
    vector<Vector3i> normalId;
    vector<Vector2f> texcoordTmp;
    vector<Vector3i> texId;
    vector<Vector4f> colorTmp;

    cout << "Load " << filename << endl;

    try{
        std::string ext = filename.substr(filename.size()-3,3);
        if(ext=="off" || ext=="OFF")
            MeshLoader::loadOFF(filename, positionTmp, faceId, colorTmp);
        else if(ext=="obj" || ext=="OBJ")
            MeshLoader::loadObj(filename, positionTmp, faceId, normalTmp, normalId, texcoordTmp, texId);
        else
            cerr << "Mesh: extension \'" << ext << "\' not supported." << endl;
    }catch(MeshLoaderException e){
        cerr << e.getMessage();
        return;
    }

    if(normalId.size() == 0) // vertex normals not available, compute them from the faces normal
    {
        // pass 1: set the normal to 0
        normalTmp.resize(positionTmp.size(),Vector3f(0.f,0.f,0.f));

        // pass 2: compute face normals and accumulate
        for(unsigned int i = 0; i < faceId.size(); i++)
        {
            Vector3f v0 = positionTmp[(faceId[i])[0]];
            Vector3f v1 = positionTmp[(faceId[i])[1]];
            Vector3f v2 = positionTmp[(faceId[i])[2]];

            Vector3f n = (v1-v0).cross(v2-v0);

            normalTmp[(faceId[i])[0]] += n;
            normalTmp[(faceId[i])[1]] += n;
            normalTmp[(faceId[i])[2]] += n;
        }

        // pass 3: normalize
        for(vector<Vector3f>::iterator n_iter = normalTmp.begin() ; n_iter!=normalTmp.end() ; ++n_iter)
            (*n_iter).normalize();

        normalId = faceId;
    }

    Vector3f pos;
    Vector2f tex;
    Vector3f norm;
    Vector4f color = Vector4f::Constant(0.5f);
    unsigned int size = faceId.size();
    _vertices.reserve(3*size);

    for(unsigned int i = 0; i < size; i++){
        for (unsigned int j=0; j <3; j++){
            pos = positionTmp[(faceId[i])[j]];
            if(i < texId.size())
                tex = texcoordTmp[(texId[i])[j]];
            if(i < normalId.size())
                norm = normalTmp[(normalId[i])[j]];
            if(colorTmp.size() == positionTmp.size())
                color = colorTmp[(faceId[i])[j]];

            _vertices.push_back(Vertex(Vector4f(pos[0],pos[1],pos[2],1),color,norm,tex));
            _indices.push_back(3*i + j);
            _bbox.extend(pos);
        }
    }
}

void Mesh::init()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(2, _vbo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _indices.size(), _indices.data(),  GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*_vertices.size(), _vertices[0].position.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    _ready = true;
}

void Mesh::display(Shader *shader)
{
    if (!_ready)
        init();

    glBindVertexArray(_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);

    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0) {
        glVertexAttribPointer(vertex_loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(vertex_loc);
    }

    int color_loc = shader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector4f)));
        glEnableVertexAttribArray(color_loc);
    }

    int normal_loc = shader->getAttribLocation("vtx_normal");
    if(normal_loc>=0){
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector4f)));
        glEnableVertexAttribArray(normal_loc);
    }

    int tangent_loc = shader->getAttribLocation("vtx_tangent");
    if(tangent_loc>=0){
        glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector4f)));
        glEnableVertexAttribArray(tangent_loc);
    }

    int bitangent_loc = shader->getAttribLocation("vtx_bitangent");
    if(bitangent_loc>=0){
        glVertexAttribPointer(bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector4f)));
        glEnableVertexAttribArray(bitangent_loc);
    }

    int texCoord_loc = shader->getAttribLocation("vtx_texcoord");
    if(texCoord_loc>=0){
        glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector3f)+2*sizeof(Vector4f)));
        glEnableVertexAttribArray(texCoord_loc);
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
    if(bitangent_loc>=0)
        glDisableVertexAttribArray(bitangent_loc);
    if(texCoord_loc>=0)
        glDisableVertexAttribArray(texCoord_loc);

    glBindVertexArray(0);
}
