#pragma once
#include <stdint.h>
#include "glad/glad.h"
#include "cglm/cglm.h"

struct Model;

typedef struct ShadowSystem {
    GLuint fbo;
    GLuint depthTex;
    int    width, height;
    mat4   lightView, lightProj, lightVP;
    GLuint depthProgram;
} ShadowSystem;

void shadow_init(ShadowSystem* s, int width, int height,
                 const char* vsPath, const char* fsPath);
void shadow_shutdown(ShadowSystem* s);

void shadow_update_dir(ShadowSystem* s, vec3 lightDir, vec3 focusPoint,
                       float orthoRadius, float nearZ, float farZ);

void shadow_begin(ShadowSystem* s);
void shadow_end(ShadowSystem* s, int winW, int winH);
void shadow_submit_model(const ShadowSystem* s,
                         const struct Model* model,
                         const mat4 modelMtx);

int  shadow_bind_depth(const ShadowSystem* s, int textureUnit);

// getters
static inline const float* shadow_light_vp(const ShadowSystem* s) { return (const float*)s->lightVP; }
static inline GLuint shadow_depth_tex(const ShadowSystem* s) { return s->depthTex; }

