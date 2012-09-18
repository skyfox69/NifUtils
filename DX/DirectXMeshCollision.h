#pragma once

#include "DirectXMesh.h"
#include "DirectXVertex.h"

namespace NifUtility
{
	class DirectXMeshCollision : public DirectXMesh
	{
		protected:
			D3DCustomVertexColor*	_pBufVertices;
			unsigned short*			_pBufIndices;

		public:
							DirectXMeshCollision(D3DXMATRIX transform,
												 D3DCustomVertexColor* pBufferV,
												 const unsigned int countV,
												 unsigned short* pBufferI,
												 const unsigned int countI
												);
			virtual			~DirectXMeshCollision();

			virtual void	SetColorWireframe(DWORD color);

			virtual	bool	Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
	};
}
