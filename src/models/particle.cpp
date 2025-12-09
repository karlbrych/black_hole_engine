#include "particle.h"
#include "shader.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



void Plane::draw(const shader &shader) const {
      for (auto &obj : objs)
          if (!obj->dead)
              obj->draw(shader);
  }

void Plane::rotate(float time)
{
	for(auto &obj:  objs) {
		obj -> rotate(time);
	}
}

void Object::draw(const shader &shader) const {
  shader.setMat4("model", modelMatrix);
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

}
void Object::rotate(float time)
{
  modelMatrix = glm::rotate(modelMatrix, time, glm::vec3(1.0f, 0.0f, 0.0));


}

static inline double quakeInvSqrt(double n) {
    uint64_t i;
    double x2 = n * 0.5;
    double y = n;
    memcpy(&i, &y, sizeof(i));
    i = 0x5fe6eb50c7aa19f9ULL - (i >> 1);
    memcpy(&y, &i, sizeof(y));
    const double three_halves = 1.5;
    y = y * (three_halves - (x2 * y * y));
    y = y * (three_halves - (x2 * y * y));
    return y;
}
#include <immintrin.h>

static inline double hwInvSqrt(double x) {
    __m128d in = _mm_set_sd(x);
    __m128d out = _mm_cvtss_sd(in, _mm_rsqrt_ss(_mm_cvtpd_ps(in)));
    __m128d half = _mm_set_sd(0.5 * x);
    __m128d threehalves = _mm_set_sd(1.5);
    out = _mm_mul_sd(out, _mm_sub_sd(threehalves, _mm_mul_sd(_mm_mul_sd(half, out), out)));
    return _mm_cvtsd_f64(out);
}

const bool hasSSE = __builtin_cpu_supports("sse");
double invSqrt(double n) {
#if defined(__x86_64__) || defined(__i386__)
    if (hasSSE)
        return hwInvSqrt(n);
    return quakeInvSqrt(n);
#else
    return quakeInvSqrt(n);
#endif
}

struct DSU {
    std::vector<int> parent;
    explicit DSU(int n) : parent(n) { for (int i = 0; i < n; i++) parent[i] = i; }
    int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
    void unite(int a, int b) { a = find(a); b = find(b); if (a != b) parent[b] = a; }
};

void DoGravity(Plane *plane, double G, double dt) {
    int n = plane->objs.size();
    if (n == 0) return;

    std::vector<glm::dvec3> accel(n, glm::dvec3(0));
    std::vector<bool> dead(n, false);

    // Gravity + collision detection
    for (int i = 0; i < n; ++i) {
        auto &a = plane->objs[i];
        if (a->mass <= 0) continue;

        for (int j = i + 1; j < n; ++j) {
            auto &b = plane->objs[j];
            if (b->mass <= 0) continue;

            double dx = b->pos.x - a->pos.x;
            double dy = b->pos.y - a->pos.y;
            double dz = b->pos.z - a->pos.z;

            double r2 = dx*dx + dy*dy + dz*dz;
            double r = sqrt(r2);

            double radiusSum = a->radius + b->radius;

            // Merge
            if (r < radiusSum * 0.99) {
                double totalMass = a->mass + b->mass;

                glm::dvec3 pos = (
                    a->mass * glm::dvec3(a->pos) +
                    b->mass * glm::dvec3(b->pos)
                ) / totalMass;

                glm::dvec3 vel = (
                    a->mass * glm::dvec3(a->xv, a->yv, a->zv) +
                    b->mass * glm::dvec3(b->xv, b->yv, b->zv)
                ) / totalMass;

                a->mass = totalMass;
                a->pos = glm::vec3(pos);
                a->xv = vel.x;
                a->yv = vel.y;
                a->zv = vel.z;

                dead[j] = true;
                continue;
            }

            double inv_r = invSqrt(r2 + 0.05 * 0.05);
            double factor = G * inv_r * inv_r * inv_r;

            accel[i] += b->mass * factor * glm::dvec3(dx, dy, dz);
            accel[j] -= a->mass * factor * glm::dvec3(dx, dy, dz);
        }
    }

    // Integrate motion
    for (int i = 0; i < n; ++i) {
        if (dead[i]) continue;
        auto &o = plane->objs[i];

        o->xv += accel[i].x * dt;
        o->yv += accel[i].y * dt;
        o->zv += accel[i].z * dt;

        o->pos += glm::vec3(o->xv * dt, o->yv * dt, o->zv * dt);
    }

    MergeAndDeadRemove(plane,&n,dead);


}


void MergeAndDeadRemove(Plane *plane, const int *n, std::vector<bool> &dead) {
    // Mark merged objects as dead
    for (int i = 0; i < *n; i++)
        if (dead[i])
            plane->objs[i]->dead = true;

    // Remove dead objects
    plane->objs.erase(
        std::remove_if(
            plane->objs.begin(),
            plane->objs.end(),
            [](const std::unique_ptr<Object> &o) {
                return o->dead;
            }
        ),
        plane->objs.end()
    );

    // Update matrices
    for (auto &o : plane->objs)
        o->modelMatrix = glm::translate(glm::mat4(1.0f), o->pos);
}

