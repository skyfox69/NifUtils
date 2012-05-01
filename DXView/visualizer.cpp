#include "visualizer.h"

#define D3DFVF_CUSTOMVERTEX D3DFVF_XYZ | D3DFVF_DIFFUSE

//-----------------------------------------------------------------------------
Visualizer::Visualizer()
  : _pD3D       (NULL),
    _pd3dDevice (NULL),
    _pVBuffer   (NULL),
    _pIBuffer   (NULL),
    _hWnd       (NULL),
    _rotAngleX  (0.0),
    _rotAngleY  (0.0),
    _rotAngleZ  (0.0),
    _scale      (0.15f),
    _width      (0),
    _height     (0),
    _actIndex   (-1),
    _actColor1  (TRI_HIGH_COLOR),
    _actColor2  (TRI_HIGH_COLOR),
    _actColor3  (TRI_HIGH_COLOR),
    _initialized(false),
    _wireframe  (true)
{

}

//-----------------------------------------------------------------------------
Visualizer::~Visualizer()
{
  shutdown();
}

//-----------------------------------------------------------------------------
bool Visualizer::startup(HWND hWnd, const int width, const int height, const char* pFileName)
{
  if (!createRenderingContext(hWnd, width, height))
  {
    shutdown();
    return false;
  }

  if (!initScene(pFileName))
  {
    shutdown();
    return false;
  }

  _initialized = true;

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::shutdown()
{
  _initialized = false;
  cleanupScene();
  destroyRenderingContext();

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::createRenderingContext(HWND hWnd, const int width, const int height)
{
  D3DDISPLAYMODE        d3ddm;
  D3DPRESENT_PARAMETERS d3dpp;

  _hWnd   = hWnd;
  _width  = width;
  _height = height;

  // Create the D3D object.
  if ((_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
  {
    return false;
  }

  // Get the current desktop display mode, so we can set up a back
  // buffer of the same format
  if (FAILED(_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
  {
    return false;
  }

  // Set up the structure used to create the D3DDevice
  ZeroMemory(&d3dpp, sizeof(d3dpp));
  d3dpp.Windowed         = TRUE;
  d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat = d3ddm.Format;

  // Create the D3DDevice
  if (FAILED(_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &_pd3dDevice)))
  {
	  return false;
  }

  resetPosition();

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::destroyRenderingContext()
{
  if (_pd3dDevice != NULL)
	{
//    _pd3dDevice->Release();
	}

  if(_pD3D != NULL)
	{
//    _pD3D->Release();
	}
	
	_hWnd   = NULL;
	_width  = 0;
	_height = 0;

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::initScene(const char* pFileName)
{
	// --------------------------------------------------------------
	// Init viewport.
	// --------------------------------------------------------------
	D3DVIEWPORT9 vp;

	vp.X      = 0;
	vp.Y      = 0;
	vp.Width  = _width;
	vp.Height = _height;
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f;
	_pd3dDevice->SetViewport(&vp);
	
	// --------------------------------------------------------------
	// Init camera.
	// --------------------------------------------------------------
  // Init the camera type (orthographic or perspective)
	// Here, perspective camera.
	float   fov    = 1.04f; // 60 degrees = 1.04 radian.
	float   aspect = (float)_width/(float)_height;
	float   znear  = 0.1f;
	float   zfar   = 4512.0f;

  D3DXMatrixPerspectiveFovLH(&_matProj, fov, aspect, znear, zfar);
  _pd3dDevice->SetTransform(D3DTS_PROJECTION, &_matProj);

  // Init camera view matrix
  D3DXMatrixLookAtLH(&_matView, &D3DXVECTOR3(0.0f, 2050.0f, 2550.0f), // camera position
//								&D3DXVECTOR3(0.0f, 0.0f, -1500.0f),   // target position
                                &D3DXVECTOR3(0.0f, 0.0f, 0.0f),   // target position
                                &D3DXVECTOR3(0.0f, 1.0f, 0.0f));  // camera up vector
  _pd3dDevice->SetTransform(D3DTS_VIEW, &_matView);

	// Turn off culling, so we see the front and back of the triangle
  _pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  // Turn off D3D lighting, since we are providing our own vertex colors
  _pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

  //_mesh.readMeshFromFile("C:\\sourcen\\NifDX\\NifDX\\meshData.txt");
  //if (!_mesh.readMeshFromFile("F:\\Sourcen\\NifTools\\DXView\\data\\meshData_hlaalu04.txt"))
  if (!_mesh.readMeshFromFile(pFileName))
  {
	  return false;
  }

  createMesh();

	// Wireframe render
	_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	
	// Turn off culling
  _pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::cleanupScene()
{



  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::render()
{
  if (!_initialized)    return false;

  if (_wireframe)
  {
	_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  }
  else
  {
	_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  }

	beginScene();
	renderScene();
	endScene();

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::beginScene()
{
  //  clear background
  _pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x20, 0x00, 0x20), 1.0f, 0);

  //  Begin the scene
  _pd3dDevice->BeginScene();

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::endScene()
{
  // End the scene
  _pd3dDevice->EndScene();

  // Present the backbuffer contents to the display
  _pd3dDevice->Present(NULL, NULL, NULL, NULL);

  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::renderScene()
{
  D3DXMATRIX		matScale;
  D3DXMATRIX		matRotX;
  D3DXMATRIX		matRotY;
  D3DXMATRIX		matRotZ;
  D3DXMATRIX		matTrans;
  int               numVertices(_mesh.getNumVertices());
  int               numFaces   (_mesh.getNumIndices());

  D3DXMatrixRotationX(&matRotX, -_rotAngleX*3.14159f/180.0f); // m_rotAng in degrees!!!
  D3DXMatrixRotationY(&matRotY, -_rotAngleY*3.14159f/180.0f); // m_rotAng in degrees!!!
  D3DXMatrixRotationZ(&matRotZ, -_rotAngleZ*3.14159f/180.0f); // m_rotAng in degrees!!!
  D3DXMatrixScaling(&matScale, _scale, _scale, _scale);
  D3DXMatrixTranslation(&matTrans, _mesh.getCenter()._x, _mesh.getCenter()._y, _mesh.getCenter()._z);

  _matWorld = matTrans * (matRotZ*matRotX*matRotY) * matScale;

  _pd3dDevice->SetTransform(D3DTS_WORLD, &_matWorld);

  _pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertex));
  _pd3dDevice->SetIndices          (_pIBuffer);
  _pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX);
  _pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVertices, 0, numFaces);


  return true;
}

//-----------------------------------------------------------------------------
bool Visualizer::createMesh()
{
  D3DCustomVertex*  pVertexBuffer(NULL);
  void*             pVertices    (NULL);
  void*             pIndices     (NULL);
//  int               numVertices  (4);
//  int               numFaces     (2);
  int				numVertices  (_mesh.getNumVertices());
  int				numFaces     (_mesh.getNumIndices());

	// create vertex buffer.
  _pd3dDevice->CreateVertexBuffer(numVertices*sizeof(D3DCustomVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);

  //  copy vertices
  _pVBuffer->Lock(0, 0, (void**)&pVertices, 0);
  pVertexBuffer = _mesh.getVertices();
  memcpy(pVertices, pVertexBuffer, numVertices*sizeof(D3DCustomVertex));
  _pVBuffer->Unlock();

	// create index buffer
	_pd3dDevice->CreateIndexBuffer(numFaces*3*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
	_pIBuffer->Lock(0, 0, (void**)&pIndices, 0);

    // fill the index buffer.
	unsigned short* pIndex = (unsigned short*) pIndices;

	memcpy(pIndex, _mesh.getIndices(), numFaces*sizeof(unsigned short));
	_pIBuffer->Unlock();

  return true;
}

//-----------------------------------------------------------------------------
void Visualizer::increaseRotAngleX(const float degree)
{
	_rotAngleX += degree;
	if (_rotAngleX >= 360.0)	_rotAngleX = 0.0;
}

//-----------------------------------------------------------------------------
void Visualizer::increaseRotAngleY(const float degree)
{
	_rotAngleY += degree;
	if (_rotAngleY >= 360.0)	_rotAngleY = 0.0;
}

//-----------------------------------------------------------------------------
void Visualizer::increaseRotAngleZ(const float degree)
{
	_rotAngleZ += degree;
	if (_rotAngleZ >= 360.0)	_rotAngleZ = 0.0;
}

//-----------------------------------------------------------------------------
void Visualizer::increaseScale(const float factor)
{
	_scale += factor;
}

//-----------------------------------------------------------------------------
void Visualizer::resetPosition()
{
	_rotAngleX = 0.0f;
	_rotAngleY = 0.0f;
	_rotAngleZ = 0.0f;
	_scale     = 0.15f;
}

//-----------------------------------------------------------------------------
unsigned short Visualizer::increaseActIndex(const int delta)
{
	D3DCustomVertex*	pVBuffer(NULL);
	unsigned short*		pIBuffer(NULL);
	void*				      pIndices(NULL);
	int					      oldIndex(_actIndex);
	int					      numFaces(_mesh.getNumIndices() / 3);
	short				      idxOldV1(-1);
	short				      idxOldV2(-1);
	short				      idxOldV3(-1);
	short				      idxNewV1(-1);
	short				      idxNewV2(-1);
	short				      idxNewV3(-1);

	//  show/hide triangle
	if (delta == 0)
	{
		_actIndex = (_actIndex == -1) ? 0 : -1;
	}
	//  increase index
	else
	{
		_actIndex += delta;
		if (_actIndex >= numFaces)		_actIndex = 0;
		if (_actIndex < 0)				    _actIndex = numFaces - 1;
	}

	_pIBuffer->Lock(0, 0, (void**)&pIndices, 0);
	pIBuffer = (unsigned short*) pIndices;

	if (oldIndex >= 0)
	{
		idxOldV1 = pIBuffer[(oldIndex * 3) + 0];
		idxOldV2 = pIBuffer[(oldIndex * 3) + 1];
		idxOldV3 = pIBuffer[(oldIndex * 3) + 2];
	}
	if (_actIndex >= 0)
	{
		idxNewV1 = pIBuffer[(_actIndex * 3) + 0];
		idxNewV2 = pIBuffer[(_actIndex * 3) + 1];
		idxNewV3 = pIBuffer[(_actIndex * 3) + 2];
	}

	_pIBuffer->Unlock();

	_pVBuffer->Lock(0, 0, (void**)&pIndices, 0);
	pVBuffer = (D3DCustomVertex*) pIndices;

	if (oldIndex >= 0)
	{
		pVBuffer[idxOldV1]._color = _actColor1;
		pVBuffer[idxOldV2]._color = _actColor2;
		pVBuffer[idxOldV3]._color = _actColor3;
	}
	if (_actIndex >= 0)
	{
		_actColor1 = pVBuffer[idxNewV1]._color;
		_actColor2 = pVBuffer[idxNewV2]._color;
		_actColor3 = pVBuffer[idxNewV3]._color;
		pVBuffer[idxNewV1]._color = TRI_HIGH_COLOR;
		pVBuffer[idxNewV2]._color = TRI_HIGH_COLOR;
		pVBuffer[idxNewV3]._color = TRI_HIGH_COLOR;
	}

	_pVBuffer->Unlock();

  return _mesh.getFaceNumByIndex(_actIndex);
}

