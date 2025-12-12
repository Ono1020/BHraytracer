#include "Vec3.h"

Vec3::Vec3()
{
  this->e[0] = 0.0;
  this->e[1] = 0.0;
  this->e[2] = 0.0;
}

Vec3::Vec3(double e0, double e1, double e2)
{
  this->e[0] = e0;
  this->e[1] = e1;
  this->e[2] = e2;
}

void Vec3::displayInfo()
{
  std::cout << e[0] << ", ";
  std::cout << e[1] << ", ";
  std::cout << e[2] << std::endl;

  return;
}