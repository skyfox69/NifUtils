#include "..\Common\stdafx.h"
#include "DirectXMeshModel.h"
#include "DirectXVertex.h"

using namespace NifUtility;

DirectXMeshModel::DirectXMeshModel()
	:	DirectXMesh  (DX_NIF_SHP_MODEL),
		_pBufVertices(NULL),
		_pBufIndices (NULL)
{
}

DirectXMeshModel::DirectXMeshModel(D3DXMATRIX transform,
								   D3DCustomVertexColNormal* pBufferV,
								   const unsigned int countV,
								   unsigned short* pBufferI,
								   const unsigned int countI
								  )
	:	DirectXMesh  (DX_NIF_SHP_MODEL),
		_pBufVertices(pBufferV),
		_pBufIndices (pBufferI)
{
	_transform     = transform;
	_countVertices = countV;
	_countIndices  = countI;
}

DirectXMeshModel::~DirectXMeshModel()
{
	if (_pBufVertices != NULL)		delete[] _pBufVertices;
	if (_pBufIndices  != NULL)		delete[] _pBufIndices;
}

void DirectXMeshModel::SetVBuffer(D3DCustomVertexColNormal* pBuffer, const unsigned int count)
{
	_pBufVertices  = pBuffer;
	_countVertices = count;
}

void DirectXMeshModel::SetIBuffer(unsigned short* pBuffer, const unsigned int count)
{
	_pBufIndices  = pBuffer;
	_countIndices = count;
}

bool DirectXMeshModel::Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix)
{
	//  early return on non rendering
	if (!_doRender)		return true;

	//  create DX parameters if not existing
	if (_pVBuffer == NULL)
	{
		D3DCustomVertexColNormal*	pVAxis(NULL);
		unsigned short*				pIAxis(NULL);

		pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertexColNormal), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);
		_pVBuffer->Lock(0, 0, (void**)&pVAxis, 0);
		memcpy(pVAxis, _pBufVertices, _countVertices*sizeof(D3DCustomVertexColNormal));
		_pVBuffer->Unlock();

		pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
		_pIBuffer->Lock(0, 0, (void**)&pIAxis, 0);
		memcpy(pIAxis, _pBufIndices, _countIndices*sizeof(unsigned short));
		_pIBuffer->Unlock();

		//  once used buffers are deleteable
		delete[] _pBufVertices;
		delete[] _pBufIndices;
		_pBufVertices = NULL;
		_pBufIndices  = NULL;
	}

	pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);
	pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);
	pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertexColNormal));
	pd3dDevice->SetIndices          (_pIBuffer);
	pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLNORMAL);
	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);

	return true;
}
