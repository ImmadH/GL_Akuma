//All windows related systems will go here
#include <Windows.h>
#include <winuser.h>
#include <wingdi.h>
#include "opengl.h" 
#include "glad/glad.h"
#include <stdio.h>

typedef struct Window 
{
  HWND handle;
  HINSTANCE hInstance;
  int width;
  int height;
  const wchar_t* title;
  const wchar_t* className;
  HDC hDC;
  HGLRC openglContext;
} Window;


const wchar_t* getClassName()
{
  return L"WindowClass";
}

//function to manage the window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CLOSE:
      DestroyWindow(hwnd);
    break;
    case WM_DESTROY:
      PostQuitMessage(0);
    break;
    case WM_ERASEBKGND:
      return 1;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}


//Function to initialize the window class(A template?)
static void register_class()
{
  WNDCLASSEX window_class = {0};
  window_class.cbSize = sizeof(WNDCLASSEX);
  window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = WndProc;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = GetModuleHandle(NULL);
  window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.hbrBackground = NULL;
  window_class.lpszClassName =  getClassName();
  window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if(!RegisterClassEx(&window_class))
  {
      MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
  }
}

Window* create_window()
{
  //populate the window struct 
  //and do creation
  static Window mainWindow;
  mainWindow.width = 800;
  mainWindow.height = 600;
  mainWindow.title = L"A window";
  mainWindow.className = getClassName();
  mainWindow.hInstance = GetModuleHandle(NULL); 
  
  mainWindow.handle = CreateWindowEx(
                    WS_EX_CLIENTEDGE,
                    mainWindow.className,
                    mainWindow.title,
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    mainWindow.width, mainWindow.height,
                    NULL, NULL, mainWindow.hInstance, NULL);

  mainWindow.openglContext = enableGL(mainWindow.handle, &mainWindow.hDC);


  return &mainWindow;
}


//we need a main entry point that will grab from the populated struct MyStruct 
//function to create the actual window
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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


