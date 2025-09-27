#ifndef MODEL_H
#define MODEL_H
#include "mesh.h"
#include <stdint.h>
#include "cgltf.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct Model 
{
  Mesh* meshes;
  uint32_t meshCount;
  Material* materials;
  uint32_t materialCount; 
  Texture* textures;
  uint32_t textureCount;

} Model;


Model* model_load(const char* filePath);
void model_destroy(Model* model);
void model_draw(const Model* model, uint32_t shaderProgram);
void model_draw_depth(const Model* model, uint32_t program);
#endif
