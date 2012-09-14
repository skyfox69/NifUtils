#include "..\Common\stdafx.h"
#include "DirectXMesh.h"

using namespace NifUtility;

DirectXMesh::DirectXMesh()
	:	_pAlpha       (NULL),
		_pVBuffer     (NULL),
		_pIBuffer     (NULL),
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
	if (_pAlpha   != NULL)		delete _pAlpha;
}

void DirectXMesh::SetDoRender(const bool doRender)
{
	_doRender = doRender;
}

void DirectXMesh::SetAlpha(const DWORD source, const DWORD destination, const DWORD argument)
{
	if (_pAlpha == NULL)		_pAlpha = new AlphaState;

	_pAlpha->_source      = source;
	_pAlpha->_destination = destination;
	_pAlpha->_argument    = argument;
}