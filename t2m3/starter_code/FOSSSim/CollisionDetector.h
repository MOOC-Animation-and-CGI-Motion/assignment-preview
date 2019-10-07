#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include "MathDefs.h"

class TwoDScene;

class DetectionCallback
{
 public:
  virtual void ParticleParticleCallback(int idx1, int idx2)=0;

  virtual void ParticleEdgeCallback(int vidx, int eidx)=0;

  virtual void ParticleHalfplaneCallback(int vidx, int hidx)=0;
};

class CollisionDetector
{
 public:
  virtual void performCollisionDetection(const TwoDScene &scene, const VectorXs &qs, const VectorXs &qe, DetectionCallback &dc) = 0;

};


#endif
