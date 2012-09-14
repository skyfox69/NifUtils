#pragma once

#include <d3dx9.h>

namespace NifUtility
{
	class DirectXMesh
	{
		protected:
			struct AlphaState
			{
				DWORD	_source;
				DWORD	_destination;
				DWORD	_argument;
			};

		protected:
			AlphaState*				_pAlpha;		//  parameters used for alpha blending
			D3DXMATRIX				_transform;		//  transformation to global coord system
			D3DMATERIAL9			_material;		//  material of object
			LPDIRECT3DVERTEXBUFFER9	_pVBuffer;		//  VertexBuffer to hold vertices
			LPDIRECT3DINDEXBUFFER9	_pIBuffer;		//  IndexBuffer to hold face indexes
			unsigned int			_countVertices;	//  number of vertices
			unsigned int			_countIndices;	//  number of indices
			bool					_doRender;		//  flag enabling rendering

		public:
							DirectXMesh();
			virtual			~DirectXMesh();

			virtual	void	SetDoRender(const bool doRender);
			virtual void	SetAlpha   (const DWORD source, const DWORD destination, const DWORD argument);
			virtual	bool	HasAlpha   () {return (_pAlpha != NULL); }

			virtual	bool	Render     (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix) = 0;
	};
}
