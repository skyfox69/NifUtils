#include "..\Common\stdafx.h"
#include "..\Common\Configuration.h"
#include "..\Common\FDCLibHelper.h"
#include "DirectXNifConverter.h"
#include "DirectXMeshModel.h"
#include "DirectXMeshCollision.h"
#include "DirectXVertex.h"

//  Niflib includes
#include "niflib.h"
#include "obj/rootcollisionnode.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiSourceTexture.h"
#include "obj/bhkCollisionObject.h"
#include "obj/NiBillboardNode.h"

using namespace NifUtility;

extern Configuration	glConfig;

DirectXNifConverter::DirectXNifConverter()
	:	_isCollision(false),
		_isBillboard(false)
{
}

DirectXNifConverter::~DirectXNifConverter()
{
}

unsigned int DirectXNifConverter::getGeometryFromNode(NiNodeRef pNode, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	vector<NiAVObjectRef>	childList(pNode->GetChildren());

	//  check for NiBillboardNode
	_isBillboard = (DynamicCast<NiBillboardNode>(pNode) != NULL);

	//  add own translation to list
	transformAry.push_back(pNode->GetLocalTransform());

	//  iterate over children
	for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), meshList, transformAry, pTmplAlphaProp);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			//  set collision flag
			_isCollision = true;

			//  recurse sub-tree
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), meshList, transformAry, pTmplAlphaProp);

			//  reset collision flag
			_isCollision = false;
		}
		//  RootCollisionNode
		else if (DynamicCast<bhkCollisionObject>(*ppIter) != NULL)
		{
			//  special handling of Skyrims collision node
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			//  find NiAlphaProperty and use as template in sub-nodes
			if (DynamicCast<NiAlphaProperty>((DynamicCast<NiNode>(*ppIter))->GetPropertyByType(NiAlphaProperty::TYPE)) != NULL)
			{
				pTmplAlphaProp = DynamicCast<NiAlphaProperty>((DynamicCast<NiNode>(*ppIter))->GetPropertyByType(NiAlphaProperty::TYPE));
			}

			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), meshList, transformAry, pTmplAlphaProp);
		}
	}  //  for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)

	//  reset billboard flag
	_isBillboard = false;

	//  remove own translation from list
	transformAry.pop_back();

	return meshList.size();
}

unsigned int DirectXNifConverter::getGeometryFromTriShape(NiTriShapeRef pShape, vector<DirectXMesh*>& meshList, vector<Matrix44>& transformAry, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiTriShapeDataRef	pData(DynamicCast<NiTriShapeData>(pShape->GetData()));

	if (pData != NULL)
	{
		D3DMATERIAL9			material;
		vector<TexCoord>		vecTexCoords;
		vector<Vector3>			vecVertices (pData->GetVertices());
		vector<Triangle>		vecTriangles(pData->GetTriangles());
		vector<Vector3>			vecNormals  (pData->GetNormals());
		vector<Color4>			vecColors   (pData->GetColors());
		vector<NiPropertyRef>	propList    (pShape->GetProperties());
		Matrix44				locTransform(pShape->GetLocalTransform());
		string					baseTexture;
		DWORD					alpSource   (0);
		DWORD					alpDest     (0);
		DWORD					alpArg      (0);
		bool					hasMaterial (false);
		bool					hasAlpha    (false);

		//  get uv set
		if (pData->GetUVSetCount() > 0)
		{
			vecTexCoords = pData->GetUVSet(0);
		}

		//  parse properties
		for (vector<NiPropertyRef>::iterator pIter=propList.begin(); pIter != propList.end(); ++pIter)
		{
			//  NiTexturingProperty
			if (DynamicCast<NiTexturingProperty>(*pIter) != NULL)
			{
				TexDesc		baseTex((DynamicCast<NiTexturingProperty>(*pIter))->GetTexture(BASE_MAP));
				string		texName(baseTex.source->GetTextureFileName());
				ifstream	inFile;
				char		cBuffer[1000];
				char*		pStart(NULL);

				for (vector<string>::iterator pIter(glConfig._dirTexturePath.begin()), pEnd(glConfig._dirTexturePath.end()); pIter != pEnd; ++pIter)
				{
					_snprintf(cBuffer, 1000, "%s\\%s", pIter->c_str(), texName.c_str());

					//  test for '\\'
					if ((pStart = strstr(cBuffer, "\\\\")) != NULL)
					{
						memmove(pStart, pStart+1, strlen(pStart));
					}
					
					//  test for 'texture\texture' (case insensitive)
					if ((pStart = (char*) strcasestr(cBuffer, "textures\\textures")) != NULL)
					{
						memmove(pStart, pStart+8, strlen(pStart));
					}

					//  set texture path
					baseTexture = cBuffer;

					//  test for forced dds
					if (glConfig._dxForceDDS)
					{
						baseTexture = baseTexture.substr(0, baseTexture.length() - 3) + "dds";
					}

					//  test if texture file exists
					inFile.open(baseTexture.c_str());
					if (inFile)		break;

				}  //  for (auto pIter(glConfig._dirTexturePath.begin()), ...
			}
			//  NiAlphaProperty
			else if (DynamicCast<NiAlphaProperty>(*pIter) != NULL)
			{
				NiAlphaProperty*	pProp(DynamicCast<NiAlphaProperty>(*pIter));

				BlendFuncToDXBlend(pProp->GetSourceBlendFunc(), alpSource, alpArg);
				BlendFuncToDXBlend(pProp->GetDestBlendFunc(), alpDest, alpArg);
				hasAlpha  = true;
			}
			//  NiMaterialProperty
			else if (DynamicCast<NiMaterialProperty>(*pIter) != NULL)
			{
				NiMaterialProperty*	pProp(DynamicCast<NiMaterialProperty>(*pIter));
				Color3				tColor;

				tColor = pProp->GetAmbientColor();		material.Ambient  = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
				tColor = pProp->GetDiffuseColor();		material.Diffuse  = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
				tColor = pProp->GetEmissiveColor();		material.Emissive = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
				tColor = pProp->GetSpecularColor();		material.Specular = D3DXCOLOR(tColor.r, tColor.g, tColor.b, 1.0f);
														material.Power    = pProp->GetGlossiness();

				hasMaterial = true;
			}
		}  //  for (vector<NiPropertyRef>::iterator pIter=propList.begin(); pIter != propList.end(); ++pIter)

		//  collected all data needed => convert to DirectX
		//  - transformation matrix
		for (vector<Matrix44>::iterator pIter=transformAry.begin(); pIter != transformAry.end(); ++pIter)
		{
			locTransform *= *pIter;
		}

		//  - vertices
		unsigned int				countV       (vecVertices.size());
		D3DCustomVertexColNormTex*	pBufVertices (new D3DCustomVertexColNormTex[countV]);
		D3DCustomVertexColor*		pBufVerticesW(new D3DCustomVertexColor[countV]);

		for (unsigned int i(0); i < countV; ++i)
		{
			//  shape of model object => no model vertices
			if (!_isCollision)
			{
				//  model
				pBufVertices[i]._x        = vecVertices[i].x;
				pBufVertices[i]._y        = vecVertices[i].y;
				pBufVertices[i]._z        = vecVertices[i].z;
				pBufVertices[i]._normal.x = vecNormals[i].x;
				pBufVertices[i]._normal.y = vecNormals[i].y;
				pBufVertices[i]._normal.z = vecNormals[i].z;
				pBufVertices[i]._color    = !vecColors.empty() ? D3DXCOLOR(vecColors[i].r, vecColors[i].g, vecColors[i].b, 1.0f) : D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				pBufVertices[i]._u        = vecTexCoords[i].u;
				pBufVertices[i]._v        = vecTexCoords[i].v;
			}

			//  wireframe
			pBufVerticesW[i]._x     = vecVertices[i].x;
			pBufVerticesW[i]._y     = vecVertices[i].y;
			pBufVerticesW[i]._z     = vecVertices[i].z;
			pBufVerticesW[i]._color = _isCollision ? glConfig._colorWireCollision : glConfig._colorWireframe;
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

		//  shape of model object
		if (!_isCollision)
		{
			//  - material
			if (!hasMaterial)
			{
				ZeroMemory(&material, sizeof(material));
				material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
				material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			}

			//  - alpha
			if (!hasAlpha && (pTmplAlphaProp != NULL))
			{
				BlendFuncToDXBlend(pTmplAlphaProp->GetSourceBlendFunc(), alpSource, alpArg);
				BlendFuncToDXBlend(pTmplAlphaProp->GetDestBlendFunc(), alpDest, alpArg);
				hasAlpha  = true;
			}

			//  append mesh to list
			meshList.push_back(new DirectXMeshModel(Matrix44ToD3DXMATRIX(locTransform), material, pBufVertices, countV, pBufIndices, countI, baseTexture, pBufVerticesW, _isBillboard));
		
			//  alpha blending defined?
			if (hasAlpha)		meshList.back()->SetAlpha(alpSource, alpDest, alpArg);
		}
		else  //  if (!_isCollision)
		//  shape of collision object
		{
			//  append mesh to list
			meshList.push_back(new DirectXMeshCollision(Matrix44ToD3DXMATRIX(locTransform), pBufVerticesW, countV, pBufIndices, countI));

			//  delte unused data
			delete[] pBufVertices;

		}  //  else [if (!_isCollision)]

		//  add additional info to object
		meshList.back()->SetInfo(pShape->GetName(), pShape->GetType().GetTypeName(), pShape->internal_block_number);

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
	NiNodeRef			pRootInput(NULL);
	vector<Matrix44>	transformAry;
	bool				fakedRoot (false);

	//  test on existing file names
	if (fileName.empty())		return false;

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileName, fakedRoot)) == NULL)
	{
		return false;
	}

	//  parse geometry
	getGeometryFromNode(pRootInput, meshList, transformAry, NULL);

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

void DirectXNifConverter::BlendFuncToDXBlend(const NiAlphaProperty::BlendFunc value, DWORD& dxBlend, DWORD& dxArg)
{
	switch (value)
	{
		case NiAlphaProperty::BF_SRC_ALPHA:
		case NiAlphaProperty::BF_SRC_ALPHA_SATURATE:
		{
			dxBlend = D3DBLEND_SRCALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}

		case NiAlphaProperty::BF_DST_ALPHA:
		{
			dxBlend = D3DBLEND_DESTALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}

		case NiAlphaProperty::BF_ONE_MINUS_SRC_ALPHA:
		{
			dxBlend = D3DBLEND_INVSRCALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}

		case NiAlphaProperty::BF_ONE_MINUS_DST_ALPHA:
		{
			dxBlend = D3DBLEND_INVDESTALPHA;
			dxArg   = D3DTA_TEXTURE;
			break;
		}
	}
}