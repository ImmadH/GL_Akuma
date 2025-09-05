#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <string.h>

typedef struct Shader 
{
  uint32_t ID;
} Shader;


//pass file paths return the shader program to ID with compiled shaders
void shader_create(Shader* shader, const char* vertexPath, const char* fragmentPath);
void shader_delete(Shader* shader);
void shader_use(const Shader* shader);

//utility
uint32_t compile_shader(uint32_t shaderType, const char* shaderSource);
void checkErrors(uint32_t shader, const char* type);
char* read_file(const char* filepath);

