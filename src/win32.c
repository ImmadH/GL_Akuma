//All windows related systems will go here
#include "win32.h"
#include <wingdi.h>
#include <winuser.h>
#include "camera.h"
#include <windowsx.h>
#include "gui.h"

extern Camera camera;
static bool g_mouseLook = false;
static POINT g_lastPos;

const wchar_t* getClassName(void)
{
  return L"WindowClass";
}


//function to manage the window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (gui_wndproc_handler(hwnd, msg, wParam, lParam))
        return 1;

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
        camera.aspect = (float)width / (float)height;
        camera_update_projection(&camera);
        //resize(width, height);
        glViewport(0, 0, width, height);
        return 0;
      }
    break;
    case WM_LBUTTONDOWN:
        g_mouseLook = true;
        SetCapture(hwnd);
        g_lastPos.x = GET_X_LPARAM(lParam);
        g_lastPos.y = GET_Y_LPARAM(lParam);
    return 0;
    case WM_LBUTTONUP:
        g_mouseLook = false;
        ReleaseCapture();
    return 0;
    case WM_MOUSEMOVE:
        if (gui_want_capture_input())
            return 0;  
        if (g_mouseLook) 
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            float xoffset = (float)(x - g_lastPos.x);
            float yoffset = (float)(g_lastPos.y - y); // invert y

            g_lastPos.x = x;
            g_lastPos.y = y;
  
            camera_process_mouse(&camera, xoffset, yoffset);
        }
    return 0;

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
