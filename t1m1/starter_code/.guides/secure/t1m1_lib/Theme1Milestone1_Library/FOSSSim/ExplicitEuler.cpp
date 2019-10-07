#include "ExplicitEuler.h"

ExplicitEuler::ExplicitEuler()
: SceneStepper()
{}

ExplicitEuler::~ExplicitEuler()
{}

std::string ExplicitEuler::getName() const
{
    return "Explicit Euler";
}
