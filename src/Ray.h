#ifndef _RAY_H_
#define _RAY_H_

#include <iostream>
#include <cmath>

#include "common.h"
#include "Vec3.h"

struct dd2
{
  double dr, dtheta, dphi;
  double d2r, d2theta, d2phi;
};

class Ray
{
public:
  double R, G, B;
  Vec3 pos;
  Vec3 posDir;
  Vec3 color;
  double r, theta, phi;
  double dr, dtheta, dphi;
  double lambda;
  double E;
  double f;
  double cdt;

  Ray();
  Ray(Vec3 pos, Vec3 dir);

  dd2 calcdd2(
      double cr, double ctheta, double cphi,
      double cdr, double cdtheta, double cdphi);

  void mainLoop();
  void polarCoordinateTransformer();
  bool judgeCollision();
  void update();
};

#endif /* _RAY_H_ */