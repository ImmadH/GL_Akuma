#include "cglm/affine.h"
#include "win32.h"
#include "glad/glad.h"
#include "shader.h"
#include "camera.h"
#include <winuser.h>
#include "model.h"
#include "skybox.h"
#include "gui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

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

  //GLSETUP
  glEnable(GL_DEPTH_TEST);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  
  //GUI 
  gui_init(window->handle, "#version 460");



  //Camera 
  vec3 startPos = {0.0f, 0.0f, 3.0f};
  vec3 upVector = {0.0f, 1.0f, 0.0f};
  camera_init(&camera, startPos, upVector, -90.0f, 0.0f);
  camera_set_perspective(&camera, 70.0f, (float)width / (float)height, 0.1f, 100.0f);
  glViewport(0, 0, width, height);

  Skybox skybox = skybox_create();


  //shader setup
  Shader mainShader;
  Shader skyboxShader;
  shader_create(&mainShader, "shaders/shader.vert", "shaders/shader.frag");
  shader_create(&skyboxShader, "shaders/skybox.vert", "shaders/skybox.frag");
  
  glUseProgram(skyboxShader.ID);
  uint32_t skyboxUniform = glGetUniformLocation(skyboxShader.ID, "skybox");
  glUniform1i(skyboxUniform, 0);


  glUseProgram(mainShader.ID);
  glUniform1i(glGetUniformLocation(mainShader.ID, "uTex0"), 0);
  Model* coolModel = model_load("assets/miguel/scene.gltf");
  if(!coolModel)
  {
    printf("FAILED TO LOAD");
  }


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
    glClearColor(0.0, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader_use(&mainShader);
    mat4 viewMtx;
    mat4 modelMtx;
    glm_mat4_identity(modelMtx);
    //glm_scale(modelMtx, (vec3){ 0.01f, 0.01f, 0.01f });
    glm_rotate_x(modelMtx, glm_rad(90.0f), modelMtx);
    camera_get_view_matrix(&camera, viewMtx);
    
    GLint loc = glGetUniformLocation(mainShader.ID, "uProjection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)camera.projection);

    loc = glGetUniformLocation(mainShader.ID, "uView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)viewMtx);

    uint32_t locModel = glGetUniformLocation(mainShader.ID, "model");
    glUniformMatrix4fv(locModel, 1, GL_FALSE, (float*)modelMtx);

    model_draw(coolModel, mainShader.ID);
      

    //SKYBOX 
    glUseProgram(skyboxShader.ID);
    mat4 viewNoTrans;
    glm_mat4_copy(viewMtx, viewNoTrans);
    viewNoTrans[3][0] = 0.0f;
    viewNoTrans[3][1] = 0.0f;
    viewNoTrans[3][2] = 0.0f;

    uint32_t skyboxView = glGetUniformLocation(skyboxShader.ID, "view");
    glUniformMatrix4fv(skyboxView, 1, GL_FALSE, (float*)viewNoTrans);

    uint32_t skyboxProj = glGetUniformLocation(skyboxShader.ID, "projection");
    glUniformMatrix4fv(skyboxProj, 1, GL_FALSE, (float*)camera.projection);
    glActiveTexture(GL_TEXTURE0);
    skybox_draw(&skybox);
    

    gui_new_frame();
    igBegin("Hello", NULL, 0);
    igText("It works!"); 
    igEnd();
    gui_render();



    SwapBuffers(window->hDC);
  }

  model_destroy(coolModel);
  shader_delete(&mainShader);
  shader_delete(&skyboxShader);
  skybox_destroy(&skybox);
  gui_shutdown();
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


