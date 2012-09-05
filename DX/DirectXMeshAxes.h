#pragma once

#include "DirectXMesh.h"

namespace NifUtility
{
	class DirectXMeshAxes : public DirectXMesh
	{
		public:
							DirectXMeshAxes();
			virtual			~DirectXMeshAxes();

			virtual	bool	Render(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix);
	};
}
