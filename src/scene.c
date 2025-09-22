#include "scene.h"
#include <string.h>
#include "cglm/util.h"
#include "glad/glad.h"
#include <limits.h>

void scene_init(Scene* s) {
    memset(s, 0, sizeof(*s));
}

unsigned scene_add(Scene* s, vec3 pos, Model* model) {
    if (s->count >= MAX_SCENE_INSTANCES) return UINT_MAX; // full
    SceneInstance* inst = &s->items[s->count];

    glm_vec3_copy(pos, inst->pos);
    glm_vec3_zero(inst->rotDeg);
    glm_mat4_identity(inst->model);
    inst->mainModel = model;

    return s->count++;
}

void scene_update(Scene* s) {
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
    }
}

void scene_draw(const Scene* s, uint32_t program) {
    if (s->count == 0) return;

    // NOTE: change "model" to "uModel" if your shader uses that
    const GLint locModel = glGetUniformLocation(program, "model");
    glUseProgram(program);

    for (unsigned i = 0; i < s->count; ++i) {
        const SceneInstance* inst = &s->items[i];
        if (!inst->mainModel) continue;

        if (locModel >= 0) {
            glUniformMatrix4fv(locModel, 1, GL_FALSE, (float*)inst->model);
        }
        model_draw(inst->mainModel, program);
    }
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
                    s->items[idx].rotDeg[0] = 45.0f;
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
                    igText("Model Position");
                    igPushItemWidth(-1);
                    igInputFloat3("##pos", s->items[i].pos, "%.3f", 0);
                    igPopItemWidth();

                    igText("Model Rotation (deg)");
                    igPushItemWidth(-1);
                    igInputFloat3("##rot", s->items[i].rotDeg, "%.1f", 0);
                    igPopItemWidth();

                    igTreePop();
                }
                igPopID();
            }
        }
    }
}

