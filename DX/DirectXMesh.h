#pragma once

#include <d3dx9.h>

namespace NifUtility
{
	enum DirectXNifShapeType { DX_NIF_SHP_MODEL=0, DX_NIF_SHP_COLLISION, DX_NIF_SHP_SYSTEM };

	class DirectXMesh
	{
		protected:
			D3DXMATRIX				_transform;		//  transformation to global coord system
			D3DMATERIAL9			_material;		//  material of object
			LPDIRECT3DVERTEXBUFFER9	_pVBuffer;		//  VertexBuffer to hold vertices
			LPDIRECT3DINDEXBUFFER9	_pIBuffer;		//  IndexBuffer to hold face indexes
			DirectXNifShapeType		_shapeType;		//  Type of shape represented
			unsigned int			_countVertices;	//  number of vertices
			unsigned int			_countIndices;	//  number of indices
			bool					_doRender;		//  flag enabling rendering


		public:
							DirectXMesh(const DirectXNifShapeType shapeType);
			virtual			~DirectXMesh();

			virtual	void	SetDoRender(const bool doRender);

			virtual	bool	Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix) = 0;
	};
}
