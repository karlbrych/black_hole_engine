#include "engine.h"
#include "models/texture.h"
#include "models/sphere.h"
#include "models/shader.h"
#include "gl_api.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cstdio>

static shader* gBgShader = nullptr;
static GLuint gBgTex = 0;
static GLuint gBgVAO = 0, gBgVBO = 0;

struct RenderObj {
    sphere mesh;
    GLuint tex = 0;
    glm::mat4 model = glm::mat4(1.0f);
};

static std::vector<RenderObj> gObjs;
static shader* gShader = nullptr;

static float gAspect = 800.0f / 600.0f;

void engineSetAspect(float aspect) {
    gAspect = (aspect > 0.0f) ? aspect : (800.0f / 600.0f);
}

void engineInit() {
    glEnable(GL_DEPTH_TEST);

    // Background shader + texture
    gBgShader = new shader("shaders/bg.vert", "shaders/bg.frag");
    gBgTex = Texture::LoadTexture("assets/space.jpg");
    printf("bg tex id = %u\n", (unsigned)gBgTex);

    // Fullscreen quad (pos + uv)
    float quad[] = {
    //  x,   y,   u,  v
    -1.f, -1.f,  0.f, 0.f,
    1.f, -1.f,  1.f, 0.f,
    -1.f,  1.f,  0.f, 1.f,

    -1.f,  1.f,  0.f, 1.f,
    1.f, -1.f,  1.f, 0.f,
    1.f,  1.f,  1.f, 1.f
    };

    glGenVertexArrays(1, &gBgVAO);
    glGenBuffers(1, &gBgVBO);
    glBindVertexArray(gBgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gBgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    glBindVertexArray(0);

#ifdef __EMSCRIPTEN__
    gShader = new shader("shaders/basic.vert", "shaders/basic.frag");
#else
    // uprav podle desktop cest – pokud desktop načítá relativně, klidně dej "shaders/..."
    gShader = new shader("src/shaders/basic.vert", "src/shaders/basic.frag");
#endif

    gObjs.clear();

    // Earth
    {
        RenderObj earth;
        earth.mesh  = createSphere(1.0f, 32, 16);
        earth.tex   = Texture::LoadTexture("assets/sun-texture.jpg");
        earth.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        gObjs.push_back(earth);

        std::printf("sun tex id = %u\n", (unsigned)earth.tex);
    }

    // Moon (zatím stejná textura)
    {
        RenderObj moon;
        moon.mesh  = createSphere(0.35f, 24, 12);
        moon.tex   = Texture::LoadTexture("assets/planet.jpg");
        moon.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
        gObjs.push_back(moon);

        std::printf("earth tex id = %u\n", (unsigned)moon.tex);
    }
}

void engineUpdate(float dt) {
    // jednoduchá orbit animace (ať je vidět, že update jede)
    static float t = 0.0f;
    t += dt;

    if (gObjs.size() >= 2) {
        float r = 2.0f;
        float x = r * std::cos(t);
        float z = r * std::sin(t);

        gObjs[1].model =
            glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)); // můžeš smazat
    }
}

void engineRender() {
    // Clear nejdřív
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1) Background
    glDisable(GL_DEPTH_TEST);
    gBgShader->use();
    Texture::BindTexture(gBgTex, 0);
    gBgShader->setInt("bgTex", 0);

    glBindVertexArray(gBgVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    // 2) Scene
    gShader->use();

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), gAspect, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    gShader->setMat4("projection", projection);
    gShader->setMat4("view", view);

    for (const auto& obj : gObjs) {
        gShader->setMat4("model", obj.model);

        Texture::BindTexture(obj.tex, 0);
        gShader->setInt("tex0", 0);

        glBindVertexArray(obj.mesh.VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)obj.mesh.indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}
