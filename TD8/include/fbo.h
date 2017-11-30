#ifndef VBO_H
#define VBO_H

#include "opengl.h"

class FBO
{
public:
    ~FBO();
    void init(int width, int height);
    void clear();
    void bind();
    void unbind();
    void savePNG(const std::string& name, int i);
    void checkFBOAttachment();
    int width() const { return _width; }
    int height() const { return _height; }
    GLuint id() { return _fboId; }

    GLuint renderedTexture[2];
    GLuint depthTexture;

private:
    GLuint _fboId;
    int _width, _height;
};

#endif // FBO_H
