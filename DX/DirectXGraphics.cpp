#include "..\Common\stdafx.h"
#include "..\Common\Configuration.h"
#include "DirectXGraphics.h"
#include "DirectXVertex.h"
#include "DirectXMeshAxes.h"
#include "DirectXMeshModel.h"

using namespace NifUtility;

extern Configuration	glConfig;

CDirectXGraphics::CDirectXGraphics()
	:	_pd3dDevice   (NULL),
		_vecViewCam   (0.0f, 0.0f, 0.0f),
		_posX         (0.0f),
		_posY         (0.0f),
		_zoom         (1.0f),
		_rotX         (0.0f),
		_rotY         (0.0f),
		_showAxes     (true),
		_showModel    (true)
{
}

CDirectXGraphics::~CDirectXGraphics()
{
	dxShutdown();

	//  remove and delete objects
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end();)
	{
		delete *pIter;
		pIter = _meshList.erase(pIter);
	}
}

void CDirectXGraphics::dxCreate(CRect rect, CWnd *pParent, CWnd* pSelf)
{
	//  register class
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC, NULL, (HBRUSH)GetStockObject(BLACK_BRUSH), NULL);

	//  store original area for later usage
	_rectOrig = rect;

	//  call own create function (MFC)
	pSelf->CreateEx(0, className, _T("DirectX"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rect, pParent, 0);
}

bool CDirectXGraphics::dxShutdown()
{
	//  do some clean-up
	dxCleanupScene();
	dxDestroyRenderingContext();

	return true;
}

bool CDirectXGraphics::dxCreateRenderingContext(HWND hWnd, const int width, const int height)
{
	D3DDISPLAYMODE        d3ddm;
	D3DPRESENT_PARAMETERS d3dpp;

	//  Create the D3D object.
	if ((_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		return false;
	}

	//  Get the current desktop display mode, so we can set up a back
	//  buffer of the same format
	if (FAILED(_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
	{
		return false;
	}

	//  Set up the structure used to create the D3DDevice
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed               = true;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.hDeviceWindow          = hWnd;
	d3dpp.BackBufferWidth        = width;
	d3dpp.BackBufferHeight       = height;
	d3dpp.BackBufferFormat       = d3ddm.Format;
	d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;

	//  Create the D3DDevice
	if (FAILED(_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &_pd3dDevice)))
	{
		return false;
	}

	return true;
}

bool CDirectXGraphics::dxDestroyRenderingContext()
{
	//  destroy DirectX-Device
	if (_pd3dDevice != NULL)		_pd3dDevice->Release();

	//  destroy DirectX-Handle
	if(_pD3D != NULL)				_pD3D->Release();
	
	return true;
}

bool CDirectXGraphics::dxInitScene()
{
	// --------------------------------------------------------------
	// Init viewport.
	// --------------------------------------------------------------
	D3DVIEWPORT9 vp;

	vp.X      = 0;
	vp.Y      = 0;
	vp.Width  = _rectOrig.Width();
	vp.Height = _rectOrig.Height();
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f;
	_pd3dDevice->SetViewport(&vp);
	
	// --------------------------------------------------------------
	// Init camera.
	// --------------------------------------------------------------
	D3DXMATRIX	matProj;
	D3DXMATRIX	matView;

	// Init the camera type (orthographic or perspective)
	// Here, perspective camera.
	float   fov    = 1.04f; // 60 degrees = 1.04 radian.
	float   aspect = (float)_rectOrig.Width()/(float)_rectOrig.Height();
	float   znear  = 0.1f;
	float   zfar   = 15012.0f;

	D3DXMatrixPerspectiveFovLH(&matProj, fov, aspect, znear, zfar);
	_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// Init camera view matrix
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, 2000.0f, 0.0f),	// camera position
								 &D3DXVECTOR3(0.0f, 0.0f, 0.0f),	// target position
								 &D3DXVECTOR3(0.0f, 0.0f, -1.0f));	// camera up vector
	_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	_pd3dDevice->SetRenderState(D3DRS_AMBIENT, RGB(80,80,80));
	//_pd3dDevice->SetRenderState(D3DRS_AMBIENT, RGB(0,0,0));
#if 0
	// Turn off D3D lighting, since we are providing our own vertex colors
	//_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	_pd3dDevice->SetRenderState(D3DRS_LIGHTING, true);

	// Turn off culling, so we see the front and back of the triangle
	_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

	// Wireframe render
	//_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	//alpha blending enabled (NEW)
	_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	//source alpha (NEW)
	_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	//destination alpha (NEW)
	_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	_pd3dDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);

	//_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
	_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
#endif
	_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);


	for (int i(0); i < 8; ++i)
	{
		_pd3dDevice->SetSamplerState(i,D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		_pd3dDevice->SetSamplerState(i,D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		_pd3dDevice->SetSamplerState(i,D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
		_pd3dDevice->SetSamplerState(i,D3DSAMP_MAXANISOTROPY,0);
	}


	D3DLIGHT9	light;

	ZeroMemory(&light, sizeof(light));
	light.Type        = D3DLIGHT_DIRECTIONAL;
	light.Diffuse     = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Direction.x = 0.0f;
	light.Direction.y = -10000.0f;
	light.Direction.z = 0.0f;
	light.Range       = 21000.0f;

	_pd3dDevice->SetLight(0, &light);
	_pd3dDevice->LightEnable(0, true);

	ZeroMemory(&light, sizeof(light));
	light.Type        = D3DLIGHT_DIRECTIONAL;
	light.Diffuse     = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Direction.x = 0.0f;
	light.Direction.y = 10000.0f;
	light.Direction.z = 0.0f;
	//light.Range       = 21000.0f;

	_pd3dDevice->SetLight(1, &light);
	_pd3dDevice->LightEnable(1, true);




	return true;
}

bool CDirectXGraphics::dxCleanupScene()
{
	//  nothing to do yet
	return true;
}

bool CDirectXGraphics::dxBeginScene()
{
	//  clear background
	_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, glConfig._colorBackground, 1.0f, 0);
	_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, glConfig._colorBackground, 1.0f, 0);

	//  Begin the scene
	_pd3dDevice->BeginScene();

	return true;
}

bool CDirectXGraphics::dxEndScene()
{
	// End the scene
	_pd3dDevice->EndScene();

	// Present the backbuffer contents to the display
	_pd3dDevice->Present(NULL, NULL, NULL, NULL);

	return true;
}

bool CDirectXGraphics::dxRenderScene()
{
	//  transform global view
	D3DXMATRIX		matScale;
	D3DXMATRIX		matRotX;
	D3DXMATRIX		matRotY;
	D3DXMATRIX		matRotZ;
	D3DXMATRIX		matTrans;
	D3DXMATRIX		matWorld;

	D3DXMatrixRotationX(&matRotX, (_vecViewCam.x+_rotX)*3.14159f/180.0f);
	D3DXMatrixRotationY(&matRotY, (_vecViewCam.y)*3.14159f/180.0f);
	D3DXMatrixRotationZ(&matRotZ, (_vecViewCam.z+_rotY)*3.14159f/180.0f);
	D3DXMatrixScaling(&matScale, -_zoom, -_zoom, -_zoom);
	D3DXMatrixTranslation(&matTrans, _posX*50, 0, _posY*50);

	matWorld = (matRotZ*matRotY*matRotX) * matTrans * matScale;
	_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	//  render each mesh in list - opaque objects
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end(); ++pIter)
	{
		if (!(*pIter)->HasAlpha())
		{
			(*pIter)->Render(_pd3dDevice, matWorld);
		}
	}

	//  render each mesh in list - alpha objects
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end(); ++pIter)
	{
		if ((*pIter)->HasAlpha())
		{
			(*pIter)->Render(_pd3dDevice, matWorld);
		}
	}

	return true;
}

void CDirectXGraphics::dxSetCameraPos(const DirecXCameraPos pos)
{
	switch (pos)
	{
		default:
		case DX_CAM_POS_FRONT:
		{
			_vecViewCam.x = 0.0f;
			_vecViewCam.y = 0.0f;
			_vecViewCam.z = 0.0f;
			break;
		}

		case DX_CAM_POS_TOP:
		{
			_vecViewCam.x = 90.0f;
			_vecViewCam.y = 0.0f;
			_vecViewCam.z = 0.0f;
			break;
		}

		case DX_CAM_POS_SIDE:
		{
			_vecViewCam.x = 0.0f;
			_vecViewCam.y = 0.0f;
			_vecViewCam.z = 90.0f;
			break;
		}
	}

	_posX = 0.0f;
	_posY = 0.0f;
	_zoom = 1.0f;
	_rotX = 0.0f;
	_rotY = 0.0f;

	dxInitScene();
}

void CDirectXGraphics::dxSetShowAxes(bool show)
{
	//  early return on non-changing value
	if (show == _showAxes)		return;

	//  set flag
	_showAxes = show;

	//  search for axes and toggle display
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end(); ++pIter)
	{
		if (dynamic_cast<DirectXMeshAxes*>(*pIter) != NULL)
		{
			(*pIter)->SetDoRender(show);
		}
	}
}

void CDirectXGraphics::dxSetShowModel(bool show)
{
	//  early return on non-changing value
	if (show == _showModel)		return;

	//  set flag
	_showModel = show;

	//  search for model and toggle display
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end(); ++pIter)
	{
		if (dynamic_cast<DirectXMeshModel*>(*pIter) != NULL)
		{
			(*pIter)->SetDoRender(show);
		}
	}
}

void CDirectXGraphics::dxAddMesh(DirectXMesh* pMesh)
{
	//_meshList.push_back(pMesh);
	_meshList.insert(_meshList.begin(), pMesh);
}

void CDirectXGraphics::dxClearModel()
{
	//  search for model and remove and delete object
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end();)
	{
		if (dynamic_cast<DirectXMeshModel*>(*pIter) != NULL)
		{
			delete *pIter;
			pIter = _meshList.erase(pIter);
		}
		else
		{
			++pIter;
		}
	}
}

void CDirectXGraphics::dxSetColorWireframe(DWORD color)
{
	//  search for model and remove and delete object
	for (vector<DirectXMesh*>::iterator pIter=_meshList.begin(); pIter != _meshList.end(); ++pIter)
	{
		if (dynamic_cast<DirectXMeshModel*>(*pIter) != NULL)
		{
			(dynamic_cast<DirectXMeshModel*>(*pIter))->SetColorWireframe(color);
		}
	}
}
