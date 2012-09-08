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
								   D3DMATERIAL9 material,
								   D3DCustomVertexColNormTex* pBufferV,
								   const unsigned int countV,
								   unsigned short* pBufferI,
								   const unsigned int countI,
								   string textureName,
								   D3DCustomVertexColor* pBufferW
								  )
	:	DirectXMesh   (DX_NIF_SHP_MODEL),
		_pTexture     (NULL),
		_pWBuffer     (NULL),
		_pBufVertices (pBufferV),
		_pBufVerticesW(pBufferW),
		_pBufIndices  (pBufferI),
		_textureName  (textureName)
{
	_transform     = transform;
	_material      = material;
	_countVertices = countV;
	_countIndices  = countI;
}

DirectXMeshModel::~DirectXMeshModel()
{
	if (_pBufVerticesW != NULL)		delete[] _pBufVerticesW;
	if (_pBufVertices  != NULL)		delete[] _pBufVertices;
	if (_pBufIndices   != NULL)		delete[] _pBufIndices;
}

void DirectXMeshModel::SetVBuffer(D3DCustomVertexColNormTex* pBuffer, const unsigned int count)
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
		D3DCustomVertexColNormTex*	pVAxis(NULL);
		unsigned short*				pIAxis(NULL);

		pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertexColNormTex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pVBuffer, NULL);
		_pVBuffer->Lock(0, 0, (void**)&pVAxis, 0);
		memcpy(pVAxis, _pBufVertices, _countVertices*sizeof(D3DCustomVertexColNormTex));
		_pVBuffer->Unlock();

		pd3dDevice->CreateIndexBuffer(_countIndices*sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIBuffer, NULL);
		_pIBuffer->Lock(0, 0, (void**)&pIAxis, 0);
		memcpy(pIAxis, _pBufIndices, _countIndices*sizeof(unsigned short));
		_pIBuffer->Unlock();

		if (!_textureName.empty())
		{
			D3DXCreateTextureFromFile(pd3dDevice, CString(_textureName.c_str()).GetString(), &_pTexture);
		}

		//  once used buffers are deleteable
		delete[] _pBufVertices;
		delete[] _pBufIndices;
		_pBufVertices = NULL;
		_pBufIndices  = NULL;
	}

	//  wireframe
	if ((_pWBuffer == NULL) && (_pBufVerticesW != NULL))
	{
		D3DCustomVertexColor*	pVAxis(NULL);

		pd3dDevice->CreateVertexBuffer(_countVertices*sizeof(D3DCustomVertexColor), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &_pWBuffer, NULL);
		_pWBuffer->Lock(0, 0, (void**)&pVAxis, 0);
		memcpy(pVAxis, _pBufVerticesW, _countVertices*sizeof(D3DCustomVertexColor));
		_pWBuffer->Unlock();

		//  once used buffers are deleteable
		delete[] _pBufVerticesW;
		_pBufVerticesW = NULL;
	}

	if (_pTexture != NULL)
	{
		pd3dDevice->SetTexture(0, _pTexture);
	}
	pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_SOLID);
	pd3dDevice->SetMaterial         (&_material);
	pd3dDevice->SetRenderState      (D3DRS_LIGHTING, true);
	pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);
	pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);
	pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertexColNormTex));
	pd3dDevice->SetIndices          (_pIBuffer);
	pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLNORMTEX);
	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);
	pd3dDevice->SetTexture(0, NULL);

	if (_pWBuffer != NULL)
	{
		D3DXMATRIX	matBias;

		D3DXMatrixScaling(&matBias, 1.005f, 1.005f, 1.005f);

		pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &matBias);
		pd3dDevice->SetStreamSource     (0, _pWBuffer, 0, sizeof(D3DCustomVertexColor));
		pd3dDevice->SetIndices          (_pIBuffer);
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);
	}

	return true;
}
