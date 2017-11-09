#include "fbo.h"
#include <SOIL2.h>
#include <assert.h>

FBO::~FBO()
{
    clear();
}

void FBO::clear()
{
    glDeleteTextures(2, renderedTexture);
    glDeleteFramebuffers(1, &_fboId);
}

void FBO::init(int width, int height)
{
    _width = width;
    _height = height;

    //1. create a framebuffer object
    glGenFramebuffers(1, &_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, _fboId);

    //2. init texture
    glGenTextures(2, renderedTexture);

    for(int i=0; i<2; ++i){
        // Bind the newly created texture
        glBindTexture(GL_TEXTURE_2D, renderedTexture[i]);

        // Give an empty image to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //3. attach the texture to FBO color attachment point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, renderedTexture[i], 0);
    }

    //4. init a depth buffer as a texture (to use in a shader afterward)
    glGenTextures(1, &depthTexture);

    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //5. attach the depth buffer to FBO depth attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    //6. Set the list of draw buffers.
    GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, DrawBuffers); // "2" is the size of DrawBuffers

    //7. check FBO status
    checkFBOAttachment();

    //8. switch back to original framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void FBO::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fboId);
}

void FBO::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::savePNG(const std::string &name, int i)
{
    assert(i<2);
    glBindTexture(GL_TEXTURE_2D, renderedTexture[i]);
    GLubyte *data = new GLubyte [4 * _width * _height];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    unsigned char* raw_data = reinterpret_cast<unsigned char*>(data);

    /*  mirror image vertically */
    for(int j = 0; j*2 < _height; ++j )
    {
        int index1 = j * _width * 4;
        int index2 = (_height - 1 - j) * _width * 4;
        for(int i = _width * 4; i > 0; --i )
        {
            unsigned char temp = raw_data[index1];
            raw_data[index1] = raw_data[index2];
            raw_data[index2] = temp;
            ++index1;
            ++index2;
        }
    }

    SOIL_save_image((DATA_DIR"/" + name).c_str(),SOIL_SAVE_TYPE_PNG,_width,_height,4,raw_data);
    glBindTexture(GL_TEXTURE_2D,0);
}

void FBO::checkFBOAttachment()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "An attachment could not be bound to frame buffer object!" << std::endl;
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << "Attachments are missing! At least one image (texture) must be bound to the frame buffer object!" << std::endl;
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << "A Draw buffer is incomplete or undefinied. All draw buffers must specify attachment points that have images attached." << std::endl;
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << "A Read buffer is incomplete or undefinied. All read buffers must specify attachment points that have images attached." << std::endl;
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        std::cout << "All images must have the same number of multisample samples." << std::endl;
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
        std::cout << "If a layered image is attached to one attachment, then all attachments must be layered attachments. The attached layers do not have to have the same number of layers, nor do the layers have to come from the same kind of texture." << std::endl;
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "Attempt to use an unsupported format combinaton!" << std::endl;
        break;
    default:
        std::cout << "Unknown error while attempting to create frame buffer object!" << std::endl;
        break;
    }
}
