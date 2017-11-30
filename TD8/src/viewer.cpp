#include "viewer.h"
#include "camera.h"

#include "SOIL2.h"

extern int WIDTH;
extern int HEIGHT;

using namespace Eigen;

Viewer::Viewer()
{
}

Viewer::~Viewer()
{
}

////////////////////////////////////////////////////////////////////////////////
// GL stuff

// initialize OpenGL context
void Viewer::init(int w, int h){
    _winWidth = w;
    _winHeight = h;

    glClearColor(1.f,1.f,1.f,1.f);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    loadProgram();

    _psys.init();
    _psys.forces.push_back(&_mouseForce);

    _cam.setSceneCenter(_psys.boundingBox().center());
    _cam.setSceneRadius(_psys.boundingBox().sizes().maxCoeff());
    _cam.setSceneDistance(_cam.sceneRadius() * 3.f);
    _cam.setMinNear(0.1f);
    _cam.setNearFarOffsets(-_cam.sceneRadius() * 100.f,
                            _cam.sceneRadius() * 100.f);
    _cam.setScreenViewport(AlignedBox2f(Vector2f(0.0,0.0), Vector2f(w,h)));
}

void Viewer::reshape(int w, int h){
    _winWidth = w;
    _winHeight = h;
    _cam.setScreenViewport(AlignedBox2f(Vector2f(0.0,0.0), Vector2f(w,h)));
    glViewport(0, 0, w, h);
}


/*!
   callback to draw graphic primitives
 */
void Viewer::display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _simplePrg.activate();
    glUniformMatrix4fv(_simplePrg.getUniformLocation("projection_matrix"), 1, GL_FALSE, _cam.computeProjectionMatrix().data());
    glUniformMatrix4fv(_simplePrg.getUniformLocation("view_matrix"), 1, GL_FALSE, _cam.computeViewMatrix().data());
    glUniformMatrix4fv(_simplePrg.getUniformLocation("model_matrix"), 1, GL_FALSE, _psys.getTransformationMatrix().data());
    Vector3f col = Vector3f(0.f,0.f,0.f);
    glUniform3fv(_simplePrg.getUniformLocation("col"), 1, col.data());
    _psys.display(&_simplePrg);
    _simplePrg.deactivate();
}


void Viewer::updateScene()
{
    // Measure speed
    double currentTime = glfwGetTime();
    while(glfwGetTime() - currentTime < 0.03) {
        _psys.step(0.0001);
    }

    display();
}

void Viewer::loadProgram()
{
    _blinnPrg.loadFromFiles(DATA_DIR"/shaders/blinn.vert", DATA_DIR"/shaders/blinn.frag");
    _simplePrg.loadFromFiles(DATA_DIR"/shaders/simple.vert", DATA_DIR"/shaders/simple.frag");
    checkError();
}

////////////////////////////////////////////////////////////////////////////////
// Events

/*!
   callback to manage mouse : called when user press or release mouse button
   You can change in this function the way the user
   interact with the system.
 */
void Viewer::mousePressed(GLFWwindow *window, int button, int action, int mods)
{
    if(action == GLFW_PRESS) {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if(mods == GLFW_MOD_CONTROL) {
                Matrix4f proj4 = _cam.computeProjectionMatrix();
                Matrix4f view4 = _cam.computeViewMatrix();
                Matrix4f C = view4.inverse();
                Matrix3f proj3;
                proj3 << proj4.topLeftCorner<2,3>(), proj4.bottomLeftCorner<1,3>();
                Vector2f proj_pos = Vector2f(2.f*float(_lastMousePos[0] + 0.5f)/float(_winWidth) - 1.0,
                        -(2.f*float(_lastMousePos[1] + 0.5f)/float(_winHeight) - 1.0));
                // find nearest particle
                double dmin = 0.2; // ignore particles farther than 0.2
                for (int i = 0; i < _psys.particles.size(); i++) {
                    Particle *p = _psys.particles[i];
                    Vector4f pos;
                    pos << p->x.cast<float>(), 1.f;
                    Vector4f pos_p = (proj4 * view4 * pos);
                    Vector3f dir;
                    dir <<  pos_p.head<2>() / pos_p[3] - proj_pos, 0;
                    double d = dir.head<2>().norm();
                    if (d < dmin) {
                         _mouseForce.x = p->x - (C.topLeftCorner<3,3>() * (proj3.inverse() * dir)).cast<double>();
                         _mouseForce.p = p;
                         dmin = d;
                    }
                }
                _mod = true;
            } else {
                _cam.startRotation(_lastMousePos);
            }
        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            _cam.startTranslation(_lastMousePos);
        }
        _button = button;
    }else if(action == GLFW_RELEASE) {
        if(_button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if(mods == GLFW_MOD_CONTROL) {
                _mouseForce.p = nullptr;
                _mod = false;
            }else{
                _cam.endRotation();
            }
        }
        else if(_button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            _cam.endTranslation();
        }
        _button = -1;
    }
}


/*!
   callback to manage mouse : called when user move mouse with button pressed
   You can change in this function the way the user
   interact with the system.
 */
void Viewer::mouseMoved(int x, int y)
{
    if(_button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(_mod)
        {
            Matrix4f proj4 = _cam.computeProjectionMatrix();
            Matrix4f view4 = _cam.computeViewMatrix();
            Matrix4f C = view4.inverse();
            Matrix3f proj3;
            proj3 << proj4.topLeftCorner<2,3>(), proj4.bottomLeftCorner<1,3>();

            Vector2f proj_pos = Vector2f(2.f*float(x + 0.5f)/float(_winWidth) - 1.0,
                    -(2.f*float(y + 0.5f)/float(_winHeight) - 1.0));

            Particle *p = _mouseForce.p;
            if(p) {
                Vector4f pos;
                pos << p->x.cast<float>(), 1.f;
                Vector4f pos_p = (proj4 * view4 * pos);
                Vector3f dir;
                dir <<  pos_p.head<2>()/ pos_p[3] - proj_pos.head<2>(), 0;
                _mouseForce.x = p->x - (C.topLeftCorner<3,3>() * (proj3.inverse() * dir)).cast<double>();
            }
        } else {
            _cam.dragRotate(Vector2f(x,y));
        }
    }
    else if(_button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        _cam.dragTranslate(Vector2f(x,y));
    }
    _lastMousePos = Vector2f(x,y);
}

void Viewer::mouseScroll(double x, double y)
{
    _cam.zoom((y>0)? 1.1: 1./1.1);
}

/*!
   callback to manage keyboard interactions
   You can change in this function the way the user
   interact with the system.
 */
void Viewer::keyPressed(int key, int action, int mods)
{
    if(key == GLFW_KEY_R && action == GLFW_PRESS)
        loadProgram();
}

void Viewer::charPressed(int key)
{
}
