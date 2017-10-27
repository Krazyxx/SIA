#include "fbo.h"
#include <SOIL2.h>

void FBO::init(int width, int height)
{
    _width = width;
    _height = height;

    //1. generate a framebuffer object and bind it
    glGenFramebuffers(1, &_fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboId);

    //2. init textures
    glGenTextures(2, renderedTexture); // On créer un FBO
    for (size_t i = 0; i < 2; i++) {
        // Bind the newly created texture
        glBindTexture(GL_TEXTURE_2D, renderedTexture[i]);

        // Give an empty image to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,     GL_TEXTURE_WRAP_S, (GLint)GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,     GL_TEXTURE_WRAP_T, (GLint)GL_CLAMP_TO_EDGE);

        //3. attach the texture to FBO color attachment point
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, renderedTexture[i], 0);
    }

    //4. init a depth buffer as a texture (in order to use it inside shaders afterward)
    // Similar to step 2 with a depth specific texture
    //glGenTextures(1, &depthTexture);
    //glBindTexture(GL_TEXTURE_2D, depthTexture);

    //5. attach the depth buffer to FBO depth attachment point
    glGenRenderbuffers(1, &depthTexture);
    glBindTexture(GL_RENDERBUFFER, depthTexture);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, _width, _height);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthTexture);

    //6. Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

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

// id allows to select the render (color or normal)
void FBO::savePNG(const std::string &name, int id)
{
    glBindTexture(GL_TEXTURE_2D, renderedTexture[id]);
    GLubyte *data = new GLubyte [4 * _width * _height];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    unsigned char* raw_data = reinterpret_cast<unsigned char*>(data);

    /*	mirror image vertically */
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

    SOIL_save_image((DATA_DIR"/" + name+".png").c_str(),SOIL_SAVE_TYPE_PNG,_width,_height,4,raw_data);
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
