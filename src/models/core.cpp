#define _USE_MATH_DEFINES
#include "core.h"
#include "shader.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "texture.h"
#include <iostream>
#include <filesystem>

void save_to_binary(const Plane* plane, const std::string &filename) {
    // Create saves directory if it doesn't exist
    std::filesystem::path filepath(filename);
    std::filesystem::path directory = filepath.parent_path();
    if (!directory.empty() && !std::filesystem::exists(directory)) {
        std::filesystem::create_directories(directory);
        std::cout << "Created directory: " << directory << std::endl;
    }
    
    std::ofstream out(filename, std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing!" << std::endl;
        return;
    }

    // Write the version (size_t)
    out.write(reinterpret_cast<const char*>(&plane->version), sizeof(plane->version));

    // Write the size of the vector (objs.size())
    size_t objs_size = plane->objs.size();
    out.write(reinterpret_cast<const char*>(&objs_size), sizeof(objs_size));


    out.write(reinterpret_cast<const char*>(&plane->dt), sizeof(plane->dt));
    out.write(reinterpret_cast<const char*>(&plane->G), sizeof(plane->G));
    // Write the objects data
    for (const auto &obj : plane->objs) {
        obj->serialize(out);
    }

    out.close();
    std::cout << "Data saved to " << filename << std::endl;
}
void load_from_binary(Plane* plane, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);

    if (!in.is_open()) {
        std::cerr << "Failed to open file for reading!" << std::endl;
        return;
    }
    size_t file_version = 0;
    in.read(reinterpret_cast<char*>(&file_version), sizeof(file_version));
    // Read the version (size_t)
    if (file_version != plane->version) {
        std::cerr << "Version mismatch! Expected version " << plane->version 
                  << ", but found version " << file_version << std::endl;
        return; // or handle the error in another way (e.g., throw an exception)
    }
    // Read the size of the vector (objs.size()) as size_t
    size_t objs_size = 0;
    in.read(reinterpret_cast<char*>(&objs_size), sizeof(objs_size));
    if (!in) {
        std::cerr << "Failed to read vector size!" << std::endl;
        return;
    }
    in.read(reinterpret_cast<char *>(&plane->dt), sizeof(plane->dt));
    in.read(reinterpret_cast<char *>(&plane->G), sizeof(plane->G));
    // Clear the existing objects in the plane
    for (auto obj : plane->objs) {
        delete obj; // Free the memory allocated for existing objects
    }
    plane->objs.clear(); // Clear the vector
    // Resize the vector to accommodate the objects
    plane->objs.resize(objs_size);

    // Read each object into the vector
    for (size_t i = 0; i < objs_size; ++i) {
        plane->objs[i] = Object::deserializeFrom(in);  // Allocate memory for each object
        if (!plane->objs[i]) {
            std::cerr << "Failed to deserialize object " << i << std::endl;
            return;
        }
    }

    in.close();
    std::cout << "Data loaded from " << filename << std::endl;
}

void Plane::draw(const shader &shader) const {
  for (const auto &obj : objs) {
    obj->draw(shader);
  }
}
void Plane::rotate(float time) 
{
	for(auto &obj:  objs) {
		obj -> rotate(time);
	}
}

void Object::draw(const shader &shader) const {
  Texture::BindTexture(textureId, 0);
  shader.setMat4("model", modelMatrix);
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
void Object::rotate(float time)
{
  modelMatrix = glm::rotate(modelMatrix, time, glm::vec3(0.13f, 1.0f, 0.0));
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

double invSqrt(double n) {
#if defined(__GNUC__) || defined(__clang__)
    const bool hasSSE = __builtin_cpu_supports("sse");
#else
    const bool hasSSE = true;
#endif

#if defined(__x86_64__) || defined(__i386__)
    if (hasSSE)
        return hwInvSqrt(n);
    return quakeInvSqrt(n);
#else
    return quakeInvSqrt(n);
#endif
}
  void DoGravity(Plane *plane, double G, double dt) {
    int n = plane->objs.size();
    std::vector<glm::vec3> accel(n, glm::vec3(0,0,0));
    std::vector<int> mergeTarget(n, -1);

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
            if (r2 < radiusSum * radiusSum*0.99) {
                mergeTarget[j] = i; // merge j into i
            }

            // Gravity calculation
           double inv_r = invSqrt(r2+0.05*0.05);
       
            double factor = G * inv_r * inv_r * inv_r;

            accel[i].x += factor * plane->objs[j]->mass * dx;
            accel[i].y += factor * plane->objs[j]->mass * dy;
            accel[i].z += factor * plane->objs[j]->mass * dz;

            accel[j].x -= factor * plane->objs[i]->mass * dx;
            accel[j].y -= factor * plane->objs[i]->mass * dy;
            accel[j].z -= factor * plane->objs[i]->mass * dz;
        }
    }

    for (int i = 0; i < n; ++i) {
        if (plane->objs[i]->mass <= 0) continue;

        plane->objs[i]->xv += accel[i].x * dt;
        plane->objs[i]->yv += accel[i].y * dt;
        plane->objs[i]->zv += accel[i].z * dt;

        plane->objs[i]->pos.x += plane->objs[i]->xv * dt;
        plane->objs[i]->pos.y += plane->objs[i]->yv * dt;
        plane->objs[i]->pos.z += plane->objs[i]->zv * dt;
    }

    for (int j = 0; j < n; ++j) {
        int i = mergeTarget[j];
        if (i < 0 || plane->objs[i]->mass <= 0 || plane->objs[j]->mass <= 0) continue;

        double totalMass = plane->objs[i]->mass + plane->objs[j]->mass;

        // Center-of-mass position
        plane->objs[i]->pos = (plane->objs[i]->pos * (float)plane->objs[i]->mass +
                             plane->objs[j]->pos * (float)plane->objs[j]->mass) / (float)totalMass;

        // Momentum conservation
        plane->objs[i]->xv = (plane->objs[i]->xv * plane->objs[i]->mass + plane->objs[j]->xv * plane->objs[j]->mass) / totalMass;
        plane->objs[i]->yv = (plane->objs[i]->yv * plane->objs[i]->mass + plane->objs[j]->yv * plane->objs[j]->mass) / totalMass;
        plane->objs[i]->zv = (plane->objs[i]->zv * plane->objs[i]->mass + plane->objs[j]->zv * plane->objs[j]->mass) / totalMass;

        plane->objs[i]->mass = totalMass;

        // Fast approximate radius scaling: radius ~ cube root of mass
        plane->objs[i]->radius = cbrt(totalMass);

        // Mark merged object as inactie
        plane->objs[j]->mass = 0;
    }

    plane->objs.erase(
        std::remove_if(plane->objs.begin(), plane->objs.end(),
                       [](const Object *o){ return o->mass <= 0; }),
        plane->objs.end()
    );
	  // Update modelMatrix from pos so rendering follows simulation
  for (auto &o : plane->objs) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, o->pos);
		model = glm::scale(model, glm::vec3(o->radius));
		o->modelMatrix = model;
  }
}
