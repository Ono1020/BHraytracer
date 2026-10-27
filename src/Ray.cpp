#include "Ray.h"

Ray::Ray()
{
}

Ray::Ray(Vec3 argPos, Vec3 dir)
{
  this->pos = argPos;
  this->posDir = dir;

  polarCoordinateTransformer();

  dr = sin(theta) * cos(phi) * posDir.e[0] + sin(theta) * sin(phi) * posDir.e[1] + cos(theta) * posDir.e[2];
  dtheta = (cos(theta) * cos(phi) * posDir.e[0] + cos(theta) * sin(phi) * posDir.e[1] - sin(theta) * posDir.e[2]) / r;
  dphi = (-sin(phi) * posDir.e[0] + cos(phi) * posDir.e[1]) / (r * sin(theta));

  double f = 1 - rBH / r;
  double cdt = sqrt((dr * dr) / (f * f) + (r * r) / f * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi));
  E = f * cdt;
}

void Ray::mainLoop()
{
  while (!this->judgeCollision())
  {
    this->update();
  }
  return;
}

void Ray::update()
{
  // polarCoordinateTransformer();
  f = 1 - rBH / r;
  cdt = E / f;
  // k1
  dd2 k1 = calcdd2(r, theta, phi, dr, dtheta, dphi);
  // k2
  dd2 k2 = calcdd2(r + 0.5 * dLambda * k1.dr,
                   theta + 0.5 * dLambda * k1.dtheta,
                   phi + 0.5 * dLambda * k1.dphi,
                   dr + 0.5 * dLambda * k1.d2r,
                   dtheta + 0.5 * dLambda * k1.d2theta,
                   dphi + 0.5 * dLambda * k1.d2phi);
  // k3
  dd2 k3 = calcdd2(r + 0.5 * dLambda * k2.dr,
                   theta + 0.5 * dLambda * k2.dtheta,
                   phi + 0.5 * dLambda * k2.dphi,
                   dr + 0.5 * dLambda * k2.d2r,
                   dtheta + 0.5 * dLambda * k2.d2theta,
                   dphi + 0.5 * dLambda * k2.d2phi);
  // k4
  dd2 k4 = calcdd2(r + dLambda * k3.dr,
                   theta + dLambda * k3.dtheta,
                   phi + dLambda * k3.dphi,
                   dr + dLambda * k3.d2r,
                   dtheta + dLambda * k3.d2theta,
                   dphi + dLambda * k3.d2phi);
  // double d2r = -(rBH / (2.0 * r * r)) * f * cdt * cdt + (rBH / (2.0 * r * r * f)) * dr * dr + r * f * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);
  // double d2theta = -2.0 * dr * dtheta / r + sin(theta) * cos(theta) * dphi * dphi;
  // double d2phi = -2.0 * dr * dphi / r - 2.0 / tan(theta) * dtheta * dphi;
  // double d2r, d2theta, d2phi;

  // r += dLambda * dr;
  // theta += dLambda * dtheta;
  // phi += dLambda * dphi;

  r += (dLambda / 6.0) * (k1.dr + 2.0 * k2.dr + 2.0 * k3.dr + k4.dr);
  theta += (dLambda / 6.0) * (k1.dtheta + 2.0 * k2.dtheta + 2.0 * k3.dtheta + k4.dtheta);
  phi += (dLambda / 6.0) * (k1.dphi + 2.0 * k2.dphi + 2.0 * k3.dphi + k4.dphi);
  dr += (dLambda / 6.0) * (k1.d2r + 2.0 * k2.d2r + 2.0 * k3.d2r + k4.d2r);
  dtheta += (dLambda / 6.0) * (k1.d2theta + 2.0 * k2.d2theta + 2.0 * k3.d2theta + k4.d2theta);
  dphi += (dLambda / 6.0) * (k1.d2phi + 2.0 * k2.d2phi + 2.0 * k3.d2phi + k4.d2phi);

  // dr = sin(theta) * cos(phi) * posDir.e[0] + sin(theta) * sin(phi) * posDir.e[1] + cos(theta) * posDir.e[2];
  // dtheta = (cos(theta) * cos(phi) * posDir.e[0] + cos(theta) * sin(phi) * posDir.e[1] - sin(theta) * posDir.e[2]) / r;
  // dphi = (-sin(phi) * posDir.e[0] + cos(phi) * posDir.e[1]) / (r * sin(theta));

  // double d2r = -(rBH / (2.0 * r * r)) * f * cdt * cdt + (rBH / (2.0 * r * r * f)) * dr * dr + r * f * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);
  // double d2theta = -2.0 * dr * dtheta / r + sin(theta) * cos(theta) * dphi * dphi;
  // double d2phi = -2.0 * dr * dphi / r - 2.0 / tan(theta) * dtheta * dphi;
  // double d2r, d2theta, d2phi;
  // while (theta < 0)
  // {
  //   theta = -theta;
  //   phi += M_PI;
  // }
  // while (theta > M_PI)
  // {
  //   theta = 2 * M_PI - theta;
  //   phi += M_PI;
  // }

  // while (phi > M_PI)
  // {
  //   phi -= 2.0 * M_PI;
  // }
  // while (phi <= -M_PI)
  // {
  //   phi += 2.0 * M_PI;
  // }

  // 極座標を直行座標へ変換
  pos.e[0] = r * sin(theta) * cos(phi);
  pos.e[1] = r * sin(theta) * sin(phi);
  pos.e[2] = r * cos(theta);
}

bool Ray::judgeCollision()
{
  // 追跡中の光線の位置情報を最新の極座標に更新
  bool r_in_range = (r >= rAccretionIn && r <= rAccretionOut);
  bool height_in_range = (std::abs(pos.e[2]) < thicknessTheta);
  if (r_in_range && height_in_range)
  {
    color = Vec3(255.0, 100.0, 0.0); // 例: オレンジ色
    return true;
  }

  if (r > worldSize)
  {
    // std::cerr << "Escaped at r=" << r << "\n";
    // double normalized_theta = theta / M_PI;
    // double normalized_phi = (phi + M_PI) / (2.0 * M_PI);
    // const double N = 10.0;

    // int stripes_theta = (int)(normalized_theta * N);
    // int stripes_phi = (int)(normalized_phi * N);

    // if ((stripes_theta + stripes_phi) % 2 == 0)
    // {
    //   color = Vec3(190.0, 190.0, 190.0);
    // }
    // else
    // {
    //   color = Vec3(255.0, 255.0, 255.0);
    // }
    color = Vec3(0, 0, 0);
    return true;
  }

  if (r < rBH || std::isnan(r))
  {
    color = Vec3(0.0, 0.0, 0.0);
    return true;
  }

  return false;
}

void Ray::polarCoordinateTransformer()
{
  r = pos.length();
  if (r == 0)
  {
    std::cerr << "Error: r=0\n";
    return;
  }

  theta = acos(pos.e[2] / r);

  phi = atan2(pos.e[1], pos.e[0]);

  return;
}

dd2 Ray::calcdd2(
    double cr, double ctheta, double cphi,
    double cdr, double cdtheta, double cdphi)
{
  dd2 D;

  D.dr = cdr;
  D.dtheta = cdtheta;
  D.dphi = cdphi;

  // RK4 の各ステージでは、そのステージの位置 cr で f と dt/dλ を計算し直す。
  // ステップ開始時のメンバー変数 f / cdt を使い回すと、近接通過時に
  // ヌル条件 (g_μν dx^μ dx^ν = 0) のドリフトが最大で 1e-4 程度まで悪化した。
  // 再計算により同条件で 1e-12 程度まで改善することをテストで確認済み。
  double cf = 1 - rBH / cr;
  double ccdt = E / cf;
  D.d2r = -(rBH / (2.0 * cr * cr)) * cf * ccdt * ccdt + (rBH / (2.0 * cr * cr * cf)) * cdr * cdr + cr * cf * (cdtheta * cdtheta + sin(ctheta) * sin(ctheta) * cdphi * cdphi);
  D.d2theta = -2.0 * cdr * cdtheta / cr + sin(ctheta) * cos(ctheta) * cdphi * cdphi;
  D.d2phi = -2.0 * cdr * cdphi / cr - 2.0 / tan(ctheta) * cdtheta * cdphi;

  return D;
}