#include "skybox.h"
#include "glad/glad.h"
#include <stdio.h>
Skybox skybox_create(void)
{
  Skybox skybox = {0};
  skybox.faces[0] = "assets/skybox/negx.jpg";
  skybox.faces[1] = "assets/skybox/posx.jpg";
  skybox.faces[2] = "assets/skybox/posy.jpg";
  skybox.faces[3] = "assets/skybox/negy.jpg";
  skybox.faces[4] = "assets/skybox/negz.jpg";
  skybox.faces[5] = "assets/skybox/posz.jpg";

  glGenVertexArrays(1, &skybox.skyboxVAO);
  glGenBuffers(1, &skybox.skyboxVBO);
  glBindVertexArray(skybox.skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skybox.skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  skybox.cubemapTexture = load_cubemap(skybox.faces);

  return skybox;
}

uint32_t load_cubemap(const char* faces[6])
{
  uint32_t textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < 6; i++)
  {
      unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
      if (data)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      }
      else
      {
        printf("Failed to load CUBE MAP");
        stbi_image_free(data);
      }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
  
}

void skybox_draw(Skybox* skybox)
{
	glDepthFunc(GL_LEQUAL);
  glDisable(GL_CULL_FACE);
	glBindVertexArray(skybox->skyboxVAO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LESS); 
}

void skybox_destroy(Skybox* skybox)
{
  glDeleteVertexArrays(1, &skybox->skyboxVAO);
  glDeleteBuffers(1, &skybox->skyboxVBO);
  glDeleteTextures(1, &skybox->cubemapTexture);
}


