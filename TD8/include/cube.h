#ifndef _CUBE_H
#define _CUBE_H

#include "shape.h"

class Cube : public Shape
{
public:
    Cube();
    virtual ~Cube();

    void init();
    void display(Shader *shader);

private:
    int _vbo[3];
    int _vao;
};

#endif
