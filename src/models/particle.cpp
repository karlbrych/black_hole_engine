#include "particle.h"
#include "shader.h"
#include <unordered_map>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
void Plane::draw(const shader &shader) const {
  for (const auto &obj : objs) {
    obj->draw(shader);
  }
}
void Object::draw(const shader &shader) const {
  shader.setMat4("model", modelMatrix);
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}


static inline double quakeInvSqrt(double n) {
    uint64_t i;
    double x2 = n * 0.5;
    double y = n;
  
    memcpy(&i, &y, sizeof(i));
    i = 0x5fe6eb50c7aa19f9ULL - (i >> 1);
    memcpy(&y, &i, sizeof(y));
  
    const double threehalves = 1.5;
    y = y * (threehalves - (x2 * y * y));
    y = y * (threehalves - (x2 * y * y));
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

    DSU(int n) : parent(n) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a != b) parent[b] = a;
    }
};

  void DoGravity(Plane *plane, double G, double dt) {
      int n = plane->objs.size();
      std::vector<glm::vec3> accel(n, glm::vec3(0,0,0));
      std::vector<int> mergeTarget(n, -1);
      DSU dsu(n);
      std::unordered_map<int, std::vector<int>> groups;
      bool anyCollision = false;
//movement between objects and collision detection
    for (int i = 0; i < n; ++i) {
        if (plane->objs[i]->mass <= 0) continue;

        for (int j = i + 1; j < n; ++j) {
            if (plane->objs[j]->mass <= 0) continue;

            double dx = plane->objs[j]->pos.x - plane->objs[i]->pos.x;
            double dy = plane->objs[j]->pos.y - plane->objs[i]->pos.y;
            double dz = plane->objs[j]->pos.z - plane->objs[i]->pos.z;

           double r2 = dx*dx + dy*dy + dz*dz;
            double radiusSum = plane->objs[i]->radius + plane->objs[j]->radius;

            // Merge if colliding
            if (r2 < radiusSum * radiusSum * 0.99f) {
                dsu.unite(i, j);
                anyCollision = true;
            }

            // Gravity calculation
           double inv_r = invSqrt(r2+0.05*0.05);
           const double factor = G * inv_r * inv_r * inv_r;

            accel[i].x += factor * plane->objs[j]->mass * dx;
            accel[i].y += factor * plane->objs[j]->mass * dy;
            accel[i].z += factor * plane->objs[j]->mass * dz;

            accel[j].x -= factor * plane->objs[i]->mass * dx;
            accel[j].y -= factor * plane->objs[i]->mass * dy;
            accel[j].z -= factor * plane->objs[i]->mass * dz;
        }
    }
//self movement with its velocity
    for (int i = 0; i < n; ++i) {
        if (plane->objs[i]->mass <= 0) continue;

        plane->objs[i]->xv += accel[i].x * dt;
        plane->objs[i]->yv += accel[i].y * dt;
        plane->objs[i]->zv += accel[i].z * dt;

        plane->objs[i]->pos.x += plane->objs[i]->xv * dt;
        plane->objs[i]->pos.y += plane->objs[i]->yv * dt;
        plane->objs[i]->pos.z += plane->objs[i]->zv * dt;
    }

//DSU collision
      if (anyCollision) {
          //grouping DSU
          for (int i = 0; i < n; i++) {
              int root = dsu.find(i);
              groups[root].push_back(i);
          }
          //sort by mass
          for (auto &kv : groups) {
              auto &group = kv.second;

              std::sort(group.begin(), group.end(),
                  [&](int a, int b) {
                      return plane->objs[a]->mass > plane->objs[b]->mass;
                  }
              );
          }



          // merging
          std::vector<bool> dead(n, false);

          for (auto &kv : groups) {
              auto &g = kv.second;
              if (g.size() <= 1) continue;

              int BiggestMass = g[0];

              double totalMass = 0.0;
              glm::dvec3 pos(0), vel(0);

              for (int idx : g) {
                  double m = plane->objs[idx]->mass;
                  if (m <= 0) continue;
                  totalMass += m;
                  pos += m * glm::dvec3(plane->objs[idx]->pos.x, plane->objs[idx]->pos.y, plane->objs[idx]->pos.z);
                  vel += m * glm::dvec3(plane->objs[idx]->xv, plane->objs[idx]->yv, plane->objs[idx]->zv);
              }

              pos /= totalMass;
              vel /= totalMass;

              plane->objs[BiggestMass]->mass = totalMass;
              plane->objs[BiggestMass]->pos = glm::vec3(pos);
              plane->objs[BiggestMass]->xv = vel.x;
              plane->objs[BiggestMass]->yv = vel.y;
              plane->objs[BiggestMass]->zv = vel.z;

              for (size_t k = 1; k < g.size(); ++k)
                  dead[g[k]] = true;
          }

          // rebuild objs
          std::vector<Object*> newObjs;
          newObjs.reserve(n);

          for (int i = 0; i < n; ++i)
              if (!dead[i])
                  newObjs.push_back(plane->objs[i]);

          plane->objs = std::move(newObjs);
      }
      for (auto &o : plane->objs) {
          o->modelMatrix = glm::translate(glm::mat4(1.0f), o->pos);
      }

}
