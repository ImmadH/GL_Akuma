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


Material material_create(const cgltf_material* in, const Texture* textures, uint32_t textureCount)
{
    Material material = {0};
    
    // defaults
    material.ambient[0] = 0.1f;
    material.ambient[1] = 0.1f;
    material.ambient[2] = 0.1f;
    material.diffuse[0] = 1.0f;
    material.diffuse[1] = 1.0f;
    material.diffuse[2] = 1.0f;
    material.specular[0] = 1.0f;
    material.specular[1] = 1.0f;
    material.specular[2] = 1.0f;
    material.shininess = 32.0f;
    material.diffuseTex = 0;  // Default to first texture or create a default
    material.normalTex = UINT32_MAX;  
    
    if (!in) {
        return material;
    }
    
    // 1) Base-color → diffuse & ambient
    float *f = in->pbr_metallic_roughness.base_color_factor;
    material.diffuse[0] = f[0];
    material.diffuse[1] = f[1];
    material.diffuse[2] = f[2];
    
    // ambient = 10% of diffuse
    material.ambient[0] = f[0] * 0.1f;
    material.ambient[1] = f[1] * 0.1f;
    material.ambient[2] = f[2] * 0.1f;
    
    // 2) Shininess from roughness
    float roughness = in->pbr_metallic_roughness.roughness_factor;
    material.shininess = (1.0f - roughness) * 255.0f + 1.0f; 
    
    // 3) Metallic tint on specular (optional)
    float m = in->pbr_metallic_roughness.metallic_factor;
    if (m > 0.0f) {
        material.specular[0] = f[0] * m;
        material.specular[1] = f[1] * m;
        material.specular[2] = f[2] * m;
    }
    
    // 4) Diffuse texture lookup - FIXED
    material.diffuseTex = UINT32_MAX; // Initialize to invalid value to detect unset textures
    
    if (in->pbr_metallic_roughness.base_color_texture.texture) {
        const cgltf_image *img = in->pbr_metallic_roughness.base_color_texture.texture->image;
        
        if (img && img->uri) {
            printf("Looking for diffuse texture: %s\n", img->uri);
            
            // Extract filename from the glTF URI
            const char* uri_filename = strrchr(img->uri, '/');
            if (!uri_filename) uri_filename = strrchr(img->uri, '\\');
            if (!uri_filename) uri_filename = img->uri;
            else uri_filename++; // Skip the slash
            
            // Find which loaded Texture matches this image URI
            bool found = false;
            for (uint32_t i = 0; i < textureCount; ++i) {
                if (textures[i].path) {
                    printf("  Comparing against texture %u: %s\n", i, textures[i].path);
                    
                    // Extract just the filename from the stored path for comparison
                    const char* stored_filename = strrchr(textures[i].path, '/');
                    if (!stored_filename) stored_filename = strrchr(textures[i].path, '\\');
                    if (!stored_filename) stored_filename = textures[i].path;
                    else stored_filename++; // Skip the slash
                    
                    if (strcmp(stored_filename, uri_filename) == 0) {
                        material.diffuseTex = i;  // Store the INDEX, not the OpenGL ID
                        printf("  ✓ Found diffuse texture at index %u (ID: %u)\n", i, textures[i].ID);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                printf("   Diffuse texture NOT FOUND: %s (filename: %s)\n", img->uri, uri_filename);
                material.diffuseTex = 0; // Fallback to first texture
            }
        }
    } else {
        printf("Material has no base color texture, using default (index 0)\n");
        material.diffuseTex = 0; // Use first texture as default
    }
    
    // 5) Normal texture lookup - Ready for future normal mapping support
    if (in->normal_texture.texture) {
        const cgltf_image *img = in->normal_texture.texture->image;
        
        if (img && img->uri) {
            printf("Looking for normal texture: %s\n", img->uri);
            
            // Extract filename from the glTF URI
            const char* uri_filename = strrchr(img->uri, '/');
            if (!uri_filename) uri_filename = strrchr(img->uri, '\\');
            if (!uri_filename) uri_filename = img->uri;
            else uri_filename++; // Skip the slash
            
            for (uint32_t i = 0; i < textureCount; ++i) {
                if (textures[i].path) {
                    const char* stored_filename = strrchr(textures[i].path, '/');
                    if (!stored_filename) stored_filename = strrchr(textures[i].path, '\\');
                    if (!stored_filename) stored_filename = textures[i].path;
                    else stored_filename++; // Skip the slash
                    
                    if (strcmp(stored_filename, uri_filename) == 0) {
                        material.normalTex = i;  // Store the INDEX
                        printf("Found normal texture at index %u (ID: %u)\n", i, textures[i].ID);
                        break;
                    }
                }
            }
        }
    }
    
    return material;
}

Mesh mesh_create(const cgltf_primitive* prim, uint32_t materialIndex)
{
    Mesh mesh = { 0 };
    mesh.materialIndex = materialIndex;

    // we need to pick out the specific accessors
    const cgltf_accessor* position  = NULL;
    const cgltf_accessor* normals   = NULL;
    const cgltf_accessor* texCoords = NULL;

    for (size_t i = 0; i < prim->attributes_count; ++i)
    {
        const cgltf_attribute *attr = &prim->attributes[i];

        switch (attr->type)
        {
            case cgltf_attribute_type_position:
                position  = attr->data;
                break;

            case cgltf_attribute_type_normal:
                normals   = attr->data;
                break;

            case cgltf_attribute_type_texcoord:
                texCoords = attr->data;
                break;

            default:
                break;
        }
    }

    // Must know vertex count for memory allocation
    size_t vertexCount = position->count;

    // calculate stride in floats: pos=3, norm=3, texcoord=2
    int temp = 3;
    if (normals)
    {
        temp += 3;
    }

    if (texCoords)
    {
        temp += 2;
    }

    size_t strideFloats = (size_t)temp;
    size_t stride       = strideFloats * sizeof(float);

    // decide byte-stride per accessor
    size_t posStride;
    if (position->buffer_view->stride)
    {
        posStride = position->buffer_view->stride;
    }
    else
    {
        posStride = 3 * sizeof(float);
    }

    size_t normStride = 0;
    if (normals)
    {
        if (normals->buffer_view->stride)
        {
            normStride = normals->buffer_view->stride;
        }
        else
        {
            normStride = 3 * sizeof(float);
        }
    }

    size_t uvStride = 0;
    if (texCoords)
    {
        if (texCoords->buffer_view->stride)
        {
            uvStride = texCoords->buffer_view->stride;
        }
        else
        {
            uvStride = 2 * sizeof(float);
        }
    }

    // upload vertex Data
    float* vertexData = malloc(vertexCount * strideFloats * sizeof(float));
    for (size_t i = 0; i < vertexCount; ++i)
    {
        size_t base = i * strideFloats;
        uint8_t* source;

        // positions
        source = get_buffer_data(position) + i * posStride;
        memcpy(&vertexData[base + 0], source, 3 * sizeof(float));

        // normals
        if (normals)
        {
            source = get_buffer_data(normals) + i * normStride;
            memcpy(&vertexData[base + 3], source, 3 * sizeof(float));
        }

        // uvs
        if (texCoords)
        {
            size_t uvOffset = 3 + (normals ? 3 : 0);
            source = get_buffer_data(texCoords) + i * uvStride;
            memcpy(&vertexData[base + uvOffset], source, 2 * sizeof(float));
        }
    }

    // INDICES
    const cgltf_accessor* indexBuffer      = prim->indices;
    size_t                indexBufferCount = indexBuffer->count;

    size_t indexStride;
    if (indexBuffer->buffer_view->stride)
    {
        indexStride = indexBuffer->buffer_view->stride;
    }
    else
    {
        if (indexBuffer->component_type == cgltf_component_type_r_16u)
        {
            indexStride = sizeof(uint16_t);
        }
        else
        {
            indexStride = sizeof(uint32_t);
        }
    }

    uint8_t* baseI            = get_buffer_data(indexBuffer);
    unsigned int* indexData   = malloc(indexBufferCount * sizeof(unsigned int));
    for (size_t i = 0; i < indexBufferCount; ++i)
    {
        if (indexBuffer->component_type == cgltf_component_type_r_16u)
        {
            uint16_t v;
            memcpy(&v, baseI + i * indexStride, sizeof(v));
            indexData[i] = v;
        }
        else
        {
            memcpy(&indexData[i], baseI + i * indexStride, sizeof(unsigned int));
        }
    }

    // GPU Upload
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * stride, vertexData, GL_STATIC_DRAW);

    // position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // normal attrib
    if (normals)
    {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    }

    // uv attrib
    if (texCoords)
    {
        size_t uvOffset = (3 + (normals ? 3 : 0)) * sizeof(float);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)uvOffset);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferCount * sizeof(unsigned int), indexData, GL_STATIC_DRAW);

    glBindVertexArray(0);

    mesh.indexCount = (uint32_t)indexBufferCount;

    // cleanup CPU buffers
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




