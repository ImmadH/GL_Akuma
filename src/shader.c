#include "shader.h"

void shader_create(Shader* shader, const char* vertexPath, const char* fragmentPath)
{
  char* vertexSource = read_file(vertexPath);
  char* fragmentSource = read_file(fragmentPath);
  if (!vertexSource || !fragmentSource)
  {
    printf("Failed to load shaders from file\n");
    shader->ID = 0;
    return;
  }
 
  uint32_t VS, FS;
  VS = compile_shader(GL_VERTEX_SHADER, vertexSource);
  FS = compile_shader(GL_FRAGMENT_SHADER, fragmentSource);

  shader->ID = glCreateProgram();
  glAttachShader(shader->ID, VS);
  glAttachShader(shader->ID, FS);
  glLinkProgram(shader->ID);
  checkErrors(shader->ID, "PROGRAM");
  
  
  glDeleteShader(VS);
  glDeleteShader(FS);

  free(vertexSource);
  free(fragmentSource);
}

void shader_delete(Shader* shader)
{
  glDeleteProgram(shader->ID);
  shader->ID = 0;
}

void shader_use(const Shader* shader)
{
  glUseProgram(shader->ID);
}

uint32_t compile_shader(uint32_t shaderType, const char* shaderSource)
{
  uint32_t shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);
  checkErrors(shader, "SHADER");
  return shader;
}

void checkErrors(uint32_t shader, const char* type)
{
  int success;
  char infoLog[1024];

  if (strcmp(type, "SHADER") == 0)
  { 
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); 
    if (!success)
    {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      printf("Failed to compile shader%s\n", infoLog);
    }
  }
  else
  {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      printf("Failed to Link Program%s\n" , infoLog);
    }
  }
}

char* read_file(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");            
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return NULL; }

    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = '\0';

    if (n >= 3 &&
        (unsigned char)buf[0] == 0xEF &&
        (unsigned char)buf[1] == 0xBB &&
        (unsigned char)buf[2] == 0xBF) {
        memmove(buf, buf + 3, n - 3);
        n -= 3;
        buf[n] = '\0';
    }
    return buf;
}


uint32_t uniform_loc(Shader* shader, const char* name) {
    return glGetUniformLocation(shader->ID, name);
}

void shader_set_bool(Shader* shader, const char* name, int value) {
    glUniform1i(uniform_loc(shader, name), value);
}

void shader_set_int(Shader* shader, const char* name, int value) {
    glUniform1i(uniform_loc(shader, name), value);
}

void shader_set_float(Shader* shader, const char* name, float value) {
    glUniform1f(uniform_loc(shader, name), value);
}

void shader_set_vec3(Shader* shader, const char* name, float x, float y, float z) {
    glUniform3f(uniform_loc(shader, name), x, y, z);
}

void shader_set_vec4(Shader* shader, const char* name, float x, float y, float z, float w) {
    glUniform4f(uniform_loc(shader, name), x, y, z, w);
}

void shader_set_mat4(Shader* shader, const char* name, const float* mat) {
    glUniformMatrix4fv(uniform_loc(shader, name), 1, GL_FALSE, mat);
}


