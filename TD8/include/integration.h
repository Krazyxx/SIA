#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <Eigen/Dense>
using namespace Eigen;

class ODESystem {
    // parent class for anything that can be represented
    // as a system of ordinary differential equations
    // (see Sec. 4 of "Differential Equation Basics" in Witkin and Baraff)
public:
    // return number of degrees of freedom
    virtual int getDimensions() = 0;
    // write state into vectors
    virtual void getState(VectorXd &state) = 0;
    // read state from vectors
    virtual void setState(const VectorXd &state) = 0;
    // write derivative of state
    virtual void getDerivative (VectorXd &deriv) = 0;
};

// perform an explicit Euler step of length dt
void explicitEulerStep (ODESystem *system, double dt);

void midPointStep (ODESystem *system, double dt);

#endif
