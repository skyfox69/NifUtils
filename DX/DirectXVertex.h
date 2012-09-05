#pragma once

#include <d3dx9.h>

namespace NifUtility
{
	//  simple vertex, coordinates only
	#define D3DFVF_CUSTOMVERTEX				D3DFVF_XYZ

	struct D3DCustomVertex
	{
	  float   _x;
	  float   _y;
	  float   _z;
	};

	//  vertex with color
	#define D3DFVF_CUSTOMVERTEX_COLOR		D3DFVF_XYZ | D3DFVF_DIFFUSE

	struct D3DCustomVertexColor
	{
	  float   _x;
	  float   _y;
	  float   _z;
	  DWORD   _color;
	};

	//  vertex with color and normals
	#define D3DFVF_CUSTOMVERTEX_COLNORMAL	D3DFVF_XYZ | D3DFVF_DIFFUSE //| D3DFVF_NORMAL

	struct D3DCustomVertexColNormal
	{
	  float   _x;
	  float   _y;
	  float   _z;
	  DWORD   _color;
//	  NORMAL  _normal;
	};
}
