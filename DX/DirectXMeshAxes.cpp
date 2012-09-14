#include "..\Common\stdafx.h"
#include "DirectXMeshAxes.h"
#include "DirectXVertex.h"

using namespace NifUtility;

static D3DCustomVertexColor vertPts[] = {{-500.0f,   0.0f,   0.0f, 0x000000},
										 {  -1.0f,   0.0f,   0.0f, 0x000000},
										 {   1.0f,   0.0f,   0.0f, 0x000000},
										 { 500.0f,   0.0f,   0.0f, 0x000000},
										 { 450.0f,  30.0f,  30.0f, 0x000000},
										 { 450.0f, -30.0f,  30.0f, 0x000000},
										 { 450.0f, -30.0f, -30.0f, 0x000000},
										 { 450.0f,  30.0f, -30.0f, 0x000000}
										};

static unsigned int vertIdx[] = {0, 1, 2, 3, 4, 3, 5, 3, 6, 3, 7, 3};

DirectXMeshAxes::DirectXMeshAxes()
	:	DirectXMesh()
{
	_countVertices = 8*3;
	_countIndices  = 12*3;
}

DirectXMeshAxes::~DirectXMeshAxes()
{
}

bool DirectXMeshAxes::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if (!_doRender)		return true;

	//  create DX parameters if not existing
	if (_pVBuffer == NULL)
	{
		D3DCustomVertexColor*	pVAxis(NULL);
		unsigned short*			pIAxis(NULL);

		pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertexColor), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);
		_pVBuffer->Lock(0, 0, (void**)&pVAxis, 0);

		for (short i(0); i < 8; ++i)
		{
			//  x-axis
			pVAxis[i]._x     = vertPts[i]._x;
			pVAxis[i]._y     = vertPts[i]._y;
			pVAxis[i]._z     = vertPts[i]._z;
			pVAxis[i]._color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);

			//  y-axis
			pVAxis[i+8]._x     = vertPts[i]._y;
			pVAxis[i+8]._y     = vertPts[i]._x;
			pVAxis[i+8]._z     = vertPts[i]._z;
			pVAxis[i+8]._color = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);

			//  z-axis
			pVAxis[i+16]._x     = vertPts[i]._z;
			pVAxis[i+16]._y     = -vertPts[i]._y;
			pVAxis[i+16]._z     = vertPts[i]._x;
			pVAxis[i+16]._color = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
		}

		_pVBuffer->Unlock();

		pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
		_pIBuffer->Lock(0, 0, (void**)&pIAxis, 0);

		for (short i(0); i < 12; ++i)
		{
			//  x-axis
			pIAxis[i] = vertIdx[i];

			//  y-axis
			pIAxis[i+12] = vertIdx[i] + 8;

			//  z-axis
			pIAxis[i+24] = vertIdx[i] + 16;
		}

		_pIBuffer->Unlock();
	}

	pd3dDevice->SetMaterial         (&_material);
	pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);
	pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);
	pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertexColor));
	pd3dDevice->SetIndices          (_pIBuffer);
	pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);
	pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, _countIndices, 0, _countIndices/2);

	return true;
}
