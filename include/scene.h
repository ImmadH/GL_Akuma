#pragma once
#include "cglm/cglm.h"
#include "model.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#ifndef MAX_SCENE_INSTANCES
#define MAX_SCENE_INSTANCES 128
#endif

#ifndef MAX_POINT_LIGHTS
#define MAX_POINT_LIGHTS 16
#endif

typedef struct {
    vec3 pos;
    mat4 model;
    vec3 rotDeg;
    float scale;
    Model* mainModel;
} SceneInstance;

typedef struct {
    SceneInstance items[MAX_SCENE_INSTANCES];
    unsigned count;

    //point light system
    unsigned plCount;
    vec3     plPos[MAX_POINT_LIGHTS];
    vec3     plColor[MAX_POINT_LIGHTS];
    float    plIntensity[MAX_POINT_LIGHTS];
    float    plRange[MAX_POINT_LIGHTS];
    float    plScale[MAX_POINT_LIGHTS]; 
    int      plProxyIdx[MAX_POINT_LIGHTS]; 
    Model*   plCubeModel;                  
} Scene;

void scene_init(Scene* s);
unsigned scene_add(Scene* s, vec3 pos, Model* model);
void scene_update(Scene* s);
void scene_draw(const Scene* s, unsigned shader_program);

void scene_ui(Scene* s, char* const* list, int n, int* selected);
