#include "win32.h"
#include "glad/glad.h"
#include "shader.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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
  
  //shader setup

  //Game loop
  int running = 1;
  while (running)
  {
    while(PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
    {
      if (msg.message== WM_QUIT) running = 0;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    //draw here test
    glClearColor(1.0, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    SwapBuffers(window->hDC);
  }

  return  msg.wParam;

}
