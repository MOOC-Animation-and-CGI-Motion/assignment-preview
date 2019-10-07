#ifndef __LINEARIZED_IMPLICIT_EULER__
#define __LINEARIZED_IMPLICIT_EULER__

#include <Eigen/Dense>
#include <iostream>

#include "SceneStepper.h"

class LinearizedImplicitEuler : public SceneStepper
{
public:
  LinearizedImplicitEuler();
  
  virtual ~LinearizedImplicitEuler();
  
  virtual bool stepScene( TwoDScene& scene, scalar dt );
  
  virtual std::string getName() const;
};

#endif
