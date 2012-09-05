#pragma once

#include "DirectXMesh.h"

namespace NifUtility
{
	struct D3DCustomVertexColNormal;

	class DirectXMeshModel : public DirectXMesh
	{
		protected:
			D3DCustomVertexColNormal*	_pBufVertices;
			unsigned short*				_pBufIndices;

		public:
							DirectXMeshModel();
							DirectXMeshModel(D3DXMATRIX transform,
											 D3DCustomVertexColNormal* pBufferV,
											 const unsigned int countV,
											 unsigned short* pBufferI,
											 const unsigned int countI
											);
			virtual			~DirectXMeshModel();

			virtual	void	SetVBuffer(D3DCustomVertexColNormal* pBuffer, const unsigned int count);
			virtual	void	SetIBuffer(unsigned short* pBuffer, const unsigned int count);

			virtual	bool	Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
	};
}
