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
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* buffer = malloc(length + 1); // +1 for null terminator
    if (!buffer) return NULL;

    fread(buffer, 1, length, file);
    buffer[length] = '\0'; 

    fclose(file);
    return buffer;
}

