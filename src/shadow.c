#include "shadow.h"
#include <string.h>
#include <stdio.h>
#include "shader.h"   
#include "model.h"      
#include "cglm/cam.h"

static GLuint make_depth_fbo(GLuint* outTex, int w, int h) {
    GLuint fbo, tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // for PCF
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float border[4] = {1,1,1,1};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Shadow FBO incomplete.\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    *outTex = tex;
    return fbo;
}

void shadow_init(ShadowSystem* s, int width, int height, const char* vsPath, const char* fsPath) {
    memset(s, 0, sizeof(*s));
    s->width = width; s->height = height;
    s->fbo = make_depth_fbo(&s->depthTex, width, height);

    Shader prog = {0};
    shader_create(&prog, (char*)vsPath, (char*)fsPath);
    s->depthProgram = prog.ID;
}

void shadow_shutdown(ShadowSystem* s) {
    if (s->depthProgram) glDeleteProgram(s->depthProgram);
    if (s->depthTex) glDeleteTextures(1, &s->depthTex);
    if (s->fbo) glDeleteFramebuffers(1, &s->fbo);
    memset(s, 0, sizeof(*s));
}

void shadow_update_dir(ShadowSystem* s, vec3 lightDir, vec3 focusPoint,
                       float orthoRadius, float nearZ, float farZ) {
    vec3 dir; glm_vec3_normalize_to(lightDir, dir);

    // Place the light so it looks toward focusPoint from opposite direction
    vec3 lightPos;
    glm_vec3_scale(dir, -50.0f, lightPos);
    glm_vec3_add(lightPos, focusPoint, lightPos);

    vec3 up = {0,1,0};
    glm_lookat(lightPos, focusPoint, up, s->lightView);
    glm_ortho(-orthoRadius, orthoRadius, -orthoRadius, orthoRadius, nearZ, farZ, s->lightProj);
    glm_mat4_mul(s->lightProj, s->lightView, s->lightVP);
}

void shadow_begin(ShadowSystem* s) {
    glViewport(0, 0, s->width, s->height);
    glBindFramebuffer(GL_FRAMEBUFFER, s->fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT); // fight peter-panning
    glUseProgram(s->depthProgram);

    GLint ul = glGetUniformLocation(s->depthProgram, "uLightVP");
    if (ul >= 0) glUniformMatrix4fv(ul, 1, GL_FALSE, (const float*)s->lightVP);
}

 void shadow_submit_model(const ShadowSystem* s, const struct Model* model, const mat4 modelMtx){
    GLint um = glGetUniformLocation(s->depthProgram, "model");
    if (um >= 0) glUniformMatrix4fv(um, 1, GL_FALSE, (const float*)modelMtx);
    model_draw_depth(model, s->depthProgram); 
}

void shadow_end(ShadowSystem* s, int winW, int winH) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winW, winH);
    glCullFace(GL_BACK);
}

int shadow_bind_depth(const ShadowSystem* s, int textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, s->depthTex);
    return textureUnit;
}

