#ifndef _QUAD_H_
#define _QUAD_H_

#include "shape.h"

class Quad : public Shape
{
public:
    Quad();
    virtual ~Quad();

    void init();

    //! id must be in [0..3]
    void setTexCoord(unsigned int id, float x, float y);

    //! id must be in [0..3]
    void setPos(unsigned int id, float x, float y);

    void display(Shader *shader);

private:
    float mTexCoord[8];
    float mPos[8];
    float mNormal[12];
    float mColor[12];
    unsigned int mIndices[6];

    GLuint _vao;
    GLuint _vbo[5];
};

#endif
