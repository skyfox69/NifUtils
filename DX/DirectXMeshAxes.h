#pragma once

#include "DirectXMesh.h"

namespace NifUtility
{
	class DirectXMeshAxes : public DirectXMesh
	{
		public:
							DirectXMeshAxes();
			virtual			~DirectXMeshAxes();

			virtual void	SetColorWireframe(DWORD) {};

			virtual	bool	Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
	};
}
