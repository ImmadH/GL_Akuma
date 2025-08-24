//All windows related systems will go here
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <winuser.h>

typedef struct Window 
{
  HWND handle;
  HINSTANCE hInstance;
  int width;
  int height;
  const wchar_t* title;
  const wchar_t* className;
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
  window_class.style = 0;
  window_class.lpfnWndProc = WndProc;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = GetModuleHandle(NULL);
  window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
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
  return &mainWindow;
}


//we need a main entry point that will grab from the populated struct MyStruct 
//function to create the actual window
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  
  MSG msg;
  register_class();
  Window* window = create_window();
  if (window->handle == NULL)
  {
    MessageBox(NULL, L"Window Creation Failed!", L"Error!",
    MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  ShowWindow(window->handle, nCmdShow);
  UpdateWindow(window->handle);


  while (GetMessage(&msg, NULL, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return  msg.wParam;

}


