#include "..\Common\stdafx.h"
#include "DirectXNifConverter.h"
#include "DirectXMeshModel.h"
#include "DirectXVertex.h"

//  Niflib includes
#include "niflib.h"
#include "obj/rootcollisionnode.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiAlphaProperty.h"
#include "obj/NiSourceTexture.h"

using namespace NifUtility;

DirectXNifConverter::DirectXNifConverter()
{
}

DirectXNifConverter::~DirectXNifConverter()
{
}

unsigned int DirectXNifConverter::getGeometryFromNode(NiNodeRef pNode, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry)
{
	vector<NiAVObjectRef>	childList(pNode->GetChildren());

	//  add own translation to list
	transformAry.push_back(pNode->GetLocalTransform());

	//  iterate over children
	for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), meshList, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), meshList, transformAry);
		}
	}  //  for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)

	//  remove own translation from list
	transformAry.pop_back();

	return meshList.size();
}

unsigned int DirectXNifConverter::getGeometryFromTriShape(NiTriShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry)
{
	NiTriShapeDataRef	pData(DynamicCast<NiTriShapeData>(pShape->GetData()));

	if (pData != NULL)
	{
		vector<Vector3>			vecVertices (pData->GetVertices());
		vector<Triangle>		vecTriangles(pData->GetTriangles());
		vector<Vector3>			vecNormals  (pData->GetNormals());
		vector<TexCoord>		vecTexCoords(pData->GetUVSet(0));
		vector<NiPropertyRef>	propList    (pShape->GetProperties());
		Matrix44				locTransform(pShape->GetLocalTransform());
		Vector3					locCenter   (pData->GetCenter());
		string					baseTexture;

		//  parse properties
		for (vector<NiPropertyRef>::iterator pIter=propList.begin(); pIter != propList.end(); ++pIter)
		{
			//  NiTexturingProperty
			if (DynamicCast<NiTexturingProperty>(*pIter) != NULL)
			{
				TexDesc		baseTex((DynamicCast<NiTexturingProperty>(*pIter))->GetTexture(BASE_MAP));

				baseTexture = baseTex.source->GetTextureFileName();
			}
			//  NiAlphaProperty
			else if (DynamicCast<NiAlphaProperty>(*pIter) != NULL)
			{



			}
		}  //  for (vector<NiPropertyRef>::iterator pIter=propList.begin(); pIter != propList.end(); ++pIter)

		//  collected all data needed => convert to DirectX
		//  - transformation matrix
		for (vector<Matrix44>::iterator pIter=transformAry.begin(); pIter != transformAry.end(); ++pIter)
		{
			locTransform *= *pIter;
		}

		//  - vertices
		unsigned int				countV      (vecVertices.size());
		D3DCustomVertexColNormal*	pBufVertices(new D3DCustomVertexColNormal[countV]);

		for (unsigned int i(0); i < countV; ++i)
		{
			pBufVertices[i]._x     = vecVertices[i].x;
			pBufVertices[i]._y     = vecVertices[i].y;
			pBufVertices[i]._z     = vecVertices[i].z;
			pBufVertices[i]._color = 0xC0C0C0;
		}
		
		//  - indices
		unsigned int		countI     (vecTriangles.size()*3);
		unsigned short*		pBufIndices(new unsigned short[countI]);

		for (unsigned int i(0); i < countI; i+=3)
		{
			pBufIndices[i]   = vecTriangles[i/3].v1;
			pBufIndices[i+1] = vecTriangles[i/3].v2;
			pBufIndices[i+2] = vecTriangles[i/3].v3;
		}




		//  append mesh to list
		meshList.push_back(new DirectXMeshModel(Matrix44ToD3DXMATRIX(locTransform), pBufVertices, countV, pBufIndices, countI));

	}  //  if (pData != NULL)

	return meshList.size();
}

NiNodeRef DirectXNifConverter::getRootNodeFromNifFile(string fileName, bool& fakedRoot)
{
	NiObjectRef		pRootTree (NULL);
	NiNodeRef		pRootInput(NULL);

	//  get input nif
	pRootTree = ReadNifTree((const char*) fileName.c_str());

	//  NiNode as root
	if (DynamicCast<NiNode>(pRootTree) != NULL)
	{
		pRootInput = DynamicCast<NiNode>(pRootTree);
	}
	//  NiTriShape as root
	else if (DynamicCast<NiTriShape>(pRootTree) != NULL)
	{
		//  create faked root
		pRootInput = new NiNode();

		//  add root as child
		pRootInput->AddChild(DynamicCast<NiAVObject>(pRootTree));

		//  mark faked root node
		fakedRoot = true;
	}

	return pRootInput;
}

bool DirectXNifConverter::ConvertModel(string fileName, vector<DirectXMesh*>& meshList)
{
	NiNodeRef				pRootInput(NULL);
	vector<NiAVObjectRef>	childList;
	vector<Matrix44>		transformAry;
	bool					fakedRoot (false);

	//  test on existing file names
	if (fileName.empty())		return false;

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileName, fakedRoot)) == NULL)
	{
		return false;
	}

	//  add own transform to list
	transformAry.push_back(pRootInput->GetLocalTransform());

	//  get list of children from input node
	childList = pRootInput->GetChildren();

	//  iterate over source nodes and convert using template
	for (vector<NiAVObjectRef>::iterator  ppIter = childList.begin(); ppIter != childList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), meshList, transformAry);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			//  ignore node
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), meshList, transformAry);
		}
	}

	//  remove own translation from list
	transformAry.pop_back();

	return true;
}

bool DirectXNifConverter::ConvertCollision(string fileName, vector<DirectXMesh*>& meshList)
{


	return true;
}

D3DXMATRIX DirectXNifConverter::Matrix44ToD3DXMATRIX(const Matrix44& matrixIn)
{
	D3DXMATRIX	matrixOut;

	for (short idx(0); idx < 16; ++idx)
	{
		matrixOut((idx / 4), (idx % 4)) = matrixIn.rows[(idx / 4)][(idx % 4)];
	}

	return matrixOut;
}
