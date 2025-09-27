#include "cglm/affine.h"
#include "cglm/cglm.h"
#include "cglm/vec3.h"
#include "win32.h"
#include "glad/glad.h"
#include "shader.h"
#include "camera.h"
#include <winuser.h>
#include "skybox.h"
#include "gui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "scene.h"
#include "shadow.h"

void process_input(Camera* camera);
static ShadowSystem gShadows;
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
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  
  //GUI 
  gui_init(window->handle, "#version 460");
  bool wireFrame = false;
  bool enableMSAA = true;
  bool unlit= false;
  bool toggleNight = false;
  bool toggleSkybox = true;


  //Camera 
  vec3 startPos = {0.0f, 0.0f, 3.0f};
  vec3 upVector = {0.0f, 1.0f, 0.0f};
  camera_init(&camera, startPos, upVector, -90.0f, 0.0f);
  camera_set_perspective(&camera, 70.0f, (float)width / (float)height, 0.1f, 100.0f);
  glViewport(0, 0, width, height);

  Skybox skybox = skybox_create();


  //shader setup TODO make this return a shader object so we can clean this up 
  Shader mainShader;
  Shader skyboxShader;
  shader_create(&mainShader, "shaders/shader.vert", "shaders/shader.frag");
  vec3 lightDir = { 1.0f, -0.1f, -0.48f };
  glm_normalize(lightDir);
  shader_create(&skyboxShader, "shaders/skybox.vert", "shaders/skybox.frag");
  
  glUseProgram(skyboxShader.ID);
  uint32_t skyboxUniform = glGetUniformLocation(skyboxShader.ID, "skybox");
  glUniform1i(skyboxUniform, 0);

  //SHADOW MAPS
  shadow_init(&gShadows, 4096, 4096,
            "shaders/shadow.vert",
            "shaders/shadow.frag");

  Scene scene;
  scene_init(&scene);

  static char* gModels[] = {
    "assets/miguel/scene.gltf",
    "assets/model/scene.gltf",
    "assets/hoonam/scene.gltf",
    "assets/bistro/scene.gltf",
    "assets/sponza/sponza.gltf",
    "assets/bunny/bunny.gltf",
    "assets/new/scene.gltf"
  };
  static int gSel = 0;


  //Game loop
  bool running = true;
  while (running)
  { 
    GetClientRect(window->handle, &rect);
    int widthDynamic  = rect.right - rect.left;
    int heightDynamic = rect.bottom - rect.top;
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

    //options check before any drawing 
    if(wireFrame)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (enableMSAA) 
      glEnable(GL_MULTISAMPLE);
    else 
      glDisable(GL_MULTISAMPLE);
    

    scene_update(&scene);
    {
      vec3 dir;
      glm_vec3_copy(lightDir, dir);
      glm_normalize(dir);

      vec3 focus = { camera.position[0], camera.position[1], camera.position[2] };
      shadow_update_dir(&gShadows, dir, focus, 50.f, 0.1f, 150.f);
      shadow_begin(&gShadows);
      for (unsigned i = 0; i < scene.count; ++i) {
          const SceneInstance* inst = &scene.items[i];
          if (!inst->mainModel) continue;
          shadow_submit_model(&gShadows, inst->mainModel, inst->model);
      }
      shadow_end(&gShadows, widthDynamic, heightDynamic);
    }
    shader_use(&mainShader);

    mat4 viewMtx;
    camera_get_view_matrix(&camera, viewMtx);
    

    shader_use(&mainShader);// back to window
    
    shader_set_mat4(&mainShader, "uProjection", (float*)camera.projection);
    shader_set_mat4(&mainShader, "uView", (float*)viewMtx);
    
    shader_set_vec3(&mainShader, "uViewPos", camera.position[0],camera.position[1],camera.position[2]);
    shader_set_vec3(&mainShader, "uLightColor", 1.0f, 1.0f, 1.0f);


    if (toggleNight) shader_set_vec3(&mainShader, "uLightColor", 0.0f, 0.0f, 0.0f);
    else             shader_set_vec3(&mainShader, "uLightColor", 1.0f, 1.0f, 1.0f);

    vec3 dir;
    glm_vec3_copy(lightDir, dir);
    glm_normalize(dir);
    shader_set_vec3(&mainShader, "uLightDir", dir[0], dir[1], dir[2]);
    
    shader_set_int(&mainShader, "uShadowMap", shadow_bind_depth(&gShadows, 1));
    shader_set_mat4(&mainShader, "uLightVP",  shadow_light_vp(&gShadows));


    if(unlit)
      shader_set_float(&mainShader, "uShininess", 0.0f);
    else if(toggleNight)
      shader_set_float(&mainShader, "uShininess", -1.0f);
    else
     shader_set_float(&mainShader, "uShininess", 16.0f);


    //SCENE DRAWING 
    scene_draw(&scene, mainShader.ID);

    //Skybox
    if(toggleSkybox)
    {
      glUseProgram(skyboxShader.ID);
      mat4 viewNoTrans;
      glm_mat4_copy(viewMtx, viewNoTrans);
      viewNoTrans[3][0] = 0.0f;
      viewNoTrans[3][1] = 0.0f;
      viewNoTrans[3][2] = 0.0f;

      shader_set_mat4(&skyboxShader, "view", (float*)viewNoTrans);
      shader_set_mat4(&skyboxShader, "projection", (float*)camera.projection);
      glActiveTexture(GL_TEXTURE0);
      skybox_draw(&skybox);
    }

    gui_new_frame();
    igBegin("Akuma - OpenGL 4.6 Renderer", NULL, 0);
    scene_ui(&scene, gModels, (int)(sizeof gModels / sizeof gModels[0]), &gSel);
    igSeparator();
    igText("Settings");
    igCheckbox("Wireframe", &wireFrame);
    igCheckbox("MSAA 8X", &enableMSAA);
    igSliderFloat3("Light Direction", lightDir, -1.0f, 1.0f, "%.2f", 0);
    igCheckbox("Toggle Unlit", &unlit);
    igCheckbox("Toggle Night", &toggleNight);
    igCheckbox("Toggle Skybox", &toggleSkybox);

    igEnd();
    gui_render();



    SwapBuffers(window->hDC);
  }

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


