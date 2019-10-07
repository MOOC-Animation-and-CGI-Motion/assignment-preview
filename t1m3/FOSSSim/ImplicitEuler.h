#ifndef __IMPLICIT_EULER__
#define __IMPLICIT_EULER__

#include <Eigen/Dense>
#include <iostream>

#include "SceneStepper.h"

class ImplicitEuler : public SceneStepper
{
public:
  ImplicitEuler();
  
  virtual ~ImplicitEuler();
  
  virtual bool stepScene( TwoDScene& scene, scalar dt );
  
  virtual std::string getName() const;
};

#endif
