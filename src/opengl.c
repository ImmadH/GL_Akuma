#include <Windows.h>
#include <wingdi.h>
#include "opengl.h"
#include "glad/glad.h" 
//Function to take a window handle and return a opengl context

#include <Windows.h>
#include <wingdi.h>
#include "opengl.h"
#include "glad/glad.h"

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB 
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
#define WGL_CONTEXT_FLAGS_ARB         0x2094 
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
#endif 
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);

// MSAA
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB        0x2042
#define WGL_DRAW_TO_WINDOW_ARB    0x2001
#define WGL_SUPPORT_OPENGL_ARB    0x2010
#define WGL_DOUBLE_BUFFER_ARB     0x2011
#define WGL_ACCELERATION_ARB      0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_PIXEL_TYPE_ARB        0x2013
#define WGL_TYPE_RGBA_ARB         0x202B
#define WGL_COLOR_BITS_ARB        0x2014
#define WGL_ALPHA_BITS_ARB        0x201B
#define WGL_DEPTH_BITS_ARB        0x2022
#define WGL_STENCIL_BITS_ARB      0x2023

typedef BOOL (WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC)(
    HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static PFNWGLCHOOSEPIXELFORMATARBPROC load_wglChoosePixelFormatARB(void) 
{
    WNDCLASSA wc = {0};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "GLA_dummy";
    RegisterClassA(&wc);

    HWND dummy = CreateWindowA("GLA_dummy","",WS_OVERLAPPEDWINDOW,0,0,16,16,
                               NULL,NULL,wc.hInstance,NULL);
    HDC dc = GetDC(dummy);

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize=sizeof(pfd); pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType=PFD_TYPE_RGBA; pfd.cColorBits=24; pfd.cDepthBits=24; pfd.cStencilBits=8;
    int pf = ChoosePixelFormat(dc,&pfd);
    SetPixelFormat(dc,pf,&pfd);

    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);

    PFNWGLCHOOSEPIXELFORMATARBPROC proc =
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    wglMakeCurrent(NULL,NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
    UnregisterClassA("GLA_dummy", wc.hInstance);
    return proc;
}

HGLRC enableGL(HWND handle, HDC* outHdc)
{
    HDC   hDC   = GetDC(handle);
    HGLRC hRC   = NULL;
    HGLRC temp  = NULL;

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = load_wglChoosePixelFormatARB();

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize=sizeof(pfd); pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType=PFD_TYPE_RGBA; pfd.cColorBits=24; pfd.cAlphaBits=8;
    pfd.cDepthBits=24; pfd.cStencilBits=8;

    int pixelFormat = 0;
    if (wglChoosePixelFormatARB) 
    {
        int iAttrs[] = {
            WGL_DRAW_TO_WINDOW_ARB, 1,
            WGL_SUPPORT_OPENGL_ARB, 1,
            WGL_DOUBLE_BUFFER_ARB,  1,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,     24,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   8,
            WGL_SAMPLE_BUFFERS_ARB, 1,
            WGL_SAMPLES_ARB,        8,
            0
        };
        UINT n=0;
        if (wglChoosePixelFormatARB(hDC, iAttrs, NULL, 1, &pixelFormat, &n) && n>=1) 
        {
            DescribePixelFormat(hDC, pixelFormat, sizeof(pfd), &pfd);
        }
    }
    if (pixelFormat == 0) 
    {
        pixelFormat = ChoosePixelFormat(hDC, &pfd);
    }
    if (!SetPixelFormat(hDC, pixelFormat, &pfd)) 
    {
        ReleaseDC(handle, hDC); return NULL;
    }

    // create temp context
    temp = wglCreateContext(hDC);
    if (!temp) 
    { 
      ReleaseDC(handle,hDC); return NULL; 
    }
    if (!wglMakeCurrent(hDC,temp)) 
    {
        wglDeleteContext(temp); ReleaseDC(handle,hDC); return NULL;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (wglCreateContextAttribsARB) 
    {
        const int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 6,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        hRC = wglCreateContextAttribsARB(hDC, 0, attribs);
    }

    if (hRC) 
    {
        wglMakeCurrent(NULL,NULL);
        wglDeleteContext(temp);
        if (!wglMakeCurrent(hDC, hRC)) 
        {
            wglDeleteContext(hRC); ReleaseDC(handle,hDC); return NULL;
        }
    } 
    else 
    {
      hRC = temp;
    }

    if (!gladLoadGL()) 
    {
        wglMakeCurrent(NULL,NULL); wglDeleteContext(hRC); ReleaseDC(handle,hDC); return NULL;
    }

    glEnable(GL_MULTISAMPLE);

    RECT rc; GetClientRect(handle,&rc);
    glViewport(0,0, rc.right-rc.left, rc.bottom-rc.top);

    *outHdc = hDC;
    return hRC;
}
