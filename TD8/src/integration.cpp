#include "integration.h"
#include <iostream>
using namespace std;

void explicitEulerStep (ODESystem *system, double dt) {
    VectorXd state(system->getDimensions());
    VectorXd deriv(system->getDimensions());

    system->getState(state);
    system->getDerivative(deriv);

    VectorXd newState(system->getDimensions());
    newState = state + dt * deriv;

    system->setState(newState);
}

void midPointStep (ODESystem *system, double dt) {
    // TODO: implement this
}
