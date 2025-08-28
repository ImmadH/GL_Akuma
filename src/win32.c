//All windows related systems will go here
#include "win32.h"
#include <wingdi.h>

const wchar_t* getClassName(void)
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
    break;
    case WM_SIZE: 
      {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);
      }
    break;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}


//Function to initialize the window class(A template?)
void register_class(void)
{
  WNDCLASSEX window_class = {0};
  window_class.cbSize = sizeof(WNDCLASSEX);
  window_class.style = CS_OWNDC;
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

Window* create_window(void)
{
  //populate the window struct 
  //and do creation
  static Window mainWindow;
  mainWindow.width = 800;
  mainWindow.height = 600;
  mainWindow.title = L"Akuma - OpenGL 4.6";
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



