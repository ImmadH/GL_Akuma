#pragma once
#include "cglm/cglm.h"
#include "model.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#ifndef MAX_SCENE_INSTANCES
#define MAX_SCENE_INSTANCES 128
#endif

typedef struct {
    vec3 pos;
    mat4 model;
    vec3 rotDeg;
    Model* mainModel;
} SceneInstance;

typedef struct {
    SceneInstance items[MAX_SCENE_INSTANCES];
    unsigned count;
} Scene;

void scene_init(Scene* s);
unsigned scene_add(Scene* s, vec3 pos, Model* model);
void scene_update(Scene* s);
void scene_draw(const Scene* s, unsigned shader_program);

void scene_ui(Scene* s, char* const* list, int n, int* selected);
