#include "..\Common\stdafx.h"
#include "..\Common\Configuration.h"
#include "DirectXMeshModel.h"
#include "DirectXVertex.h"

using namespace NifUtility;

extern Configuration	glConfig;


DirectXMeshModel::DirectXMeshModel()
	:	DirectXMesh  (),
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
	:	DirectXMesh   (),
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
	if (_pWBuffer      != NULL)		_pWBuffer->Release();
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

		//  load texture from file if given
		if (!_textureName.empty())
		{
			if (_pAlpha != NULL)
			{
				D3DXCreateTextureFromFileEx(pd3dDevice,
											CString(_textureName.c_str()).GetString(),
											D3DX_DEFAULT,
											D3DX_DEFAULT,
											D3DX_DEFAULT,
											NULL,
											D3DFMT_UNKNOWN,//D3DFMT_A8R8G8B8,
											D3DPOOL_MANAGED,
											D3DX_DEFAULT,
											D3DX_DEFAULT,
											0xFF000000,
											NULL,
											NULL,
											&_pTexture);
			}
			else
			{
				D3DXCreateTextureFromFile(pd3dDevice, CString(_textureName.c_str()).GetString(), &_pTexture);
			}
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

	//  render object
	bool	renderObject(!glConfig._dxShowWireframe);

	//  - with texture?
	if (glConfig._dxShowTexture)
	{
		//  set texture if given
		if (_pTexture != NULL)		pd3dDevice->SetTexture(0, _pTexture);

		//  set solid render state
		pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		renderObject = true;
	}
	//  - colored wireframe?
	else if (glConfig._dxShowColorWire)
	{
		//  set wireframe render state
		pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

		renderObject = true;
	}

	//  - something to render solid/colored?
	if (renderObject)
	{
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);										//  show both sides of face
		pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);											//  enable z buffer

		//  alpha properties?
		if (_pAlpha != NULL)
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);									//  enable alpha blending
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, _pAlpha->_source);								//  source alpha
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, _pAlpha->_destination);							//  destination alpha
			pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, _pAlpha->_argument);					//  alpha source (diffuse/texture)
		}
		else
		{
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);									//  disable alpha blending
		}

		pd3dDevice->SetMaterial         (&_material);													//  set material
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, true);											//  enable light
		pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);									//  set world transformation
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);										//  transform local object into world
		pd3dDevice->SetStreamSource     (0, _pVBuffer, 0, sizeof(D3DCustomVertexColNormTex));			//  set vertices source
		pd3dDevice->SetIndices          (_pIBuffer);													//  set indices source
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLNORMTEX);								//  set vertex style
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);	//  render
		pd3dDevice->SetTexture          (0, NULL);

	}  //  if (renderObject)

	//  - something to render pure wireframe?
	if (glConfig._dxShowWireframe && !glConfig._dxShowColorWire && (_pWBuffer != NULL))
	{
		D3DXMATRIX	matBias;

		//  create scale matrix
		D3DXMatrixScaling(&matBias, 1.005f, 1.005f, 1.005f);

		//  set object transformation if not done yet
		if (!renderObject)
		{
			pd3dDevice->SetMaterial         (&_material);												//  set material
			pd3dDevice->SetTransform        (D3DTS_WORLD, &worldMatrix);								//  set world transformation
			pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &_transform);									//  transform local object into world
		}
		pd3dDevice->MultiplyTransform   (D3DTS_WORLD, &matBias);										//  re-scale object in world view
		pd3dDevice->SetTexture          (0, NULL);														//  no texture
		pd3dDevice->SetRenderState		(D3DRS_ALPHABLENDENABLE, false);								//  disable alpha blending
		pd3dDevice->SetRenderState      (D3DRS_FILLMODE, D3DFILL_WIREFRAME);							//  forced wireframe
		pd3dDevice->SetRenderState      (D3DRS_LIGHTING, false);										//  disable light
		pd3dDevice->SetStreamSource     (0, _pWBuffer, 0, sizeof(D3DCustomVertexColor));				//  set vertices source
		pd3dDevice->SetIndices          (_pIBuffer);													//  set indices source
		pd3dDevice->SetFVF              (D3DFVF_CUSTOMVERTEX_COLOR);									//  set vertex style
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _countVertices, 0, _countIndices/3);	//  render

	}  //  if (glConfig._dxShowWireframe && !glConfig._dxShowColorWire)

	return true;
}

void DirectXMeshModel::SetColorWireframe(DWORD color)
{
	//  vertex buffer already initialized?
	if (_pWBuffer != NULL)
	{
		D3DCustomVertexColor*	pVAxis(NULL);

		_pWBuffer->Lock(0, 0, (void**)&pVAxis, 0);
		for (unsigned int i(0); i < _countVertices; ++i)
		{
			pVAxis[i]._color = color;
		}
		_pWBuffer->Unlock();
	}

	//  vertex buffer not initialized yet
	if (_pBufVerticesW != NULL)
	{
		for (unsigned int i(0); i < _countVertices; ++i)
		{
			_pBufVerticesW[i]._color = color;
		}
	}
}
