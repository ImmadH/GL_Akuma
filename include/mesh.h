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

typedef struct Material {
    float diffuse[3];
    float ambient[3];
    float specular[3];
    float shininess;
    uint32_t diffuseTex;   // UINT32_MAX if none
    uint32_t normalTex;    // UINT32_MAX if none

    float    uvScale[2];   
    float    uvOffset[2]; 
} Material;

typedef struct Mesh 
{
  uint32_t VAO, VBO, EBO;
  uint32_t indexCount;
  uint32_t materialIndex;

} Mesh;

Mesh mesh_create(const cgltf_primitive* prim, uint32_t materialIndex);
Texture texture_create(const char* uri);
Material material_create(const cgltf_material* in,
                         const Texture* textures, uint32_t textureCount,
                         const cgltf_image* images_base);
void* get_buffer_data(const cgltf_accessor* accessor);




#endif

