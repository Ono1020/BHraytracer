#include <iostream>
#include <vector>
#include <omp.h>

#include "common.h"
#include "Vec3.h"
#include "Ray.h"

int main()
{
  const int imageWidth = 1000;
  const int imageHeight = 1000;
  // const int imageWidth = 100;
  // const int imageHeight = 100;
  std::vector<Vec3> colors(imageHeight * imageWidth);

  std::cout << "P3\n"
            << imageWidth << ' ' << imageHeight << "\n255\n";

  Vec3 cameraPos(0.0, -10.0, 1.0);
  Vec3 lookat(0.0, 0.0, 0.0);
  Vec3 cameraDir = unitVector(lookat - cameraPos);
  Vec3 vup(0.0, 0.0, 1.0);
  const double focalLength = 2.0;
  const double pov = 60 * M_PI / 180;
  const double aspectRatio = 1.0;
  // const double aspectRatio = 16.0 / 9.0;
  double cameraW = focalLength * tan(pov / 2.0) * 2; // camera width
  double cameraH = cameraW * aspectRatio;            // camera height
  Vec3 h = unitVector(cross(cameraDir, vup));        // horizon
  Vec3 v = unitVector(cross(cameraDir, h));          // vertical

  Vec3 pos;
  Vec3 dir;
  Ray ray;

  Vec3 basePoint = cameraPos - (cameraW * h) - (cameraH * v);
  Vec3 unitH = cameraW * h / imageWidth * 2;
  Vec3 unitV = cameraH * v / imageHeight * 2;

  int completed_pixels = 0;
  int total_pixels = imageWidth * imageHeight;

#pragma omp parallel for private(pos, dir, ray)
  for (int i = 0; i < imageHeight; i++)
  {
    for (int j = 0; j < imageWidth; j++)
    {
      pos = basePoint + j * unitH + i * unitV;
      dir = unitVector(pos - (cameraPos - focalLength * cameraDir));

      ray = Ray(cameraPos - focalLength * cameraDir, dir);
      ray.mainLoop();

      colors[i * imageWidth + j] = ray.color;

#pragma omp atomic
      completed_pixels++;
    }

    if (omp_get_thread_num() == 0)
    {
#pragma omp critical(progress_output)
      {
        double progress = (double)completed_pixels / total_pixels * 100.0;
        std::cerr << "\rProgress: " << progress << "% " << std::flush;
      }
    }
  }

  for (auto &y : colors)
  {
    std::cout << y.e[0] << " " << y.e[1] << " " << y.e[2] << "\n";
  }

  std::cerr << "\nDone.\n";

  return 0;
}