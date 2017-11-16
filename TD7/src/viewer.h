#ifndef VIEWER_H
#define VIEWER_H

#include "opengl.h"
#include "shader.h"
#include "camera.h"
#include "trackball.h"
#include "mesh.h"

#include <iostream>

struct Object3D {
  Object3D(Mesh* m, Shader* s) : mesh(m), prg(s) {}
  Mesh* mesh = 0;
  Eigen::Matrix4f transformation = Eigen::Matrix4f::Identity();
  Shader* prg = 0;
  bool display = true;
};

class Viewer {
public:
  //! Constructor
  Viewer();
  virtual ~Viewer();

  // OpenGL stuff

  /// Automatically called to redraw the scene
  void display();

  /// Automatically called when the window is resized
  void reshape(int w, int h);

  /// Call this function to reload the shaders from files
  void loadProgram();

  /// Called once to initialize the scene
  virtual void init(int w, int h, int argc, char **argv);

  /// Called
  virtual void updateScene();

  // events
  virtual void mousePressed(GLFWwindow* window, int button, int action, int mods);
  virtual void mouseMoved(int x, int y);
  virtual void mouseScroll(double x, double y);
  virtual void keyEvent(int key, int action, int mods);
  virtual void charPressed(int key);

protected:
  void setObjectMatrix(const Eigen::Matrix4f &M, Shader& prg) const;


protected:
  int _winWidth, _winHeight;

  Camera _cam;

  // the default shader program
  Shader _main_shader;
  int _texid;

  // list of meshes to display
  std::vector<Object3D> _meshes;

  // Mouse parameters for the trackball
  enum TrackMode
  {
    TM_NO_TRACK=0, TM_ROTATE_AROUND, TM_ZOOM,
    TM_LOCAL_ROTATE, TM_FLY_Z, TM_FLY_PAN
  };
  TrackMode _trackingMode = TM_NO_TRACK;
  Trackball _trackball;
  Eigen::Vector2i _lastMousePos;

  bool _disp_selection = true;
  bool _wireframe = false;
  bool _texturing = false;
  bool _coldwarm_shading = true;
};

#endif
