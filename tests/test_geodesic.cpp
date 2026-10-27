// 測地線積分器の物理検証テスト
//
// このリポジトリの Ray クラス (Schwarzschild 測地線の RK4 積分) が
// 解析的に知られた結果と一致するかを確認する回帰テスト。
// Vulkan/GLSL への移植時には、移植先の出力とこのテストの期待値を比べる
// 「物差し」として使うことを想定している。
//
// 単位系: このコードではシュバルツシルト半径 rBH = 1 なので M = 0.5。
// 終端判定は judgeCollision() を使わず (円盤との衝突を含むため)、
// テスト側で r の範囲だけを見て行う。光線は赤道面 (z=0) 内で、
// y 方向にオフセットして発射する。
//
// ビルドと実行: リポジトリのルートで `make test`

#include <cmath>
#include <cstdio>

#include "../src/Ray.h"
#include "../src/Vec3.h"

namespace {

int failures = 0;

void check(const char *name, bool ok, const char *detail) {
  std::printf("[%s] %s: %s\n", ok ? "PASS" : "FAIL", name, detail);
  if (!ok) failures++;
}

const double M = 0.5; // rBH = 2M = 1

// 球面座標の微分から直交座標の進行方向を復元する (大きさは不問)
Vec3 cartDir(Ray &ray) {
  double st = sin(ray.theta), ct = cos(ray.theta);
  double sp = sin(ray.phi), cp = cos(ray.phi);
  Vec3 er(st * cp, st * sp, ct);
  Vec3 et(ct * cp, ct * sp, -st);
  Vec3 ep(-sp, cp, 0.0);
  return ray.dr * er + (ray.r * ray.dtheta) * et +
         (ray.r * st * ray.dphi) * ep;
}

double nullNorm(Ray &ray) {
  double f = 1 - rBH / ray.r;
  double cdt = ray.E / f;
  return -f * cdt * cdt + ray.dr * ray.dr / f +
         ray.r * ray.r * (ray.dtheta * ray.dtheta +
                          sin(ray.theta) * sin(ray.theta) * ray.dphi * ray.dphi);
}

double angMomentum(Ray &ray) {
  return ray.r * ray.r * sin(ray.theta) * sin(ray.theta) * ray.dphi;
}

// x=-100 から +x 方向に、y=b のオフセットで光線を発射する。
// 戻り値: true = 逃散 (r>250), false = 捕獲 (r<rBH)
bool shoot(double b, Ray &rayOut, long &stepsOut) {
  Ray ray(Vec3(-100.0, b, 0.0), Vec3(1.0, 0.0, 0.0));
  long steps = 0;
  while (ray.r >= rBH && ray.r <= 250.0 && steps < 3000000) {
    ray.update();
    steps++;
  }
  rayOut = ray;
  stepsOut = steps;
  return ray.r > 250.0;
}

} // namespace

int main() {
  char buf[256];

  // ---- 1. 光の曲がり角 ----
  // 弱場の展開: δ = 4M/b + (15π/4)(M/b)^2 + O((M/b)^3)
  for (double b : {10.0, 20.0, 40.0}) {
    Ray ray;
    long steps;
    bool escaped = shoot(b, ray, steps);
    Vec3 out = unitVector(cartDir(ray));
    double measured = acos(std::max(-1.0, std::min(1.0, out.e[0])));
    double expected = 4.0 * M / b + (15.0 * M_PI / 4.0) * (M / b) * (M / b);
    double rel = std::abs(measured - expected) / expected;
    std::snprintf(buf, sizeof(buf),
                  "b=%.0f measured=%.5f rad expected=%.5f rad rel=%.4f",
                  b, measured, expected, rel);
    check("bending angle", escaped && rel < 0.05, buf);
  }

  // ---- 2. 影の大きさ (捕獲される最大のインパクトパラメータ) ----
  // 理論値: b_crit = 3√3 M ≈ 2.5981
  {
    double lastCapture = 0.0;
    for (double b = 1.50; b <= 4.00; b += 0.02) {
      Ray ray;
      long steps;
      if (!shoot(b, ray, steps)) lastCapture = b;
    }
    double expected = 3.0 * sqrt(3.0) * M;
    std::snprintf(buf, sizeof(buf),
                  "last captured b=%.2f expected=%.4f (scan step 0.02)",
                  lastCapture, expected);
    check("shadow size", std::abs(lastCapture - expected) < 0.05, buf);
  }

  // ---- 3. フォトン球の円軌道と保存量 ----
  // r = 3M = 1.5 に接線方向の光子を置くと、周回し続ける (不安定平衡)。
  {
    Ray ray(Vec3(1.5, 0.0, 0.0), Vec3(0.0, 1.0, 0.0));
    double L0 = angMomentum(ray);
    double rmin = ray.r, rmax = ray.r;
    double maxNull = 0.0, maxL = 0.0;
    const long steps = 200000;
    for (long i = 0; i < steps; i++) {
      ray.update();
      rmin = std::min(rmin, ray.r);
      rmax = std::max(rmax, ray.r);
      maxNull = std::max(maxNull, std::abs(nullNorm(ray)));
      maxL = std::max(maxL, std::abs((angMomentum(ray) - L0) / L0));
    }
    std::snprintf(buf, sizeof(buf),
                  "r in [%.6f, %.6f], max|null|=%.3e, max|L drift|=%.3e",
                  rmin, rmax, maxNull, maxL);
    check("photon sphere", rmin > 1.499 && rmax < 1.501 && maxNull < 1e-9 &&
                               maxL < 1e-9,
          buf);
  }

  // ---- 4. 近接通過 (b=5) での保存量ドリフト ----
  // calcdd2 が RK4 の中間ステージで f と dt/dλ を再計算しない実装では、
  // このテストの |null| が 1e-4 程度まで悪化して FAIL する (回帰検出用)。
  {
    Ray ray(Vec3(-100.0, 5.0, 0.0), Vec3(1.0, 0.0, 0.0));
    double L0 = angMomentum(ray);
    double maxNull = 0.0, maxL = 0.0;
    long steps = 0;
    while (ray.r >= rBH && ray.r <= 250.0 && steps < 3000000) {
      ray.update();
      steps++;
      maxNull = std::max(maxNull, std::abs(nullNorm(ray)));
      maxL = std::max(maxL, std::abs((angMomentum(ray) - L0) / L0));
    }
    std::snprintf(buf, sizeof(buf),
                  "steps=%ld, max|null|=%.3e, max|L drift|=%.3e", steps,
                  maxNull, maxL);
    check("invariants (close pass)", maxNull < 1e-9 && maxL < 1e-9, buf);
  }

  std::printf("%s (%d failure(s))\n",
              failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED", failures);
  return failures == 0 ? 0 : 1;
}
