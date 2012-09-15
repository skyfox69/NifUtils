#pragma once

#include "DirectXMesh.h"
#include <string>

using namespace std;

namespace NifUtility
{
	struct D3DCustomVertexColNormTex;
	struct D3DCustomVertexColor;

	class DirectXMeshModel : public DirectXMesh
	{
		protected:
			LPDIRECT3DTEXTURE9			_pTexture;
			LPDIRECT3DVERTEXBUFFER9		_pWBuffer;		//  VertexBuffer to hold vertices for wireframe
			D3DCustomVertexColNormTex*	_pBufVertices;
			D3DCustomVertexColor*		_pBufVerticesW;
			unsigned short*				_pBufIndices;
			string						_textureName;

		public:
							DirectXMeshModel();
							DirectXMeshModel(D3DXMATRIX transform,
											 D3DMATERIAL9 material,
											 D3DCustomVertexColNormTex* pBufferV,
											 const unsigned int countV,
											 unsigned short* pBufferI,
											 const unsigned int countI,
											 string textureName,
											 D3DCustomVertexColor* pBufferW=NULL
											);
			virtual			~DirectXMeshModel();

			virtual void	SetColorWireframe(DWORD color);

			virtual	bool	Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
	};
}
