#include "quad.h"

#include <iostream>

using namespace std;

Quad::Quad()
{
    //default initialization
    setTexCoord(0, 0.f, 1.f);
    setPos     (0,-1.f,-1.f);

    setTexCoord(1, 1.f, 1.f);
    setPos     (1, 1.f,-1.f);

    setTexCoord(2, 1.f, 0.f);
    setPos     (2, 1.f, 1.f);

    setTexCoord(3, 0.f, 0.f);
    setPos     (3,-1.f, 1.f);

    // specification of the triangles indices
    mIndices[0] = 0;
    mIndices[1] = 1;
    mIndices[2] = 3;

    mIndices[3] = 1;
    mIndices[4] = 2;
    mIndices[5] = 3;

    _bbox.extend(Eigen::Vector3f(-1,-1,0));
    _bbox.extend(Eigen::Vector3f(1,1,0));

    for(int i = 0; i <12; ++i){
        mColor[i] = 0.75f;
        if(i%3==2)
            mNormal[i] = 1.f;
        else
            mNormal[i] = 0.0f;
    }
}

Quad::~Quad()
{
    if(_ready) {
        glDeleteBuffers(5, _vbo);
        glDeleteVertexArrays(1,&_vao);
    }
}

void Quad::init()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(5, _vbo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, mIndices,  GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, mPos, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, mTexCoord, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, mNormal, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, mColor, GL_STATIC_DRAW);

    glBindVertexArray(0);

    _ready = true;
}


void Quad::setTexCoord(unsigned int id, float x, float y)
{
    if (id > 3)
        cerr << "Warning: Invalid texture coordinates" << endl;

    mTexCoord[2*id  ] = x;
    mTexCoord[2*id+1] = y;
}


void Quad::setPos(unsigned int id, float x, float y)
{
    if (id > 3)
        cerr << "Warning: Invalid vertex coordinates" << endl;

    mPos[2*id  ] = x;
    mPos[2*id+1] = y;
}


void Quad::display(Shader *shader)
{
    if (!_ready)
        init();

    glBindVertexArray(_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);

    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glEnableVertexAttribArray(vertex_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
        glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int texCoord_loc = shader->getAttribLocation("vtx_texcoord");
    if(texCoord_loc>=0){
        glEnableVertexAttribArray(texCoord_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
        glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int normal_loc = shader->getAttribLocation("vtx_normal");
    if(normal_loc>=0){
        glEnableVertexAttribArray(normal_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[3]);
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int color_loc = shader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glEnableVertexAttribArray(color_loc);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[4]);
        glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if(vertex_loc>=0)
        glDisableVertexAttribArray(vertex_loc);
    if(texCoord_loc>=0)
        glDisableVertexAttribArray(texCoord_loc);
    if(color_loc>=0)
        glDisableVertexAttribArray(color_loc);
    if(normal_loc>=0)
        glDisableVertexAttribArray(normal_loc);

    glBindVertexArray(0);
}
