#include "..\Common\stdafx.h"
#include "DirectXMeshCollision.h"

using namespace NifUtility;

DirectXMeshCollision::DirectXMeshCollision(D3DXMATRIX transform,
										   D3DCustomVertexColor* pBufferV,
										   const unsigned int countV,
										   unsigned short* pBufferI,
										   const unsigned int countI
										  )
	:	DirectXMesh(),
		_pBufVertices (pBufferV),
		_pBufIndices  (pBufferI)

{
	_transform     = transform;
	_countVertices = countV;
	_countIndices  = countI;
	_doRender      = false;
}

DirectXMeshCollision::~DirectXMeshCollision()
{
	if (_pBufVertices != NULL)		delete[] _pBufVertices;
	if (_pBufIndices  != NULL)		delete[] _pBufIndices;
}

bool DirectXMeshCollision::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
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
		memcpy(pVAxis, _pBufVertices, _countVertices*sizeof(D3DCustomVertexColor));
		_pVBuffer->Unlock();

		pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
		_pIBuffer->Lock(0, 0, (void**)&pIAxis, 0);
		memcpy(pIAxis, _pBufIndices, _countIndices*sizeof(unsigned short));
		_pIBuffer->Unlock();
	}

	pd3dDevice->SetTexture          (0, NULL);														//  no texture
	pd3dDevice->SetRenderState		(D3DRS_ALPHABLENDENABLE, false);								//	disable alpha blending
	pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);							//  forced wireframe
	pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);										//  disable light
	pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);									//  set world transformation
	pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);										//  transform local object into world
	pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertexColor));				//  set vertices source
	pd3dDevice->SetIndices          (_pIBuffer);													//  set indices source
	pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);									//  set vertex style
	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);	//  render

	return true;
}

void DirectXMeshCollision::SetColorWireframe(DWORD color)
{
	//  vertex buffer already initialized?
	if (_pVBuffer != NULL)
	{
		D3DCustomVertexColor*	pVAxis(NULL);

		_pVBuffer->Lock(0, 0, (void**)&pVAxis, 0);
		for (unsigned int i(0); i < _countVertices; ++i)
		{
			pVAxis[i]._color = color;
		}
		_pVBuffer->Unlock();
	}

	//  vertex buffer not initialized yet
	if (_pBufVertices != NULL)
	{
		for (unsigned int i(0); i < _countVertices; ++i)
		{
			_pBufVertices[i]._color = color;
		}
	}
}
