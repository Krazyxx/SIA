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
    // a) Calculer une itération d'Euler
    VectorXd deriv(system->getDimensions());
    system->getDerivative(deriv);
    VectorXd dx(system->getDimensions());
    dx = dt * deriv;

    // b) Évaluer la dérivée à mi-distance
    VectorXd state(system->getDimensions());
    system->getState(state);
    VectorXd fmid(system->getDimensions());
    fmid = (state + dx) / 2.f;
    system->setState(fmid);

    // c) L'utiliser pour mettre à jour x
    VectorXd newState(system->getDimensions());
    newState = state + dt * deriv;
    system->setState(newState);
}
