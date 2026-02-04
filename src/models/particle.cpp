#ifndef __EMSCRIPTEN__
  #include <immintrin.h>   // SSE intrinsics (desktop only)
#endif

#include "particle.h"
#include "shader.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"

void Plane::draw(const shader &shader) const {
  for (const auto &obj : objs) {
    obj->draw(shader);
  }
}

void Plane::rotate(float time)
{
  for (auto &obj : objs) {
    obj->rotate(time);
  }
}

void Object::draw(const shader &shader) const {
  Texture::BindTexture(textureId, 0);
  shader.setMat4("model", modelMatrix);
  this->sphere.draw(shader);
}

void Object::rotate(float time)
{
  modelMatrix = glm::rotate(modelMatrix, time, glm::vec3(1.0f, 0.0f, 0.0));
}

// --- invSqrt helpers ---

static inline double quakeInvSqrt(double n) {
    uint64_t i;
    double x2 = n * 0.5;
    double y = n;

    std::memcpy(&i, &y, sizeof(i));
    i = 0x5fe6eb50c7aa19f9ULL - (i >> 1);
    std::memcpy(&y, &i, sizeof(y));

    const double threehalves = 1.5;
    y = y * (threehalves - (x2 * y * y));
    y = y * (threehalves - (x2 * y * y));
    return y;
}

#ifndef __EMSCRIPTEN__
// SSE verze – jen desktop x86
#if defined(__x86_64__) || defined(__i386__)
static inline double hwInvSqrt(double x) {
    __m128d in = _mm_set_sd(x);
    __m128d out = _mm_cvtss_sd(in, _mm_rsqrt_ss(_mm_cvtpd_ps(in)));
    __m128d half = _mm_set_sd(0.5 * x);
    __m128d threehalves = _mm_set_sd(1.5);

    out = _mm_mul_sd(out, _mm_sub_sd(threehalves, _mm_mul_sd(half, _mm_mul_sd(out, out))));

    double r;
    _mm_store_sd(&r, out);
    return r;
}
#endif
#endif

// jediná veřejná definice (odpovídá particle.h: double invSqrt(double n);)
double invSqrt(double n) {
#ifdef __EMSCRIPTEN__
    // WebAssembly: žádné SSE, žádný cpu_supports
    return quakeInvSqrt(n);
#else
    #if defined(__x86_64__) || defined(__i386__)
        #if defined(__GNUC__) || defined(__clang__)
            const bool hasSSE = __builtin_cpu_supports("sse");
        #else
            const bool hasSSE = true;
        #endif

        if (hasSSE) return hwInvSqrt(n);
        return quakeInvSqrt(n);
    #else
        return quakeInvSqrt(n);
    #endif
#endif
}

void DoGravity(Plane *plane, double G, double dt) {
    int n = (int)plane->objs.size();
    std::vector<glm::dvec3> accel(n, glm::dvec3(0.0));

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double dx = plane->objs[j]->position.x - plane->objs[i]->position.x;
            double dy = plane->objs[j]->position.y - plane->objs[i]->position.y;
            double dz = plane->objs[j]->position.z - plane->objs[i]->position.z;

            double r2 = dx*dx + dy*dy + dz*dz;

            // Gravity calculation
            double inv_r = invSqrt(r2 + 0.05*0.05);
            double factor = G * inv_r * inv_r * inv_r;

            accel[i].x += factor * plane->objs[j]->mass * dx;
            accel[i].y += factor * plane->objs[j]->mass * dy;
            accel[i].z += factor * plane->objs[j]->mass * dz;

            accel[j].x -= factor * plane->objs[i]->mass * dx;
            accel[j].y -= factor * plane->objs[i]->mass * dy;
            accel[j].z -= factor * plane->objs[i]->mass * dz;
        }
    }

    for (int i = 0; i < n; i++) {
        plane->objs[i]->velocity += accel[i] * dt;
        plane->objs[i]->position += plane->objs[i]->velocity * dt;

        plane->objs[i]->modelMatrix = glm::translate(glm::dmat4(1.0), plane->objs[i]->position);
    }
}
