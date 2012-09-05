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
}

DirectXMesh::~DirectXMesh()
{
}

void DirectXMesh::SetDoRender(const bool doRender)
{
	_doRender = doRender;
}
