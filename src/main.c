#include "win32.h"
#include "glad/glad.h"
#include "shader.h"
#include "camera.h"
#include <winuser.h>

void process_input(Camera* camera);
Camera camera;


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
  
  RECT rect;
  GetClientRect(window->handle, &rect);
  int width  = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  //Camera 
  vec3 startPos = {0.0f, 0.0f, 3.0f};
  vec3 upVector = {0.0f, 1.0f, 0.0f};
  camera_init(&camera, startPos, upVector, -90.0f, 0.0f);
  camera_set_perspective(&camera, 45.0f, (float)width / (float)height, 0.1f, 100.0f);
  glViewport(0, 0, width, height);


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
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  

  //Game loop
  bool running = true;
  while (running)
  { 
    process_input(&camera);
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
    mat4 viewMtx;
    camera_get_view_matrix(&camera, viewMtx);

    GLint loc = glGetUniformLocation(mainShader.ID, "uProjection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)camera.projection);

    loc = glGetUniformLocation(mainShader.ID, "uView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)viewMtx);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);


    SwapBuffers(window->hDC);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  shader_delete(&mainShader);
  return  msg.wParam;

}


void process_input(Camera* camera)
{
    static LARGE_INTEGER freq;
    static LARGE_INTEGER prev;
    static bool init = false;

    if (!init) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&prev);
        init = true;
    }

    // compute delta time
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float deltaTime = (float)((now.QuadPart - prev.QuadPart) / (double)freq.QuadPart);
    prev = now;

    // WASD movement
    if (GetAsyncKeyState('W') & 0x8000) camera_process_keyboard(camera, FORWARD,  deltaTime);
    if (GetAsyncKeyState('S') & 0x8000) camera_process_keyboard(camera, BACKWARD, deltaTime);
    if (GetAsyncKeyState('A') & 0x8000) camera_process_keyboard(camera, LEFT,     deltaTime);
    if (GetAsyncKeyState('D') & 0x8000) camera_process_keyboard(camera, RIGHT,    deltaTime);
}
