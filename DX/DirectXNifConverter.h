#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

#include "obj/bsfadenode.h"
#include "obj/nitrishape.h"
#include "obj/NiAlphaProperty.h"

using namespace Niflib;
using namespace std;

namespace NifUtility
{
	class DirectXMesh;

	class DirectXNifConverter
	{
		protected:
			virtual	NiNodeRef		getRootNodeFromNifFile (string fileName, bool& fakedRoot);
			virtual	unsigned int	getGeometryFromNode    (NiNodeRef pNode, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry);
			virtual	unsigned int	getGeometryFromTriShape(NiTriShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry);

			virtual D3DXMATRIX		Matrix44ToD3DXMATRIX(const Matrix44& matrixIn);
			virtual	DWORD			BlendFuncToDXBlend  (const NiAlphaProperty::BlendFunc value);

		public:
									DirectXNifConverter();
			virtual					~DirectXNifConverter();

			virtual	bool			ConvertModel    (string fileName, vector<DirectXMesh*>& meshList);
			virtual	bool			ConvertCollision(string fileName, vector<DirectXMesh*>& meshList);
	};
}
