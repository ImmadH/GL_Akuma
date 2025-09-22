#include "model.h"
#include <stdint.h>
// model.c
#include "model.h"
#include "mesh.h"
#include <cgltf.h>
#include <stdlib.h>
#include <stdio.h>
#include "shader.h"

Model* model_load(const char* filePath)
{
    cgltf_options options = {0};
    cgltf_data*   data    = NULL;

    // 1) parse
    if (cgltf_parse_file(&options, filePath, &data) != cgltf_result_success) {
        fprintf(stderr, "model_load: failed to parse \"%s\"\n", filePath);
        return NULL;
    }
    // 2) load buffers
    if (cgltf_load_buffers(&options, data, filePath) != cgltf_result_success) {
        fprintf(stderr, "model_load: failed to load buffers for \"%s\"\n", filePath);
        cgltf_free(data);
        return NULL;
    }
    printf("STAGE1\n");
    
    // 3) allocate the Model struct
    Model* model = calloc(1, sizeof(*model));
    if (!model) {
        fprintf(stderr, "model_load: out of memory\n");
        cgltf_free(data);
        return NULL;
    }
    printf("STAGE2\n");

    // 4) count meshes
    uint32_t meshCount = 0;
    for (size_t i = 0; i < data->nodes_count; ++i) {
        if (data->nodes[i].mesh)
            meshCount += (uint32_t)data->nodes[i].mesh->primitives_count;
    }
    model->meshCount = meshCount;
    model->meshes    = malloc(meshCount * sizeof(*model->meshes));
    printf("STAGE3\n");

    // 5) load textures with NULL checks and proper path resolution
    model->textureCount = (uint32_t)data->images_count;
    model->textures     = malloc(model->textureCount * sizeof(*model->textures));
    
    // Extract directory from filePath for relative texture paths
    char* basePath = strdup(filePath);
    char* lastSlash = strrchr(basePath, '/');
    if (!lastSlash) lastSlash = strrchr(basePath, '\\');  // Windows support
    if (lastSlash) {
        *(lastSlash + 1) = '\0';  // Keep the slash, null-terminate after it
    } else {
        strcpy(basePath, "./");   // Current directory if no path separators
    }
    
    for (uint32_t i = 0; i < model->textureCount; ++i) {
        if (data->images[i].uri) {
            // Build full path: basePath + uri
            size_t fullPathLen = strlen(basePath) + strlen(data->images[i].uri) + 1;
            char* fullPath = malloc(fullPathLen);
            strcpy(fullPath, basePath);
            strcat(fullPath, data->images[i].uri);
            
            printf("Loading texture: %s\n", fullPath);
            model->textures[i] = texture_create(fullPath);
            
            if (model->textures[i].ID == 0) {
                printf("Warning: Failed to load texture: %s\n", fullPath);
                // Create a default white texture to avoid crashes
                model->textures[i] = (Texture){0};
                model->textures[i].path = strdup(data->images[i].uri);  // Keep original URI
            }
            
            free(fullPath);
        } else {
            printf("Warning: Image %u has NULL URI\n", i);
            // Initialize empty texture
            model->textures[i] = (Texture){0};
        }
    }
    
    free(basePath);
    printf("STAGE4\n");

    // 6) build materials
    model->materialCount = (uint32_t)data->materials_count;
    model->materials     = malloc(model->materialCount * sizeof(*model->materials));
    for (uint32_t i = 0; i < model->materialCount; ++i) {
        model->materials[i] =
            material_create(&data->materials[i],
                            model->textures,
                            model->textureCount);
    }
    printf("STAGE5\n");

    // 7) build meshes - FIXED LOGIC
    uint32_t meshIdx = 0;
    for (size_t ni = 0; ni < data->nodes_count; ++ni) {
        cgltf_node *node = &data->nodes[ni];
        if (!node->mesh) continue;
        
        for (uint32_t pi = 0; pi < node->mesh->primitives_count; ++pi) {
            cgltf_primitive *prim = &node->mesh->primitives[pi];
            
            // compute material index - FIXED: initialize to 0
            uint32_t matIdx = 0;  // Default material
            if (prim->material) {
                matIdx = (uint32_t)(prim->material - data->materials);
                if (matIdx >= model->materialCount) {
                    // sanity check, fall back to 0
                    matIdx = 0;
                }
            }
            
            // FIXED: Always create mesh, not just in else block
            model->meshes[meshIdx++] = mesh_create(prim, matIdx);
        }
    }
    printf("STAGE6\n");
    
    // 8) done
    cgltf_free(data);
    printf("STAGE7\n");
    return model;
}

void model_destroy(Model* model)
{
   if (!model)
   {
     return;
   }

   for (uint32_t i = 0; i < model->meshCount; ++i)
   {
     Mesh* m = &model->meshes[i];
     glDeleteVertexArrays(1, &m->VAO);
     glDeleteBuffers(1, &m->VBO);
     glDeleteBuffers(1, &m->EBO);
   }
   free(model->meshes);

   for (uint32_t i = 0; i < model->textureCount; ++i)
   {
     glDeleteTextures(1, &model->textures[i].ID);
     free(model->textures[i].path);
   }
   free(model->textures);
}




void model_draw(const Model* model, uint32_t program)
{
    if (!model || model->meshCount == 0 || !model->meshes || !model->materials) return;

    glUseProgram(program);

    //cache uniform locations
    const GLint locSam  = glGetUniformLocation(program, "uTex0");
    const GLint locHas  = glGetUniformLocation(program, "uHasDiffuse");
    const GLint locTint = glGetUniformLocation(program, "uTint");
    if (locSam >= 0) glUniform1i(locSam, 0);  // texture unit 0
    

    glActiveTexture(GL_TEXTURE0);

    for (uint32_t i = 0; i < model->meshCount; ++i) {
        const Mesh*     mesh = &model->meshes[i];
        const Material* mat  = &model->materials[mesh->materialIndex];

        // set tint from material diffuse (fallback to white)
        if (locTint >= 0) {
            float tint[4] = {1.f, 1.f, 1.f, 1.f};
            if (mat) {
                tint[0] = mat->diffuse[0];
                tint[1] = mat->diffuse[1];
                tint[2] = mat->diffuse[2];
            }
            glUniform4fv(locTint, 1, tint);
        }

        // safe texture presence + bind
        bool has = false;
        uint32_t idx = 0;
        if (model->textureCount > 0 &&
            mat &&
            mat->diffuseTex != UINT32_MAX &&
            mat->diffuseTex < model->textureCount)
        {
            idx = mat->diffuseTex;
            has = (model->textures[idx].ID != 0);
        }

        if (locHas >= 0) glUniform1i(locHas, has ? 1 : 0);
        glBindTexture(GL_TEXTURE_2D, has ? model->textures[idx].ID : 0);


        glBindVertexArray(mesh->VAO);
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}
