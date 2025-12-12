#ifndef _COMMON_H_
#define _COMMON_H_

#include <cmath>

const int rBH = 1;
const double worldSize = 30;
const double dLambda = 0.01;

const double rAccretionIn = 3.0 * rBH; // 降着円盤の内径 (3倍シュワルツシルト半径)
const double rAccretionOut = 8.0;      // 降着円盤の外径 (適当な大きな値)
const double thicknessTheta = 0.05;    // 赤道面からの許容誤差(直行座標系で)

#endif /* _COMMON_H_ */