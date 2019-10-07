#ifndef CONTEST_DETECTOR_H
#define CONTEST_DETECTOR_H

#include "CollisionDetector.h"
#include <vector>
#include <set>

typedef std::set<std::pair<int, int> > PPList;
typedef std::set<std::pair<int, int> > PEList;
typedef std::set<std::pair<int, int> > PHList;

class ContestDetector : public CollisionDetector
{
 public:
  ContestDetector() {}

  virtual void performCollisionDetection(const TwoDScene &scene, const VectorXs &qs, const VectorXs &qe, DetectionCallback &dc);

 private:
  void findCollidingPairs(const TwoDScene &scene, const VectorXs &x, PPList &pppairs, PEList &pepairs, PHList &phpairs);
};

#endif
