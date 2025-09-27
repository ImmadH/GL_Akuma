#include "scene.h"
#include <string.h>
#include "cglm/util.h"
#include "glad/glad.h"
#include <limits.h>

void scene_init(Scene* s) {
    memset(s, 0, sizeof(*s));
    for (unsigned i = 0; i < MAX_POINT_LIGHTS; ++i) s->plProxyIdx[i] = -1;
    s->plCubeModel = NULL;
}

unsigned scene_add(Scene* s, vec3 pos, Model* model) {
    if (s->count >= MAX_SCENE_INSTANCES) return UINT_MAX; // full
    SceneInstance* inst = &s->items[s->count];

    glm_vec3_copy(pos, inst->pos);
    glm_vec3_zero(inst->rotDeg);
    glm_mat4_identity(inst->model);
    inst->scale = 1.0f;
    inst->mainModel = model;

    return s->count++;
}

void scene_update(Scene* s)
{
    for (unsigned i = 0; i < s->plCount; ++i) {
        int idx = s->plProxyIdx[i];
        if (idx >= 0 && (unsigned)idx < s->count) {
            glm_vec3_copy(s->plPos[i], s->items[idx].pos);
        }
    }

    for (unsigned i = 0; i < s->count; ++i) {
        SceneInstance* inst = &s->items[i];
        glm_mat4_identity(inst->model);
        glm_translate(inst->model, inst->pos);

        const float rx = glm_rad(inst->rotDeg[0]);
        const float ry = glm_rad(inst->rotDeg[1]);
        const float rz = glm_rad(inst->rotDeg[2]);

        if (rx != 0.0f) glm_rotate_x(inst->model, rx, inst->model);
        if (ry != 0.0f) glm_rotate_y(inst->model, ry, inst->model);
        if (rz != 0.0f) glm_rotate_z(inst->model, rz, inst->model);

        if (inst->scale != 1.0f) {
            float sc = inst->scale;
            if (sc < 0.0001f) sc = 0.0001f;
            glm_scale_uni(inst->model, sc);
        }
    }

    
    for (unsigned i = 0; i < s->plCount; ++i) {
        int idx = s->plProxyIdx[i];
        if (idx >= 0 && (unsigned)idx < s->count) {
            float sc = (s->plScale[i] > 0.0001f) ? s->plScale[i] : 0.0001f;
            glm_scale_uni(s->items[idx].model, sc);
        }
    }
}



void scene_draw(const Scene* s, uint32_t program)
{
    if (!s) return;

    glUniform1i(glGetUniformLocation(program, "uPLCount"), s->plCount);

    if (s->plCount > 0) {
        glUniform3fv(glGetUniformLocation(program, "uPLPos[0]"),
                     s->plCount, (const GLfloat*)s->plPos);
        glUniform3fv(glGetUniformLocation(program, "uPLColor[0]"),
                     s->plCount, (const GLfloat*)s->plColor);
        glUniform1fv(glGetUniformLocation(program, "uPLIntensity[0]"),
                     s->plCount, (const GLfloat*)s->plIntensity);
        glUniform1fv(glGetUniformLocation(program, "uPLRange[0]"),
                     s->plCount, (const GLfloat*)s->plRange);
    }

    const GLint locModel       = glGetUniformLocation(program, "model");
    const GLint locUseEmissive = glGetUniformLocation(program, "uUseEmissive");
    const GLint locEmissiveCol = glGetUniformLocation(program, "uEmissiveColor");

    for (unsigned i = 0; i < s->count; ++i) {
        const SceneInstance* inst = &s->items[i];
        if (!inst->mainModel) continue;

        if (locModel >= 0) {
            glUniformMatrix4fv(locModel, 1, GL_FALSE, (float*)inst->model);
        }

        int lightIdx = -1;
        if (s->plCubeModel && inst->mainModel == s->plCubeModel) {
            for (unsigned j = 0; j < s->plCount; ++j) {
                if (s->plProxyIdx[j] == (int)i) {
                    lightIdx = (int)j;
                    break;
                }
            }
        }

        if (lightIdx >= 0 && locUseEmissive >= 0 && locEmissiveCol >= 0) {
            float emissive[3] = {
                s->plColor[lightIdx][0] * s->plIntensity[lightIdx],
                s->plColor[lightIdx][1] * s->plIntensity[lightIdx],
                s->plColor[lightIdx][2] * s->plIntensity[lightIdx]
            };
            glUniform1i(locUseEmissive, 1);
            glUniform3fv(locEmissiveCol, 1, emissive);
        } else if (locUseEmissive >= 0) {
            glUniform1i(locUseEmissive, 0);
        }

        model_draw(inst->mainModel, program);
    }
}


static int scene_pl_add(Scene* s, vec3 pos, vec3 color, float intensity, float range) 
{
    if (s->plCount >= MAX_POINT_LIGHTS) return -1;
    unsigned i = s->plCount++;
    glm_vec3_copy(pos,   s->plPos[i]);
    glm_vec3_copy(color, s->plColor[i]);
    s->plIntensity[i] = intensity;
    s->plRange[i]     = range;
    s->plScale[i]     = 0.2f;
    s->plProxyIdx[i]  = -1;

    if (s->plCubeModel && s->count < MAX_SCENE_INSTANCES) {
        unsigned idx = scene_add(s, pos, s->plCubeModel);
        if (idx != UINT_MAX) {
            s->plProxyIdx[i] = (int)idx;
        }
    }
    return (int)i;
}

void scene_ui(Scene* s, char* const* list, int n, int* selected) {
    if (igCollapsingHeader_TreeNodeFlags("Scene", 0)) {
        igPushItemWidth(220.0f);
        const char* cur = list[*selected];
        if (igBeginCombo("##SelectModel", cur, 0)) {
            for (int i = 0; i < n; ++i) {
                bool sel = (*selected == i);
                if (igSelectable_Bool(list[i], sel, 0, (ImVec2){0,0})) *selected = i;
                if (sel) igSetItemDefaultFocus();
            }
            igEndCombo();
        }
        igSameLine(0, 8); igText("Select Model");
        igPopItemWidth();

        if (igButton("Load Model", (ImVec2){0,0})) {
            Model* mdl = model_load(list[*selected]);          
            if (mdl && s->count < MAX_SCENE_INSTANCES) {
                vec3 p = {0.0f, 0.0f, -5.0f};
                unsigned idx = scene_add(s, p, mdl);           
                if (idx != UINT_MAX) {
                    s->items[idx].rotDeg[0] = 90.0f;          
                }
            }
        }

        igSeparator();
        igText("Model Properties");
        if (igCollapsingHeader_TreeNodeFlags("Models", 0)) {
            for (unsigned i = 0; i < s->count; ++i) {
                igPushID_Int((int)i);
                char label[48]; snprintf(label, sizeof(label), "Instance %u", i);
                if (igTreeNode_Str(label)) {
                    igText("Position");
                    igPushItemWidth(-1);
                    igInputFloat3("##pos", s->items[i].pos, "%.3f", 0);   
                    igPopItemWidth();

                    igText("Rotation (deg)");
                    igPushItemWidth(-1);
                    igInputFloat3("##rot", s->items[i].rotDeg, "%.1f", 0); 
                    igPopItemWidth();
                    
                    igPushItemWidth(-1);
                    igSliderFloat("Scale", &s->items[i].scale, 0.01f, 10.0f, "%.2f", 0);
                    igPopItemWidth();

                    igTreePop();
                }
                igPopID();
            }
        }


        igSeparator();
        if (igCollapsingHeader_TreeNodeFlags("Lighting", 0)) {

            if (!s->plCubeModel) {
                if (igButton("Load Light Cube (once)", (ImVec2){0,0})) {
                    s->plCubeModel = model_load("assets/cube/Box.gltf");
                }
            }

            static vec3  newPos  = {0.0f, 1.0f, -5.0f};
            static vec3  newCol  = {1.0f, 1.0f, 1.0f};
            static float newInt  = 2.0f;
            static float newRng  = 15.0f;

            igText("New Point Light");
            igInputFloat3("Pos",   newPos, "%.3f", 0);  
            igInputFloat3("Color", newCol, "%.3f", 0); 
            igSliderFloat("Intensity", &newInt, 0.0f, 10.0f, "%.2f", 0);
            igSliderFloat("Range",     &newRng, 0.1f, 50.0f, "%.1f", 0);
            
            if (igButton("Add Point Light", (ImVec2){0,0})) {
                scene_pl_add(s, newPos, newCol, newInt, newRng);
            }

            if (s->plCount > 0 && igCollapsingHeader_TreeNodeFlags("Point Lights", 0)) {
                for (unsigned i = 0; i < s->plCount; ++i) {
                    igPushID_Int((int)i);
                    char lbl[48]; snprintf(lbl, sizeof(lbl), "Light %u", i);
                    if (igTreeNode_Str(lbl)) {
                        igText("Pos");
                        igSliderFloat("X", &s->plPos[i][0], -50.0f, 50.0f, "%.3f", 0);
                        igSliderFloat("Y", &s->plPos[i][1], -50.0f, 50.0f, "%.3f", 0);
                        igSliderFloat("Z", &s->plPos[i][2], -50.0f, 50.0f, "%.3f", 0);
                        igColorEdit3("Color",   s->plColor[i], 0);
                        igSliderFloat("Intensity", &s->plIntensity[i], 0.0f, 10.0f, "%.2f", 0);
                        igSliderFloat("Range",     &s->plRange[i],     0.1f, 50.0f, "%.1f", 0);
                        igSliderFloat("Scale",     &s->plScale[i], 0.01f, 5.0f, "%.2f", 0);
                        igTreePop();
                    }
                    igPopID();
                }
            }
        }
    }
}



