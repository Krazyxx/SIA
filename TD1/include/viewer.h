#ifndef VIEWER_H
#define VIEWER_H

#include "trackball.h"
#include "scene.h"
#include "block.h"

#include <nanogui/screen.h>
#include <nanogui/glutil.h>

class Viewer : public nanogui::Screen
{
    // A scene contains a list of objects, a list of light sources and a camera.
    Scene* m_scene = nullptr;
    
    // GLSL shader programs
    nanogui::GLShader m_program;
    nanogui::GLShader m_flatProgram;
    nanogui::GLShader m_tonemapProgram;

    // OpenGL camera
    Camera m_GLCamera;
    bool m_drawCamera;

    Ray m_ray;
    Hit m_hit;
    bool m_drawRay;

    bool m_drawAABB;

    enum TrackMode {
      TM_NO_TRACK=0, TM_ROTATE_AROUND, TM_ZOOM,
      TM_LOCAL_ROTATE, TM_FLY_Z, TM_FLY_PAN
    };

    TrackMode m_currentTrackingMode;
    Eigen::Vector2i m_mouseCoords;
    Trackball m_trackball;

    ImageBlock* m_resultImage = nullptr;
    std::string m_curentFilename;
    bool m_renderingDone;

    nanogui::GLFramebuffer m_fbo;

    // GUI
    nanogui::Slider *m_slider = nullptr;
    nanogui::Button* m_button = nullptr;
    nanogui::CheckBox *m_checkbox = nullptr;
    nanogui::Widget *m_panel = nullptr;
    uint32_t m_texture = 0;
    float m_scale = 1.f;
    int m_srgb = 1;

  protected:
    void initializeGL();

    /** This method is automatically called everytime the opengl windows is resized. */
    virtual void framebufferSizeChanged();

    /** This method is automatically called everytime the opengl windows has to be refreshed. */
    virtual void drawContents();

    /** This method is automatically called everytime a key is pressed */
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

    /** This method is called when files are dropped on the window */
    virtual bool dropEvent(const std::vector<std::string> &filenames);

    /** These methods are automatically called everytime the mouse is used */
    virtual bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers);
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers);
    virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2f &rel);

    /** This method selects on point in the scene by casting a ray */
    void select(const Point2i &point);

  public: 
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    /** This method load a 3D scene from a file */
    void loadScene(const std::string &filename);

    /** This method load an OpenEXR image from a file */
    void loadImage(const filesystem::path &filename);

    // default constructor
    Viewer();
    ~Viewer();
};

#endif // VIEWER_H

