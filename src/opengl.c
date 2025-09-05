#include <Windows.h>
#include <wingdi.h>
#include "opengl.h"
#include "glad/glad.h" 
//Function to take a window handle and return a opengl context

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB 
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_FLAGS_ARB 0x2094 
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001 
#endif 
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);


HGLRC enableGL(HWND handle, HDC* outHdc)
{
    HDC   hDC   = NULL;
    HGLRC hRC   = NULL;
    HGLRC temp  = NULL;
    int   pf    = 0;

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));

    hDC = GetDC(handle);

    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 24;
    pfd.cAlphaBits   = 8;
    pfd.cDepthBits   = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType   = PFD_MAIN_PLANE;

    pf = ChoosePixelFormat(hDC, &pfd);
    if (!pf || !SetPixelFormat(hDC, pf, &pfd))
    {
        ReleaseDC(handle, hDC);
        return NULL;
    }

    temp = wglCreateContext(hDC);
    if (!temp)
    {
        ReleaseDC(handle, hDC);
        return NULL;
    }

    if (!wglMakeCurrent(hDC, temp))
    {
        wglDeleteContext(temp);
        ReleaseDC(handle, hDC);
        return NULL;
    }

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (wglCreateContextAttribsARB)
    {
        const int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 6,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        hRC = wglCreateContextAttribsARB(hDC, 0, attribs);
    }

    if (hRC)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(temp);

        if (!wglMakeCurrent(hDC, hRC))
        {
            wglDeleteContext(hRC);
            ReleaseDC(handle, hDC);
            return NULL;
        }
    }
    else
    {
        hRC = temp;
    }

    if (!gladLoadGL())
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
        ReleaseDC(handle, hDC);
        return NULL;
    }

    RECT rc;
    GetClientRect(handle, &rc);
    glViewport(0, 0, rc.right - rc.left, rc.bottom - rc.top);

    *outHdc = hDC;
    return hRC;
}
