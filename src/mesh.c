#include "mesh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static Texture make_white_texture(void) {
    Texture t = (Texture){0};
    glGenTextures(1, &t.ID);
    glBindTexture(GL_TEXTURE_2D, t.ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    unsigned char px[4] = {255,255,255,255};
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1,1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glGenerateMipmap(GL_TEXTURE_2D);

    t.width = t.height = 1;
    t.channels = 4;
    t.path = NULL;
    return t;
}


Texture texture_create(const char* uri)
{
    if (uri && strcmp(uri, "__white__") == 0) {
        return make_white_texture();
    }

    Texture texture = (Texture){0};

    int w = 0, h = 0, n = 0;
    // Force 4 channels (RGBA) on load to avoid format mismatches.
    unsigned char* data = stbi_load(uri, &w, &h, &n, 4);
    if (!data || w <= 0 || h <= 0) {
        fprintf(stderr, "ERROR: stbi_load failed for '%s' (%s)\n",
                uri ? uri : "(null)", stbi_failure_reason());
        return make_white_texture();
    }

    glGenTextures(1, &texture.ID);
    glBindTexture(GL_TEXTURE_2D, texture.ID);

    // Robust defaults
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Tight packing: avoids crashes/misreads on 3/4-byte rows
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Upload as RGBA 8-bit no matter the source.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        fprintf(stderr, "GL ERROR after glTexImage2D for '%s': 0x%04X\n",
                uri ? uri : "(null)", (unsigned)err);
        // keep going, but ensure we return a valid texture if upload failed
        glDeleteTextures(1, &texture.ID);
        stbi_image_free(data);
        return make_white_texture();
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    texture.width    = w;
    texture.height   = h;
    texture.channels = 4;

    if (uri) {
        size_t len = strlen(uri) + 1;
        texture.path = (char*)malloc(len);
        if (texture.path) memcpy(texture.path, uri, len);
    }

    fprintf(stderr, "OK: '%s' -> %dx%d RGBA\n", uri ? uri : "(null)", w, h);
    return texture;
}


Material material_create(const cgltf_material* in,
                         const Texture* textures, uint32_t textureCount,
                         const cgltf_image* images_base)
{
    Material material = (Material){0};

    // defaults
    material.ambient[0] = material.ambient[1] = material.ambient[2] = 0.1f;
    material.diffuse[0] = material.diffuse[1] = material.diffuse[2] = 1.0f;
    material.specular[0] = material.specular[1] = material.specular[2] = 1.0f;
    material.shininess   = 32.0f;
    material.diffuseTex  = UINT32_MAX;
    material.normalTex   = UINT32_MAX;
    material.uvScale[0]  = material.uvScale[1]  = 1.0f;
    material.uvOffset[0] = material.uvOffset[1] = 0.0f;

    if (!in) return material;

    const float* f = in->pbr_metallic_roughness.base_color_factor
                     ? in->pbr_metallic_roughness.base_color_factor
                     : (const float[4]){1,1,1,1};
    material.diffuse[0] = f[0];
    material.diffuse[1] = f[1];
    material.diffuse[2] = f[2];

    // ambient = 10% of diffuse
    material.ambient[0] = 0.1f * f[0];
    material.ambient[1] = 0.1f * f[1];
    material.ambient[2] = 0.1f * f[2];

    const float roughness = in->pbr_metallic_roughness.roughness_factor;
    material.shininess = (1.0f - roughness) * 255.0f + 1.0f;

    // optional spec tweak from metallic
    const float metallic = in->pbr_metallic_roughness.metallic_factor;
    if (metallic > 0.0f) {
        material.specular[0] = f[0] * metallic;
        material.specular[1] = f[1] * metallic;
        material.specular[2] = f[2] * metallic;
    }

    const cgltf_texture_view* bc = &in->pbr_metallic_roughness.base_color_texture;
    if (bc->texture) {
        const cgltf_image* img = bc->texture->image;
        if (img && images_base) {
            uint32_t idx = (uint32_t)(img - images_base);
            if (idx < textureCount) material.diffuseTex = idx;
        }

        if (bc->has_transform) {
            const cgltf_texture_transform* xf = &bc->transform; // take address of struct
            material.uvScale[0]  = xf->scale[0];
            material.uvScale[1]  = xf->scale[1];
            material.uvOffset[0] = xf->offset[0];
            material.uvOffset[1] = xf->offset[1];
        }
    }

    // Normal map future use for normal mapping ;)
    const cgltf_texture_view* nrm = &in->normal_texture;
    if (nrm->texture) {
        const cgltf_image* nimg = nrm->texture->image;
        if (nimg && images_base) {
            uint32_t nidx = (uint32_t)(nimg - images_base);
            if (nidx < textureCount) material.normalTex = nidx;
        }
    }

    return material;
}



Mesh mesh_create(const cgltf_primitive* prim, uint32_t materialIndex)
{
    Mesh mesh = (Mesh){0};
    mesh.materialIndex = materialIndex;

    // pick accessors
    const cgltf_accessor* position   = NULL;
    const cgltf_accessor* normals    = NULL;
    const cgltf_accessor* texcoords0 = NULL;
    const cgltf_accessor* texcoords1 = NULL;

    for (size_t i = 0; i < prim->attributes_count; ++i) {
        const cgltf_attribute* attr = &prim->attributes[i];
        switch (attr->type) {
            case cgltf_attribute_type_position: position  = attr->data; break;
            case cgltf_attribute_type_normal:   normals   = attr->data; break;
            case cgltf_attribute_type_texcoord:
                if (attr->index == 0) texcoords0 = attr->data;
                if (attr->index == 1) texcoords1 = attr->data;
                break;
            default: break;
        }
    }

    if (!position) return mesh; 

    int desired_texcoord = 0;
    if (prim->material) {
        const cgltf_texture_view* bc = &prim->material->pbr_metallic_roughness.base_color_texture;
        desired_texcoord = (int)bc->texcoord; // cgltf default is 0
    }
    const cgltf_accessor* texCoords = NULL;
    if (desired_texcoord == 1 && texcoords1)      texCoords = texcoords1;
    else if (desired_texcoord == 0 && texcoords0) texCoords = texcoords0;
    else if (texcoords0)                          texCoords = texcoords0;
    else if (texcoords1)                          texCoords = texcoords1;

    // vertex count & layout (pos=3, +normal?3, +uv?2)
    const size_t vertexCount = position->count;
    int comps = 3;                 // pos
    if (normals)   comps += 3;     // normal
    if (texCoords) comps += 2;     // uv
    const size_t strideFloats = (size_t)comps;
    const size_t stride       = strideFloats * sizeof(float);

    float* vertexData = (float*)malloc(vertexCount * stride);
    for (size_t i = 0; i < vertexCount; ++i) {
        const size_t base = i * strideFloats;
        float tmp[4];

        // positions
        cgltf_accessor_read_float(position, i, tmp, 3);
        vertexData[base + 0] = tmp[0];
        vertexData[base + 1] = tmp[1];
        vertexData[base + 2] = tmp[2];

        // normals 
        if (normals) {
            cgltf_accessor_read_float(normals, i, tmp, 3);
            vertexData[base + 3] = tmp[0];
            vertexData[base + 4] = tmp[1];
            vertexData[base + 5] = tmp[2];
        }

        // uvs
        if (texCoords) {
            const size_t uvOffset = 3 + (normals ? 3 : 0);
            cgltf_accessor_read_float(texCoords, i, tmp, 2);
            vertexData[base + uvOffset + 0] = tmp[0];
            vertexData[base + uvOffset + 1] = tmp[1];
        }
    }

    // indices
    const cgltf_accessor* indexBuffer = prim->indices;
    const size_t indexCount = indexBuffer ? indexBuffer->count : 0;

    unsigned int* indexData = NULL;
    if (indexCount > 0) {
        indexData = (unsigned int*)malloc(indexCount * sizeof(unsigned int));
        uint8_t* baseI = (uint8_t*)get_buffer_data(indexBuffer);

        size_t indexStride = indexBuffer->buffer_view && indexBuffer->buffer_view->stride
            ? indexBuffer->buffer_view->stride
            : (indexBuffer->component_type == cgltf_component_type_r_16u
               ? sizeof(uint16_t) : sizeof(uint32_t));

        for (size_t i = 0; i < indexCount; ++i) {
            if (indexBuffer->component_type == cgltf_component_type_r_16u) {
                uint16_t v;
                memcpy(&v, baseI + i * indexStride, sizeof(v));
                indexData[i] = (unsigned int)v;
            } else {
                memcpy(&indexData[i], baseI + i * indexStride, sizeof(unsigned int));
            }
        }
    }

    // GPU upload
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * stride, vertexData, GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(0));

    // normal
    if (normals) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(3 * sizeof(float)));
    } else {
        glDisableVertexAttribArray(1);
        glVertexAttrib3f(1, 0.0f, 1.0f, 0.0f); // safe default normal
    }

    // uv
    if (texCoords) {
        const size_t uvOffsetBytes = (3 + (normals ? 3 : 0)) * sizeof(float);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)uvOffsetBytes);
    } else {
        glDisableVertexAttribArray(2);
        glVertexAttrib2f(2, 0.0f, 0.0f);
    }

    // EBO
    glGenBuffers(1, &mesh.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    if (indexCount > 0) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indexData, GL_STATIC_DRAW);
        mesh.indexCount = (uint32_t)indexCount;
    } else {
        mesh.indexCount = 0;
    }

    glBindVertexArray(0);

    // cleanup CPU
    free(vertexData);
    free(indexData);

    return mesh;
}



void* get_buffer_data(const cgltf_accessor* accessor)
{
    const cgltf_buffer_view *bv  = accessor->buffer_view;
    const cgltf_buffer      *buf = bv->buffer;
    size_t                   offset = bv->offset + accessor->offset;
    return (uint8_t*)buf->data + offset;
}




