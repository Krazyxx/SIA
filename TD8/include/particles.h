#ifndef PARTICLES_H
#define PARTICLES_H

#include "integration.h"
#include "shape.h"
#include <Eigen/Dense>
#include <vector>

using namespace Eigen;

class Particle;
class Force;

class ParticleSystem: public ODESystem, public Shape {
public:
    ~ParticleSystem();
    void init();          // initialize the system
    void step(double dt); // perform a time step of length dt
    void display(Shader *shader); // draw everything
    // ODE functions, see integration.h
    int getDimensions();
    void getState(VectorXd &state);
    void setState(const VectorXd &state);
    void getDerivative(VectorXd &deriv);

    std::vector<Particle*> particles;
    std::vector<Force*> forces;

private:
    void updateGL(bool first);

    unsigned int _vao[2];
    unsigned int _vbo[2];
    uint nbSpringForces;
};

class Particle {
public:
    Vector3d x; // position
    Vector3d v; // velocity
    Vector3d f; // force
    double m;   // mass
    Particle(double m, Vector3d x, Vector3d v): m(m), x(x), v(v) {}
};

class Force {
public:
     virtual ~Force() {}
    virtual void addForces() = 0;
};

class GravityForce : public Force {
public:
    virtual ~GravityForce() {}
    ParticleSystem *ps; // apply gravity to all particles
    Vector3d g;         // gravity
    GravityForce(ParticleSystem *ps, const Vector3d &g): ps(ps), g(g) {}
    void addForces();
};

class DragForce: public Force {
public:
    virtual ~DragForce() {}
    ParticleSystem *ps; // apply drag to all particles
    double kd;          // drag coefficient
    DragForce(ParticleSystem *ps, double kd): ps(ps), kd(kd) {}
    void addForces();
};

class SpringForce: public Force {
    // connects two particles by a spring
public:
    virtual ~SpringForce() {}
    Particle *p0, *p1; // particles
    double ks, kd;     // spring constant, damping coefficient
    double l0;         // rest length
    SpringForce(Particle *p0, Particle *p1, double ks, double kd, double l0):
        p0(p0), p1(p1), ks(ks), kd(kd), l0(l0) {}
    void addForces();
};

class AnchorForce: public Force {
    // attaches a particle to a fixed point by a spring
public:
    virtual ~AnchorForce() {}
    Particle *p;   // particle
    Vector3d x;    // point to anchor it to
    double ks, kd; // spring constant, damping coefficient
    AnchorForce(Particle *p, Vector3d x, double ks, double kd):
        p(p), x(x), ks(ks), kd(kd) {}
    void addForces();
};

#endif
