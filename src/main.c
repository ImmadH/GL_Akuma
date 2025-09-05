#include "win32.h"
#include "glad/glad.h"
#include "shader.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  AllocConsole();
  freopen("CONOUT$", "w", stdout);
  //WINDOW SETUP
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(hInstance);

  MSG msg = {0};
  register_class();
  Window* window = create_window();
  if (window->handle == NULL)
  {
    MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  ShowWindow(window->handle, nCmdShow);
  UpdateWindow(window->handle);
  
  //triangle test
  //shader setup
  Shader mainShader;
  shader_create(&mainShader, "shaders/vert.glsl", "shaders/frag.glsl");

  float vertices[] = 
  {
    -0.5f, -0.5f, 0.0f, // left  
     0.5f, -0.5f, 0.0f, // right 
     0.0f,  0.5f, 0.0f  // top   
  };
  

  uint32_t VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);



  //Game loop
  bool running = true;
  while (running)
  {
    while(PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
    {
      if (msg.message== WM_QUIT) 
        running = false;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    //draw here test
    glClearColor(0.7, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader_use(&mainShader);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEnableVertexAttribArray(0);


    SwapBuffers(window->hDC);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  shader_delete(&mainShader);
  return  msg.wParam;

}
