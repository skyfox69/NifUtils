#include "..\Common\stdafx.h"
#include "DirectXMesh.h"

using namespace NifUtility;

DirectXMesh::DirectXMesh(const DirectXNifShapeType shapeType)
	:	_pVBuffer     (NULL),
		_pIBuffer     (NULL),
		_shapeType    (shapeType),
		_countVertices(0),
		_countIndices (0),
		_doRender     (true)
{
	ZeroMemory(&_material, sizeof(_material));
	_material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	_material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

DirectXMesh::~DirectXMesh()
{
	if (_pVBuffer != NULL)		_pVBuffer->Release();
	if (_pIBuffer != NULL)		_pIBuffer->Release();
}

void DirectXMesh::SetDoRender(const bool doRender)
{
	_doRender = doRender;
}
