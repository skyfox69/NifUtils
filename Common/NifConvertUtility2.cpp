///////////////////////////////////////////////////////////
//  NifConvertUtility2.cpp
//  Implementation of the Class NifConvertUtility2
//  Created on:      06-Mai-2012 10:03:33
//  Original author: Skyfox
///////////////////////////////////////////////////////////

//  Common includes
#include "NifConvertUtility2.h"

//  Niflib includes
#include "niflib.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/NiTexturingProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiTriShapeData.h"
#include "obj/bhkRigidBody.h"
#include "obj/bhkCompressedMeshShape.h"

//  Havok includes
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h>
#include <Physics/Collide/Shape/Compound/Collection/CompressedMesh/hkpCompressedMeshShapeBuilder.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>
#include <Physics/Collide/Util/Welding/hkpMeshWeldingUtility.h>

//  used namespaces
using namespace NifUtility;



/*---------------------------------------------------------------------------*/
NifConvertUtility2::NifConvertUtility2()
	:	_vcDefaultColor(1.0f, 1.0f, 1.0f, 1.0f),
		_vcHandling    (NCU_VC_REMOVE_FLAG),
		_cnHandling    (NCU_CN_FALLBACK)
{

}

/*---------------------------------------------------------------------------*/
NifConvertUtility2::~NifConvertUtility2()
{

}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility2::getGeometryFromTriShape(NiTriShapeRef pShape, vector<hkGeometry*>& geometryAry, vector<Matrix44>& transformAry)
{
	NiTriShapeDataRef	pData(DynamicCast<NiTriShapeData>(pShape->GetData()));

	if (pData != NULL)
	{
		hkGeometry::Triangle*			pTri     (NULL);
		hkGeometry*						pTmpGeo  (NULL);
		vector<Vector3>					vertices (pData->GetVertices());
		vector<Triangle>				triangles(pData->GetTriangles());
		hkArray<hkVector4>				vertAry;
		hkArray<hkGeometry::Triangle>	triAry;
		Vector3							tVector;

		//  add local transformation to list
		transformAry.push_back(pShape->GetLocalTransform());

		//  get vertices
		for (unsigned int idx(0); idx < vertices.size(); ++idx)
		{
			//  get vertex
			tVector = vertices[idx];

			//  transform vertex to global coordinates
			for (int t((int) (transformAry.size())-1); t >= 0; --t)
			{
				tVector = transformAry[t] * tVector;
			}

			//  scale final vertex
			tVector *= 0.0143f;

			//  add vertex to tmp. array
			vertAry.append(new hkVector4(tVector.x, tVector.y, tVector.z), 1);

		}  //  for (unsigned int idx(0); idx < vertices.size(); ++idx)

		//  get triangles
		for (unsigned int idx(0); idx < triangles.size(); ++idx)
		{
			pTri = new hkGeometry::Triangle();
			pTri->set(triangles[idx].v1, triangles[idx].v2, triangles[idx].v3);
			triAry.append(pTri, 1);
		}

		//  create new geometry from vertices and triangles
		pTmpGeo = new hkGeometry();
		pTmpGeo->m_triangles = triAry;
		pTmpGeo->m_vertices  = vertAry;

		//  add geometry to result array
		geometryAry.push_back(pTmpGeo);

		//  remove local transformation from array
		transformAry.pop_back();

	}  //  if (pData != NULL)

	return geometryAry.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility2::getGeometryFromNode(NiNodeRef pNode, vector<hkGeometry*>& geometryAry, vector<Matrix44>& transformAry)
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
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), geometryAry, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), geometryAry, transformAry);
		}
	}  //  for (vector<NiAVObjectRef>::iterator ppIter = childList.begin(); ppIter != childList.end(); ppIter++)

	//  remove own translation from list
	transformAry.pop_back();

	return geometryAry.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility2::getGeometryFromObjFile(string fileName, vector<hkGeometry*>& geometryAry)
{
	hkGeometry::Triangle*			pTri   (NULL);
	hkGeometry*						pTmpGeo(NULL);
	char*							pChar  (NULL);
	char							cBuffer[1000] = {0};
	ifstream						inFile;
	hkArray<hkVector4>				vertAry;
	hkArray<hkGeometry::Triangle>	triAry;
	Vector3							tVector;
	int								tIntAry[3];
	unsigned int					faceOffset(1);
	short							idx(0);
	bool							hasFace(false);

	//  empty filename => early return
	if (fileName.empty())		return 0;

	//  open file
	inFile.open(fileName.c_str(), ifstream::in);

	//  process file
	while (inFile.good())
	{
		//  read line
		inFile.getline(cBuffer, 1000);

		//  vertex?
		if (_strnicmp(cBuffer, "v ", 2) == 0)
		{
			//  existing face? => create new geometry
			if (hasFace)
			{
				//  create new geometry from vertices and triangles
				pTmpGeo = new hkGeometry();
				pTmpGeo->m_triangles = triAry;
				pTmpGeo->m_vertices  = vertAry;

				//  add geometry to result array
				geometryAry.push_back(pTmpGeo);

				//  set new offset for face vertices
				faceOffset = vertAry.getSize() + 1;

				//  reset tmp. arrays
				vertAry.clear();
				triAry.clear();

				//  reset existing face flag
				hasFace = false;

			}  //  if (hasFace)

			//  get vector from line
			sscanf(cBuffer, "v %f %f %f", &(tVector.x), &(tVector.y), &(tVector.z));

			//  scale final vertex
			tVector *= 0.0143f;

			//  add vertex to tmp. array
			vertAry.append(new hkVector4(tVector.x, tVector.y, tVector.z), 1);
		}
		//  face?
		else if (_strnicmp(cBuffer, "f ", 2) == 0)
		{
			//  get triangle idx from line
			for (idx=0, pChar=strchr(cBuffer, ' '); ((pChar != NULL) && (idx < 3)); ++idx, pChar = strchr(pChar, ' '))
			{
				tIntAry[idx] = atoi(++pChar) - faceOffset;
			}

			//  create new triangle and add to tmp. array
			pTri = new hkGeometry::Triangle();
			pTri->set(tIntAry[0], tIntAry[1], tIntAry[2]);
			triAry.append(pTri, 1);

			//  mark existing face
			hasFace = true;
		}
	}  //  while (inFile.good())

	//  close file
	inFile.close();

	return geometryAry.size();
}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility2::getGeometryFromNifFile(string fileName, vector<hkGeometry*>& geometryAry)
{
	NiNodeRef				pRootInput     (NULL);
	vector<NiAVObjectRef>	srcChildList;
	vector<Matrix44>		transformAry;
	vector<hkGeometry*>		geometryAryColl;
	vector<hkGeometry*>		geometryAryShape;
	bool					fakedRoot      (false);

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileName, "collSource", fakedRoot)) == NULL)
	{
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get list of children from input node
	srcChildList = pRootInput->GetChildren();

	//  add own transform to list
	transformAry.push_back(pRootInput->GetLocalTransform());

	//  iterate over source nodes and get geometry
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			getGeometryFromTriShape(DynamicCast<NiTriShape>(*ppIter), geometryAryShape, transformAry);
		}
		//  RootCollisionNode
		else if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(&(*DynamicCast<RootCollisionNode>(*ppIter)), geometryAryColl, transformAry);
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			getGeometryFromNode(DynamicCast<NiNode>(*ppIter), geometryAryShape, transformAry);
		}
	}

	//  which geomertry should be used?
	if ((_cnHandling == NCU_CN_COLLISION) || (_cnHandling == NCU_CN_FALLBACK))
	{
		geometryAry.swap(geometryAryColl);
	}
	else if (_cnHandling == NCU_CN_SHAPES)
	{
		geometryAry.swap(geometryAryShape);
	}
	if ((_cnHandling == NCU_CN_FALLBACK) && (geometryAry.size() <= 0))
	{
		geometryAry.swap(geometryAryShape);
	}

	return geometryAry.size();
}

/*---------------------------------------------------------------------------*/
NiNodeRef NifConvertUtility2::getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot)
{
	NiObjectRef		pRootTree (NULL);
	NiNodeRef		pRootInput(NULL);

	//  get input nif
	pRootTree = ReadNifTree((const char*) fileName.c_str());

	//  NiNode as root
	if (DynamicCast<NiNode>(pRootTree) != NULL)
	{
		pRootInput = DynamicCast<NiNode>(pRootTree);
		_userMessages.push_back(logPreText + " root is NiNode.");
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

		_userMessages.push_back(logPreText + " root is NiTriShape.");
	}

	//  no known root type found
	if (pRootInput == NULL)
	{
		_userMessages.push_back(logPreText + " root has unhandled type: " + pRootTree->GetType().GetTypeName());
	}

	return pRootInput;
}

/*---------------------------------------------------------------------------*/
NiNodeRef NifConvertUtility2::convertNiNode(NiNodeRef pSrcNode, NiTriShapeRef pTmplNode, NiNodeRef pRootNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiNodeRef				pDstNode    (pSrcNode);
	vector<NiAVObjectRef>	srcShapeList(pDstNode->GetChildren());

	//  find NiAlphaProperty and use as template in sub-nodes
	if (DynamicCast<NiAlphaProperty>(pDstNode->GetPropertyByType(NiAlphaProperty::TYPE)) != NULL)
	{
		pTmplAlphaProp = DynamicCast<NiAlphaProperty>(pDstNode->GetPropertyByType(NiAlphaProperty::TYPE));
	}

	//  unlink protperties -> not used in new format
	pDstNode->ClearProperties();

	//  shift extra data to new version
	pDstNode->ShiftExtraData(VER_20_2_0_7);

	//  unlink children
	pDstNode->ClearChildren();

	//  iterate over source nodes and convert using template
	for (vector<NiAVObjectRef>::iterator  ppIter = srcShapeList.begin(); ppIter != srcShapeList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiTriShape(DynamicCast<NiTriShape>(*ppIter), pTmplNode, pTmplAlphaProp)));
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiNode(DynamicCast<NiNode>(*ppIter), pTmplNode, pRootNode, pTmplAlphaProp)));
		}
	}

	return pDstNode;
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility2::convertNiTriShape(NiTriShapeRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	BSLightingShaderPropertyRef	pTmplLShader(NULL);
	vector<NiPropertyRef>		dstPropList;
	short						bsPropIdx   (0);
	bool						forceAlpha  (pTmplAlphaProp != NULL);
	bool						hasAlpha    (false);

	//  NiTriShape is moved from src to dest. It's unlinked in calling function
	NiTriShapeRef		pDstNode(pSrcNode);
	NiGeometryDataRef	pDstGeo (pDstNode->GetData());

	//  force some data in destination shape
	pDstNode->SetCollisionObject(NULL);  //  no collision object here
	pDstNode->SetFlags          (14);    //  ???

	//  data node
	if ((pDstGeo != NULL) && (pTmplNode->GetData() != NULL))
	{
		pDstGeo->SetConsistencyFlags(pTmplNode->GetData()->GetConsistencyFlags());  //  nessessary ???
	}

	//  properties - get them from template
	for (short index(0); index < 2; ++index)
	{
		//  BSLightingShaderProperty
		if (DynamicCast<BSLightingShaderProperty>(pTmplNode->getBSProperty(index)) != NULL)
		{
			pTmplLShader = DynamicCast<BSLightingShaderProperty>(pTmplNode->getBSProperty(index));
		}
		//  NiAlphaProperty
		else if (DynamicCast<NiAlphaProperty>(pTmplNode->getBSProperty(index)) != NULL)
		{
			pTmplAlphaProp = DynamicCast<NiAlphaProperty>(pTmplNode->getBSProperty(index));
		}
	}  //  for (short index(0); index < 2; ++index)

	//  parse properties of destination node
	dstPropList = pDstNode->GetProperties();

	for (vector<NiPropertyRef>::iterator  ppIter = dstPropList.begin(); ppIter != dstPropList.end(); ppIter++)
	{
		//  NiAlphaProperty
		if (DynamicCast<NiAlphaProperty>(*ppIter) != NULL)
		{
			NiAlphaPropertyRef	pPropAlpha(DynamicCast<NiAlphaProperty>(*ppIter));

			//  set values from template
			pPropAlpha->SetFlags        (pTmplAlphaProp->GetFlags());
			pPropAlpha->SetTestThreshold(pTmplAlphaProp->GetTestThreshold());

			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);

			//  set new property to node
			pDstNode->setBSProperty(bsPropIdx++, &(*pPropAlpha));

			//  own alpha, reset forced alpha
			forceAlpha = false;

			//  mark alpha property
			hasAlpha = true;
		}
		//  NiTexturingProperty
		else if (DynamicCast<NiTexturingProperty>(*ppIter) != NULL)
		{
			char*						pTextPos   (NULL);
			BSLightingShaderPropertyRef	pDstLShader(new BSLightingShaderProperty(*pTmplLShader));
			BSShaderTextureSetRef		pDstSText  (new BSShaderTextureSet());
			TexDesc						baseTex    ((DynamicCast<NiTexturingProperty>(*ppIter))->GetTexture(BASE_MAP));
			char						fileName[1000] = {0};
			char						textName[1000] = {0};

			//  copy textures from template to copy
			pDstSText->setTextures(pTmplLShader->getTextureSet()->getTextures());

			//  set new texture names
			sprintf(fileName, "%s", (const char*) _pathTexture.c_str());
			baseTex.source->GetTextureFileName().copy(textName, 1000, 0);
			pTextPos = strrchr(textName, '\\');
			if (pTextPos == NULL)
			{
				pTextPos = strrchr(textName, '/');
			}
			if (pTextPos != NULL)
			{
				strcat(fileName, ++pTextPos);
			}
			else
			{
				strcat(fileName, textName);
			}
			fileName[strlen(fileName) - 3] = 0;
			strcat(fileName, "dds");

			//  set new texture map
			pDstSText->setTexture(0, fileName);

			_usedTextures.insert(fileName);
			fileName[strlen(fileName) - 4] = 0;
			strcat(fileName, "_n.dds");

			//  set new normal map
			pDstSText->setTexture(1, fileName);

			//  add texture set to texture property
			pDstLShader->setTextureSet(pDstSText);

			//  check for existing vertex colors
			if ((pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0) && ((pDstLShader->getShaderFlags2() & Niflib::SLSF2_VERTEX_COLOR) != 0))
			{
				switch (_vcHandling)
				{
					case NCU_VC_REMOVE_FLAG:
					{
						pDstLShader->setShaderFlags2((pDstLShader->getShaderFlags2() & ~Niflib::SLSF2_VERTEX_COLOR));
						break;
					}

					case NCU_VC_ADD_DEFAULT:
					{
						pDstGeo->SetVertexColors(vector<Color4>(pDstGeo->GetVertexCount(), _vcDefaultColor));
						break;
					}
				}
			}  //  if ((pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0) && ...

			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);

			//  set new property to node
			pDstNode->setBSProperty(bsPropIdx++, &(*pDstLShader));
		}
		//  NiMaterialProperty
		else if (DynamicCast<NiMaterialProperty>(*ppIter) != NULL)
		{
			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);
		}
	}  //  for (vector<NiPropertyRef>::iterator  ppIter = dstPropList.begin(); ppIter != dstPropList.end(); ppIter++)

	//  add forced NiAlphaProperty?
	if (forceAlpha)
	{
		NiAlphaPropertyRef	pPropAlpha(new NiAlphaProperty());

		//  set values from template
		pPropAlpha->SetFlags        (pTmplAlphaProp->GetFlags());
		pPropAlpha->SetTestThreshold(pTmplAlphaProp->GetTestThreshold());

		//  set new property to node
		pDstNode->setBSProperty(bsPropIdx++, &(*pPropAlpha));

		//  mark alpha property
		hasAlpha = true;

	}  //  if (forceAlpha)

	//  add default vertex colors if alpha property and no colors
	if (hasAlpha && (pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0))
	{
		pDstGeo->SetVertexColors(vector<Color4>(pDstGeo->GetVertexCount(), _vcDefaultColor));
	}

	return  pDstNode;
}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility2::convertShape(string fileNameSrc, string fileNameDst, string fileNameTmpl)
{
	NiNodeRef				pRootInput     (NULL);
	NiNodeRef				pRootOutput    (NULL);
	NiNodeRef				pRootTemplate  (NULL);
	NiTriShapeRef			pNiTriShapeTmpl(NULL);
	vector<NiAVObjectRef>	srcChildList;
	bool					fakedRoot      (false);

	//  test on existing file names
	if (fileNameSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDst.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	_userMessages.push_back("Source:\n  "      + fileNameSrc);
	_userMessages.push_back("Template:\n  "    + fileNameTmpl);
	_userMessages.push_back("Destination:\n  " + fileNameDst);
	_userMessages.push_back("Texture:\n  "     + _pathTexture);

	//  initialize used texture list
	_usedTextures.clear();

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileNameSrc, "source", fakedRoot)) == NULL)
	{
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get shapes from template
	//  - shape root
	pNiTriShapeTmpl = DynamicCast<NiTriShape>(pRootTemplate->GetChildren().at(0));
	if (pNiTriShapeTmpl == NULL)
	{
		_userMessages.push_back("Template has no NiTriShape.");
	}

	//  template root is used as root of output
	pRootOutput = pRootTemplate;

	//   get rid of unwanted subnodes
	pRootOutput->ClearChildren();           //  remove all children
	pRootOutput->SetCollisionObject(NULL);  //  unlink collision object
	//  hold extra data and property nodes

	//  get list of children from input node
	srcChildList = pRootInput->GetChildren();

	//  unlink children 'cause moved to output
	pRootInput->ClearChildren();

	//  iterate over source nodes and convert using template
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			pRootOutput->AddChild(&(*convertNiTriShape(DynamicCast<NiTriShape>(*ppIter), pNiTriShapeTmpl)));
		}
/*
		//  RootCollisionNode
		else if ((DynamicCast<RootCollisionNode>(*ppIter) != NULL) && ((_cnHandling == NCU_CN_INPUT_COLLISION) || (_cnHandling == NCU_CN_INPUT_COL_FB_SHAPE)))
		{
			pRootOutput->SetCollisionObject(convertCollNode(DynamicCast<NiNode>(*ppIter), pCollNodeTmpl, &(*pRootOutput)));
			hasCollNode = (pRootOutput->GetCollisionObject() != NULL);
		}
*/
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			pRootOutput->AddChild(&(*convertNiNode(DynamicCast<NiNode>(*ppIter), pNiTriShapeTmpl, pRootOutput)));
		}
	}

	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootOutput, NifInfo(VER_20_2_0_7, 12, 83));

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility2::addCollision(string fileNameCollSrc, string fileNameNifDst, string fileNameCollTmpl)
{
	NiNodeRef				pRootInput   (NULL);
	NiNodeRef				pRootTemplate(NULL);
	bhkCollisionObjectRef	pCollNodeTmpl(NULL);
	vector<hkGeometry*>		geometryAry;
	vector<NiAVObjectRef>	srcChildList;
	bool					fakedRoot    (false);

	//  test on existing file names
	if (fileNameCollSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameNifDst.empty())			return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameCollTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	_userMessages.push_back("CollSource:\n  "   + fileNameCollSrc);
	_userMessages.push_back("VollTemplate:\n  " + fileNameCollTmpl);
	_userMessages.push_back("Destination:\n  "  + fileNameNifDst);

	//  get geometry data
	switch (fileNameCollSrc[fileNameCollSrc.size() - 3])
	{
		//  from OBJ file
		case 'O':
		case 'o':
		{
			_userMessages.push_back("Getting geometry from OBJ.");
			getGeometryFromObjFile(fileNameCollSrc, geometryAry);
			break;
		}
		//  from NIF file
		case 'N':
		case 'n':
		{
			_userMessages.push_back("Getting geometry from NIF.");
			getGeometryFromNifFile(fileNameCollSrc, geometryAry);
			break;
		}
		//  from 3DS file
		case '3':
		{
			//  would be nice ;-)
			_userMessages.push_back("Getting geometry from 3DS.");
			break;
		}
	}  //  switch (fileNameCollSrc[fileNameCollSrc.size() - 3])

	//  early break on missing geometry data
	if (geometryAry.size() <= 0)
	{
		_userMessages.push_back("Can't get geometry from input file.");
		return NCU_ERROR_CANT_GET_GEOMETRY;
	}

	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameCollTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get shapes from template
	//  - collision root
	pCollNodeTmpl = DynamicCast<bhkCollisionObject>(pRootTemplate->GetCollisionObject());
	if (pCollNodeTmpl == NULL)
	{
		_userMessages.push_back("Template has no bhkCollisionObject.");
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get root node from destination
	if ((pRootInput = getRootNodeFromNifFile(fileNameNifDst, "target", fakedRoot)) == NULL)
	{
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  create and add collision node to target
	pRootInput->SetCollisionObject(createCollNode(geometryAry, pCollNodeTmpl, pRootInput));

	//  get list of children from input node
	srcChildList = pRootInput->GetChildren();

	//  iterate over source nodes and remove possible old-style collision node
	for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ppIter != srcChildList.end(); ppIter++)
	{
		//  RootCollisionNode
		if (DynamicCast<RootCollisionNode>(*ppIter) != NULL)
		{
			pRootInput->RemoveChild(*ppIter);
		}
	}  //  for (vector<NiAVObjectRef>::iterator  ppIter = srcChildList.begin(); ....

	//  write modified nif file
	WriteNifTree((const char*) fileNameNifDst.c_str(), pRootInput, NifInfo(VER_20_2_0_7, 12, 83));

	return NCU_OK;
}

/*---------------------------------------------------------------------------*/
bhkCollisionObjectRef NifConvertUtility2::createCollNode(vector<hkGeometry*>& geometryAry, bhkCollisionObjectRef pTmplNode, NiNodeRef pRootNode)
{
	//  template collision node will be output collision node. it's unlinked from root in calling function
	bhkCollisionObjectRef	pDstNode(pTmplNode);

	//  parse collision node subtrees and correct targets
	bhkRigidBodyRef		pTmplBody(DynamicCast<bhkRigidBody>(pDstNode->GetBody()));

	//  bhkRigidBody found
	if (pTmplBody != NULL)
	{
		bhkMoppBvTreeShapeRef	pTmplMopp(DynamicCast<bhkMoppBvTreeShape>(pTmplBody->GetShape()));

		//  bhkMoppBvTreeShape found
		if (pTmplMopp != NULL)
		{
			bhkCompressedMeshShapeRef	pTmplCShape(DynamicCast<bhkCompressedMeshShape>(pTmplMopp->GetShape()));

			//  bhkCompressedMeshShape found
			if (pTmplCShape != NULL)
			{
				bhkCompressedMeshShapeDataRef	pData(pTmplCShape->GetData());

				//  bhkCompressedMeshShapeData found
				if (pData != NULL)
				{
					//  fill in Havok data into Nif structures
					injectCollisionData(geometryAry, pTmplMopp, pData);

					//  set new target
					pTmplCShape->SetTarget(pRootNode);

				}  //  if (pData != NULL)
			}  //  if (pTmplCShape != NULL)
		}  //  if (pTmplMopp != NULL)
	}  //  if (pTmplBody != NULL)

	//  remove target from destination node
	pDstNode->SetTarget(NULL);

	return pDstNode;
}

/*---------------------------------------------------------------------------*/
bool NifConvertUtility2::injectCollisionData(vector<hkGeometry*>& geometryAry, bhkMoppBvTreeShapeRef pMoppShape, bhkCompressedMeshShapeDataRef pData)
{
	if (pMoppShape == NULL)   return false;
	if (pData      == NULL)   return false;
	if (geometryAry.empty())  return false;

	//----  Havok  ----  START
	hkpCompressedMeshShape*         pCompMesh  (NULL);
	hkpMoppCode*                    pMoppCode  (NULL);
	hkpMoppBvTreeShape*             pMoppBvTree(NULL);
	hkpCompressedMeshShapeBuilder   shapeBuilder;
	hkpMoppCompilerInput            mci;
	int                             subPartId  (0);

	//  initialize shape Builder
	shapeBuilder.m_stripperPasses = 5000;

	//  create compressedMeshShape
	pCompMesh = shapeBuilder.createMeshShape(0.001f, hkpCompressedMeshShape::MATERIAL_SINGLE_VALUE_PER_CHUNK);

	//  add geometries to compressedMeshShape
	for (vector<hkGeometry*>::iterator geoIter = geometryAry.begin(); geoIter != geometryAry.end(); geoIter++)
	{
		subPartId = shapeBuilder.beginSubpart(pCompMesh);
		shapeBuilder.addGeometry(**geoIter, hkMatrix4::getIdentity(), pCompMesh);
		shapeBuilder.endSubpart(pCompMesh);
		shapeBuilder.addInstance(subPartId, hkMatrix4::getIdentity(), pCompMesh);
	}

	//  create welding info
	mci.m_enableChunkSubdivision = true;
	pMoppCode   = hkpMoppUtility::buildCode(pCompMesh, mci);
	pMoppBvTree = new hkpMoppBvTreeShape(pCompMesh, pMoppCode);
	hkpMeshWeldingUtility::computeWeldingInfo(pCompMesh, pMoppBvTree, hkpWeldingUtility::WELDING_TYPE_TWO_SIDED);
	//----  Havok  ----  END

	//----  Merge  ----  START
	hkArray<hkpCompressedMeshShape::Chunk>  chunkListHvk;
	vector<bhkCMSDChunk>                    chunkListNif = pData->GetChunks();
	vector<Niflib::byte>                    tByteVec;
	vector<Vector4>                         tVec4Vec;
	vector<bhkCMSDBigTris>                  tBTriVec;
	vector<bhkCMSDTransform>                tTranVec;
	short                                   chunkIdxNif(0);

	//  --- modify MoppBvTree ---
	//  set origin
	pMoppShape->SetMoppOrigin(Vector3(pMoppBvTree->getMoppCode()->m_info.m_offset(0), pMoppBvTree->getMoppCode()->m_info.m_offset(1), pMoppBvTree->getMoppCode()->m_info.m_offset(2)));

	//  set scale
	pMoppShape->SetMoppScale(pMoppBvTree->getMoppCode()->m_info.getScale());

	//  copy mopp data
	tByteVec.resize(pMoppBvTree->m_moppDataSize);
	tByteVec[0] = pMoppBvTree->m_moppData[pMoppBvTree->m_moppDataSize - 1];
	for (hkUint32 i(0); i < (pMoppBvTree->m_moppDataSize - 1); ++i)
	{
		tByteVec[i+1] = pMoppBvTree->m_moppData[i];
	}
	pMoppShape->SetMoppCode(tByteVec);

	//  set boundings
	pData->SetBoundsMin(Vector4(pCompMesh->m_bounds.m_min(0), pCompMesh->m_bounds.m_min(1), pCompMesh->m_bounds.m_min(2), pCompMesh->m_bounds.m_min(3)));
	pData->SetBoundsMax(Vector4(pCompMesh->m_bounds.m_max(0), pCompMesh->m_bounds.m_max(1), pCompMesh->m_bounds.m_max(2), pCompMesh->m_bounds.m_max(3)));

	//  resize and copy bigVerts
	pData->SetNumBigVerts(pCompMesh->m_bigVertices.getSize());
	tVec4Vec = pData->GetBigVerts();
	tVec4Vec.resize(pData->GetNumBigVerts());
	for (unsigned int idx(0); idx < pData->GetNumBigVerts(); ++idx)
	{
		tVec4Vec[idx].x = pCompMesh->m_bigVertices[idx](0);
		tVec4Vec[idx].y = pCompMesh->m_bigVertices[idx](1);
		tVec4Vec[idx].z = pCompMesh->m_bigVertices[idx](2);
		tVec4Vec[idx].w = pCompMesh->m_bigVertices[idx](3);
	}
	pData->SetBigVerts(tVec4Vec);

	//  resize and copy bigTris
	pData->SetNumBigTris(pCompMesh->m_bigTriangles.getSize());
	tBTriVec = pData->GetBigTris();
	tBTriVec.resize(pData->GetNumBigTris());
	for (unsigned int idx(0); idx < pData->GetNumBigTris(); ++idx)
	{
		tBTriVec[idx].triangle1     = pCompMesh->m_bigTriangles[idx].m_a;
		tBTriVec[idx].triangle2     = pCompMesh->m_bigTriangles[idx].m_b;
		tBTriVec[idx].triangle3     = pCompMesh->m_bigTriangles[idx].m_c;
		tBTriVec[idx].unknownInt1   = pCompMesh->m_bigTriangles[idx].m_material;
		tBTriVec[idx].unknownShort1 = pCompMesh->m_bigTriangles[idx].m_weldingInfo;
	}
	pData->SetBigTris(tBTriVec);

	//  resize and copy transform data
	pData->SetNumTransforms(pCompMesh->m_transforms.getSize());
	tTranVec = pData->GetChunkTransforms();
	tTranVec.resize(pData->GetNumTransforms());
	for (unsigned int idx(0); idx < pData->GetNumTransforms(); ++idx)
	{
		tTranVec[idx].translation.x = pCompMesh->m_transforms[idx].m_translation(0);
		tTranVec[idx].translation.y = pCompMesh->m_transforms[idx].m_translation(1);
		tTranVec[idx].translation.z = pCompMesh->m_transforms[idx].m_translation(2);
		tTranVec[idx].translation.w = pCompMesh->m_transforms[idx].m_translation(3);
		tTranVec[idx].rotation.x    = pCompMesh->m_transforms[idx].m_rotation(0);
		tTranVec[idx].rotation.y    = pCompMesh->m_transforms[idx].m_rotation(1);
		tTranVec[idx].rotation.z    = pCompMesh->m_transforms[idx].m_rotation(2);
		tTranVec[idx].rotation.w    = pCompMesh->m_transforms[idx].m_rotation(3);
	}
	pData->SetChunkTransforms(tTranVec);

	//  get chunk list from mesh
	chunkListHvk = pCompMesh->m_chunks;

	// resize nif chunk list
	chunkListNif.resize(chunkListHvk.getSize());

	//  for each chunk
	for (hkArray<hkpCompressedMeshShape::Chunk>::iterator pCIterHvk = pCompMesh->m_chunks.begin(); pCIterHvk != pCompMesh->m_chunks.end(); pCIterHvk++)
	{
		//  get nif chunk
		bhkCMSDChunk&	chunkNif = chunkListNif[chunkIdxNif];

		//  set offset => translation
		chunkNif.translation.x = pCIterHvk->m_offset(0);
		chunkNif.translation.y = pCIterHvk->m_offset(1);
		chunkNif.translation.z = pCIterHvk->m_offset(2);
		chunkNif.translation.w = pCIterHvk->m_offset(3);

		//  force flags to fixed values
		chunkNif.materialIndex  = 0;
		chunkNif.unknownShort1  = 65535;
		chunkNif.transformIndex = pCIterHvk->m_transformIndex;

		//  vertices
		chunkNif.numVertices = pCIterHvk->m_vertices.getSize();
		chunkNif.vertices.resize(chunkNif.numVertices);
		for (unsigned int i(0); i < chunkNif.numVertices; ++i)
		{
			chunkNif.vertices[i] = pCIterHvk->m_vertices[i];
		}

		//  indices
		chunkNif.numIndices = pCIterHvk->m_indices.getSize();
		chunkNif.indices.resize(chunkNif.numIndices);
		for (unsigned int i(0); i < chunkNif.numIndices; ++i)
		{
			chunkNif.indices[i] = pCIterHvk->m_indices[i];
		}

		//  strips
		chunkNif.numStrips = pCIterHvk->m_stripLengths.getSize();
		chunkNif.strips.resize(chunkNif.numStrips);
		for (unsigned int i(0); i < chunkNif.numStrips; ++i)
		{
			chunkNif.strips[i] = pCIterHvk->m_stripLengths[i];
		}

		//  welding
		chunkNif.numIndices2 = pCIterHvk->m_weldingInfo.getSize();
		chunkNif.indices2.resize(chunkNif.numIndices2);
		for (unsigned int i(0); i < chunkNif.numIndices2; ++i)
		{
			chunkNif.indices2[i] = pCIterHvk->m_weldingInfo[i];
		}

		//  next chunk
		++chunkIdxNif;

	}  //  for (hkArray<hkpCompressedMeshShape::Chunk>::iterator pCIterHvk = 

	//  set modified chunk list to compressed mesh shape data
	pData->SetChunks(chunkListNif);
	//----  Merge  ----  END

	return true;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility2::setTexturePath(string pathTexture)
{
	_pathTexture = pathTexture;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility2::setVertexColorHandling(VertexColorHandling vcHandling)
{
	_vcHandling = vcHandling;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility2::setCollisionNodeHandling(CollisionNodeHandling cnHandling)
{
	_cnHandling = cnHandling;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility2::setDefaultVertexColor(Color4 defaultColor)
{
	_vcDefaultColor = defaultColor;
}

/*---------------------------------------------------------------------------*/
vector<string>& NifConvertUtility2::getUserMessages()
{
	return _userMessages;
}

/*---------------------------------------------------------------------------*/
set<string>& NifConvertUtility2::getUsedTextures()
{
	return _usedTextures;
}
