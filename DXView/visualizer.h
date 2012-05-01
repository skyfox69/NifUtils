#include <windows.h>
#include <d3dx9.h>
#include "FDMesh.h"


class Visualizer
{
  private:
            FDMesh					        _mesh;
            LPDIRECT3D9             _pD3D;			  // Used to create the D3DDevice
            LPDIRECT3DDEVICE9       _pd3dDevice;	// Our rendering device
            LPDIRECT3DVERTEXBUFFER9 _pVBuffer;	  // VertexBuffer to hold vertices
            LPDIRECT3DINDEXBUFFER9  _pIBuffer;		// IndexBuffer to hold face indexes
            D3DXMATRIX              _matProj;
            D3DXMATRIX              _matView;
            D3DXMATRIX              _matWorld;
            HWND                    _hWnd;
            float                   _rotAngleX;
            float                   _rotAngleY;
            float                   _rotAngleZ;
            float					          _scale;
            int                     _width;
            int                     _height;
            int						          _actIndex;
            DWORD					          _actColor1;
            DWORD					          _actColor2;
            DWORD					          _actColor3;
            bool                    _initialized;
            bool					          _wireframe;


    virtual bool  createRenderingContext(HWND hWnd, const int width, const int height);
    virtual bool  destroyRenderingContext();
    virtual bool  initScene(const char* pFileName);
    virtual bool  cleanupScene();
    virtual bool  beginScene();
    virtual bool  endScene();
    virtual bool  renderScene();
    virtual bool  createMesh();

  public:
                  Visualizer();
    virtual       ~Visualizer();

    virtual bool            startup(HWND hWnd, const int width, const int height, const char* pFileName);
    virtual bool            shutdown();
    virtual bool            render();
	  virtual void            increaseRotAngleX(const float degree);
	  virtual void            increaseRotAngleY(const float degree);
	  virtual void            increaseRotAngleZ(const float degree);
	  virtual void            increaseScale    (const float factor);
	  virtual unsigned short  increaseActIndex (const int delta);
	  virtual void            resetPosition();
	  virtual void            toggleWireframe() { _wireframe = !_wireframe; }
};
