#ifndef MESH_H
#define MESH_H
#include <stdint.h>
#include "cglm/cglm.h"
#include <stb_image.h>
#include "glad/glad.h"
#include <string.h>
#include "cgltf.h"
typedef struct Texture 
{
  uint32_t ID ;
  int width;
  int height;
  int channels;
  char* path; 
} Texture;

typedef struct Material 
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  uint32_t diffuseTex;
  uint32_t normalTex;

} Material;


typedef struct Mesh 
{
  uint32_t VAO, VBO, EBO;
  uint32_t indexCount;
  uint32_t materialIndex;

} Mesh;

Mesh mesh_create(const cgltf_primitive* prim, uint32_t materialIndex);
Texture texture_create(const char* uri);
Material material_create(const cgltf_material* in, const Texture* textures, uint32_t textureCount);

void* get_buffer_data(const cgltf_accessor* accessor);




#endif

