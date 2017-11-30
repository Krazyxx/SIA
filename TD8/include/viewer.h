#ifndef VIEWER_H
#define VIEWER_H

#include "opengl.h"
#include "shader.h"
#include "camera.h"
#include "particles.h"

#include <iostream>

class Viewer{
public:
    //! Constructor
    Viewer();
    virtual ~Viewer();

    // gl stuff
    void init(int w, int h);
    void display();
    void updateScene();
    void reshape(int w, int h);
    void loadProgram();

    // events
    void mousePressed(GLFWwindow* window, int button, int action, int mods);
    void mouseMoved(int x, int y);
    void mouseScroll(double x, double y);
    void keyPressed(int key, int action, int mods);
    void charPressed(int key);

private:
    int _winWidth, _winHeight;

    Camera _cam;

    Shader _simplePrg, _blinnPrg;

    ParticleSystem _psys;
    AnchorForce _mouseForce = AnchorForce(nullptr, Vector3d(0,0,0), 1000, 1);

    // Mouse parameters
    Eigen::Vector2f _lastMousePos;
    int _button = -1;
    bool _mod = false;
};

#endif
