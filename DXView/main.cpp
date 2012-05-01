#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "visualizer.h"

LRESULT WINAPI	WndProc(HWND, UINT, WPARAM, LPARAM);
void GetFileName(HWND, char*);

Visualizer  glVisualizer;

//---------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Register the window class
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, 
                    GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                    L"NifDX", NULL };
  RegisterClassEx(&wc);

  // Create the application's window
  HWND hWnd = CreateWindow(L"NifDX", L"Nif DirectX Visualization", WS_CAPTION|WS_SYSMENU, 100, 100, 800, 600, GetDesktopWindow(), NULL, wc.hInstance, NULL);

  //  get size of frame window
  char  fileName[1000];
	RECT  rect;
	int   frameWidth (0);
	int   frameHeight(0);

	GetClientRect(hWnd, &rect);
	frameWidth  = rect.right - rect.left;
	frameHeight = rect.bottom - rect.top;

  GetFileName(hWnd, fileName);

  //  initialize visualizer
  if (!glVisualizer.startup(hWnd, frameWidth, frameHeight, fileName))
  {
		MessageBox(NULL, L"Visualizer startup failed.", L"Nif DirectX Visualization", MB_OK );
		return 0;
  }

  // Show the window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
  UpdateWindow(hWnd);

	MSG   msg;
  bool  done(false);

  while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			do
			{
				if (msg.message == WM_QUIT)
				{
					done = true;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			
			} while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
		}

    //  render frame
    glVisualizer.render();
    //Sleep( 5 ); // Avoid a 100% cpu usage... 

	}  //  while (!done)

  //  shutdown visualizer
  glVisualizer.shutdown();
  UnregisterClass(L"NifDX", wc.hInstance);

	return (int) msg.wParam;
}

//---------------------------------------------------------------
LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	HDC             hdc;
  unsigned short  actIndex(-2);

	switch (message) 
	{
    case WM_PAINT:
    {
	    hdc = BeginPaint(hWnd, &ps);
    	
	    //demo.renderFrame();

	    EndPaint(hWnd, &ps);
	    break;
    }

    case WM_DESTROY:
    {
	    PostQuitMessage(0);
	    break;
    }

	  case WM_KEYDOWN:
	  {
		  int   keyCode ((int) wParam);

		  switch (keyCode)
		  {
			  case 103:		{ glVisualizer.increaseRotAngleX(0.5f); break; }
			  case 104:		{ glVisualizer.increaseRotAngleY(0.5f); break; }
			  case 105:		{ glVisualizer.increaseRotAngleZ(0.5f); break; }
			  case 100:		{ glVisualizer.increaseRotAngleX(-0.5f); break; }
			  case 101:		{ glVisualizer.increaseRotAngleY(-0.5f); break; }
			  case 102:		{ glVisualizer.increaseRotAngleZ(-0.5f); break; }
			  case  98:		{ glVisualizer.resetPosition(); break; }
			  case  99:		{ glVisualizer.increaseScale(-0.02f); break; }
			  case  97:		{ glVisualizer.increaseScale(0.02f); break; }
			  case  96:		{ glVisualizer.toggleWireframe(); break; }
			  case 107:		{ actIndex = glVisualizer.increaseActIndex(2); break; }
			  case 109:		{ actIndex = glVisualizer.increaseActIndex(-2); break; }
			  case 111:		{ actIndex = glVisualizer.increaseActIndex(10); break; }
			  case 106:		{ actIndex = glVisualizer.increaseActIndex(-10); break; }
			  case  13:		{ actIndex = glVisualizer.increaseActIndex(0); break; }
			  case  27:		{ DestroyWindow(hWnd); break; }
		  }
		  break;
	  }

    default:
    {
	    return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }  //  switch (message)

  if (actIndex > -1)
  {
    char  cbuffer[1000];

    sprintf(cbuffer, "Nif DirectX Visualization - Face: %d", actIndex);
    SetWindowTextA(hWnd, cbuffer);
  }
  else if (actIndex == -1)
  {
    SetWindowText(hWnd, L"Nif DirectX Visualization");
  }


  return 0;
}

void GetFileName(HWND owner, char* pFileName)
{
  OPENFILENAMEA ofn;
  char  fileName[1000] = "";

  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner   = owner;
  ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
  ofn.lpstrFile   = fileName;
  ofn.nMaxFile    = MAX_PATH;
  ofn.Flags       = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  ofn.lpstrDefExt = "";

  if (GetOpenFileNameA(&ofn))
  {
    strncpy(pFileName, fileName, 1000);
  }
}
