#include "NifConvertUtility.h"

#include "niflib.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/NiTexturingProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiMaterialProperty.h"
#include "obj/nitrishapedata.h"
#include "obj/bhkRigidBody.h"
#include "obj/bhkCompressedMeshShape.h"

#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h>
#include <Physics/Collide/Shape/Compound/Collection/CompressedMesh/hkpCompressedMeshShapeBuilder.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>
#include <Physics/Collide/Util/Welding/hkpMeshWeldingUtility.h>

#pragma warning ( disable : 4482 )

NifConvertUtility::NifConvertUtility()
  : _vcHandling(NCU_VC_REMOVE_FLAG)
{
}

NifConvertUtility::~NifConvertUtility()
{
}

set<string> NifConvertUtility::getUsedTextures()
{
  return _usedTextures;
}

vector<string> NifConvertUtility::getUserMessages()
{
  return _userMessages;
}

unsigned short NifConvertUtility::convertNif(string fileNameSrc, string fileNameTmpl, string fileNameOut, string pathTexture, unsigned char vcHandling)
{
  NiNodeRef             pRootInput      = NULL;
  NiNodeRef             pRootTemplate   = NULL;
  NiNodeRef             pRootOutput     = NULL;
  NiObjectRef           pRootTree       = NULL;
  bhkCollisionObjectRef pCollNodeTmpl   = NULL;
  NiTriShapeRef         pNiTriShapeTmpl = NULL;
  vector<NiAVObjectRef> srcShapeList;
  bool                  fakedRoot  (false);
  bool                  hasCollNode(false);

  if (fileNameSrc.empty())      return NCU_ERROR_MISSING_FILE_NAME;
  if (fileNameTmpl.empty())     return NCU_ERROR_MISSING_FILE_NAME;
  if (fileNameOut.empty())      return NCU_ERROR_MISSING_FILE_NAME;
  if (pathTexture.empty())      return NCU_ERROR_MISSING_TEXTURE_NAME;

  //  some messages
  _userMessages.clear();
  _userMessages.push_back("Source: "      + fileNameSrc);
  _userMessages.push_back("Template: "    + fileNameTmpl);
  _userMessages.push_back("Destination: " + fileNameOut);
  _userMessages.push_back("Texture: "     + pathTexture);

  //  set actual texture path
  _pathTexture = pathTexture;

  //  set handling of vertex colors
  _vcHandling = vcHandling;

  //  reset vector of used textures
  _usedTextures.clear();

  //  get input nif
  pRootTree  = ReadNifTree((const char*) fileNameSrc.c_str());

  //  NiNode as root
  if (DynamicCast<NiNode>(pRootTree) != NULL)
  {
    pRootInput = DynamicCast<NiNode>(pRootTree);
    _userMessages.push_back("Source root is NiNode.");
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

    _userMessages.push_back("Source root is NiTriShape.");
  }

  if (pRootInput == NULL)
  {
    _userMessages.push_back("Source root has unhandled type: " + pRootTree->GetType().GetTypeName());
    return NCU_ERROR_CANT_OPEN_INPUT;
  }

  //  get template nif
  pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameTmpl.c_str()));
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
  }

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
  srcShapeList = pRootInput->GetChildren();

  //  unlink children 'cause moved to output
  pRootInput->ClearChildren();

  //  iterate over source nodes and convert using template
  for (vector<NiAVObjectRef>::iterator  nodeIter = srcShapeList.begin(); nodeIter != srcShapeList.end(); nodeIter++)
  {
    //  NiTriShape
    if (DynamicCast<NiTriShape>(*nodeIter) != NULL)
    {
      pRootOutput->AddChild(&(*convertNiTriShape(DynamicCast<NiTriShape>(*nodeIter), pNiTriShapeTmpl)));
    }
    //  RootCollisionNode
    else if (DynamicCast<RootCollisionNode>(*nodeIter) != NULL)
    {
      pRootOutput->SetCollisionObject(convertCollNode(DynamicCast<RootCollisionNode>(*nodeIter), pCollNodeTmpl, &(*pRootOutput)));
      hasCollNode = true;
    }
    //  NiNode (and derived classes?)
    else if (DynamicCast<NiNode>(*nodeIter) != NULL)
    {
      pRootOutput->AddChild(&(*convertNiNode(DynamicCast<NiNode>(*nodeIter), pNiTriShapeTmpl)));
    }
  }  //  for (vector<NiAVObjectRef>::iterator  nodeIter = srcShapeList...

  if (!hasCollNode)
  {
    _userMessages.push_back("Nif has no collision node!");
  }

  //  build NifInfo
  NifInfo   nifInfo(VER_20_2_0_7, 12, 83);

  //  write modified nif file
  WriteNifTree((const char*) fileNameOut.c_str(), pRootOutput, nifInfo);

  return NCU_OK;
}

NiTriShapeRef NifConvertUtility::convertNiTriShape(NiTriShapeRef srcNode, NiTriShapeRef tmplNode, NiAlphaPropertyRef tmplAlpha)
{
  BSLightingShaderPropertyRef   tmplLShader = NULL;
  vector<NiPropertyRef>         dstPList;
  short                         bsPropIdx (0);
  bool                          forceAlpha(tmplAlpha != NULL);
  bool                          hasAlpha  (false);

  //  NiTriShape is moved from src to dest. It's unlinked in calling function
  NiTriShapeRef     dstNode = srcNode;
  NiGeometryDataRef dstGeo  = dstNode->GetData();

  //  force some data in destination shape
  dstNode->SetCollisionObject(NULL);  //  no collision object here
  dstNode->SetFlags          (14);    //  ???

  //  data node
  if ((dstGeo != NULL) && (tmplNode->GetData() != NULL))
  {
    NiGeometryDataRef tmplGeo = tmplNode->GetData();

    dstGeo->SetConsistencyFlags(tmplGeo->GetConsistencyFlags());  //  nessessary ???

  }  //  if ((srcNode->GetData() != NULL) && (tmplNode->GetData() != NULL))

  //  properties - get them from template
  for (short index(0); index < 2; ++index)
  {
    //  BSLightingShaderProperty
    if (DynamicCast<BSLightingShaderProperty>(tmplNode->getBSProperty(index)) != NULL)
    {
      tmplLShader = DynamicCast<BSLightingShaderProperty>(tmplNode->getBSProperty(index));
    }
    //  NiAlphaProperty
    else if (DynamicCast<NiAlphaProperty>(tmplNode->getBSProperty(index)) != NULL)
    {
      tmplAlpha = DynamicCast<NiAlphaProperty>(tmplNode->getBSProperty(index));
    }
  }  //  for (short index(0); index < 2; ++index)

  //  parse properties of destination node
  dstPList = dstNode->GetProperties();

  for (vector<NiPropertyRef>::iterator  propIter = dstPList.begin(); propIter != dstPList.end(); propIter++)
  {
    //  NiAlphaProperty
    if (DynamicCast<NiAlphaProperty>(*propIter) != NULL)
    {
      NiAlphaPropertyRef  propAlpha = DynamicCast<NiAlphaProperty>(*propIter);

      //  set values from template
      propAlpha->SetFlags        (tmplAlpha->GetFlags());
      propAlpha->SetTestThreshold(tmplAlpha->GetTestThreshold());
 
      //  remove property from node
      dstNode->RemoveProperty(*propIter);

      //  set new property to node
      dstNode->setBSProperty(bsPropIdx++, &(*propAlpha));

      //  own alpha, reset forced alpha
      forceAlpha = false;

      //  mark alpha property
      hasAlpha   = true;
    }
    //  NiTexturingProperty
    else if (DynamicCast<NiTexturingProperty>(*propIter) != NULL)
    {
      char*                       pTextPos   = NULL;
      BSLightingShaderPropertyRef dstLShader = new BSLightingShaderProperty(*tmplLShader);
      BSShaderTextureSetRef       dstSText   = new BSShaderTextureSet();
      TexDesc                     baseTex    = (DynamicCast<NiTexturingProperty>(*propIter))->GetTexture(BASE_MAP);
      char                        fileName[1000] = {0};
      char                        textName[1000] = {0};

      //  copy textures from template to copy
      dstSText->setTextures(tmplLShader->getTextureSet()->getTextures());

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
      dstSText->setTexture(0, fileName);

      _usedTextures.insert(fileName);
      fileName[strlen(fileName) - 4] = 0;
      strcat(fileName, "_n.dds");

      //  set new normal map
      dstSText->setTexture(1, fileName);

      //  add texture set to texture property
      dstLShader->setTextureSet(dstSText);

      //  check for existing vertex colors
      if ((dstGeo != NULL) && (dstGeo->GetColors().size() <= 0) && ((dstLShader->getShaderFlags2() & SkyrimLightingShaderFlags2::SLSF2_VERTEX_COLOR) != 0))
      {
        switch (_vcHandling)
        {
          case NCU_VC_REMOVE_FLAG:
          {
            dstLShader->setShaderFlags2((dstLShader->getShaderFlags2() & ~SkyrimLightingShaderFlags2::SLSF2_VERTEX_COLOR));
            break;
          }

          case NCU_VC_ADD_COLORS:
          {
            Color4          tColor (1.0f, 1.0f, 1.0f);
            vector<Color4>  vColors(dstGeo->GetVertexCount(), tColor);

            dstGeo->SetVertexColors(vColors);
            break;
          }
        }
      }  //  if ((dstGeo != NULL) && (dstGeo->GetColors().size() <= 0) && ...

      //  remove property from node
      dstNode->RemoveProperty(*propIter);

      //  set new property to node
      dstNode->setBSProperty(bsPropIdx++, &(*dstLShader));
    }
    //  NiMaterialProperty
    else if (DynamicCast<NiMaterialProperty>(*propIter) != NULL)
    {
      //  remove property from node
      dstNode->RemoveProperty(*propIter);
    }
  }  //  for (vector<NiPropertyRef>::iterator  propIter = dstPList.begin(); ...

  //  add forced NiAlphaProperty?
  if (forceAlpha)
  {
    NiAlphaPropertyRef  propAlpha = new NiAlphaProperty();

    //  set values from template
    propAlpha->SetFlags        (tmplAlpha->GetFlags());
    propAlpha->SetTestThreshold(tmplAlpha->GetTestThreshold());

    //  set new property to node
    dstNode->setBSProperty(bsPropIdx++, &(*propAlpha));

    //  mark alpha property
    hasAlpha = true;

  }  //  if (forceAlpha)

  //  add default vertex colors if alpha property and no colors
  if (hasAlpha && (dstGeo != NULL))
  {
    //  exisitng colors?
    if (dstGeo->GetColors().size() <= 0)
    {
      Color4          tColor (1.0f, 1.0f, 1.0f);
      vector<Color4>  vColors(dstGeo->GetVertexCount(), tColor);

      dstGeo->SetVertexColors(vColors);

    }  //  if (dstGeo->GetColors().size() <= 0)
  }  //  if (hasAlpha)

  return dstNode;
}

NiNodeRef NifConvertUtility::convertNiNode(NiNodeRef srcNode, NiTriShapeRef tmplNode, NiAlphaPropertyRef tmplAlpha)
{
  NiNodeRef             dstNode      = srcNode;
  vector<NiAVObjectRef> srcShapeList = dstNode->GetChildren();

  //  find NiAlphaProperty and use as template in sub-nodes
  if (DynamicCast<NiAlphaProperty>(dstNode->GetPropertyByType(NiAlphaProperty::TYPE)) != NULL)
  {
    tmplAlpha = DynamicCast<NiAlphaProperty>(dstNode->GetPropertyByType(NiAlphaProperty::TYPE));
  }

  //  unlink protperties -> not used in new format
  dstNode->ClearProperties();

  //  shift extra data to new version
  dstNode->ShiftExtraData(VER_20_2_0_7);

  //  unlink children
  dstNode->ClearChildren();

  //  iterate over source nodes and convert using template
  for (vector<NiAVObjectRef>::iterator  nodeIter = srcShapeList.begin(); nodeIter != srcShapeList.end(); nodeIter++)
  {
    //  NiTriShape
    if (DynamicCast<NiTriShape>(*nodeIter) != NULL)
    {
      dstNode->AddChild(&(*convertNiTriShape(DynamicCast<NiTriShape>(*nodeIter), tmplNode, tmplAlpha)));
    }
    //  RootCollisionNode
    else if (DynamicCast<RootCollisionNode>(*nodeIter) != NULL)
    {
      //dstNode->SetCollisionObject(convertCollNode(pCollNodeInput, pCollNodeTmpl, &(*pRootOutput)));
    }
    //  NiNode (and derived classes?)
    else if (DynamicCast<NiNode>(*nodeIter) != NULL)
    {
      dstNode->AddChild(&(*convertNiNode(DynamicCast<NiNode>(*nodeIter), tmplNode, tmplAlpha)));
    }
  }

  return dstNode;
}

bhkCollisionObjectRef NifConvertUtility::convertCollNode(RootCollisionNodeRef srcNode, bhkCollisionObjectRef tmplNode, NiAVObjectRef rootOutput)
{
  vector<hkGeometry*>   geometryAry;
  unsigned int          geoCount(getGeometryFromCollNode(srcNode, geometryAry));

  //  template collision node will be output collision node. it's unlinked from root in calling function
  bhkCollisionObjectRef dstNode = tmplNode;

  //  parse collision node subtrees and correct targets
  bhkRigidBodyRef   tmplBody = DynamicCast<bhkRigidBody>(dstNode->GetBody());

  //  bhkRigidBodyRef found
  if (tmplBody != NULL)
  {
    bhkMoppBvTreeShapeRef tmplMopp = DynamicCast<bhkMoppBvTreeShape>(tmplBody->GetShape());

    //  bhkMoppBvTreeShapeRef found
    if (tmplMopp != NULL)
    {
      bhkCompressedMeshShapeRef tmplCShape = DynamicCast<bhkCompressedMeshShape>(tmplMopp->GetShape());

      //  bhkCompressedMeshShapeRef found
      if (tmplCShape != NULL)
      {
        bhkCompressedMeshShapeDataRef pData = tmplCShape->GetData();

        //  fill in Havok data into Nif structures
        fillCollNodes(geometryAry, tmplMopp, pData);

        //  set new target
        tmplCShape->SetTarget(rootOutput);

      }  //  if (tmplCShape != NULL)
    }  //  if (tmplMopp != NULL)
  }  //  if (tmplBody != NULL)

  return dstNode;
}

unsigned int NifConvertUtility::getGeometryFromCollNode(RootCollisionNodeRef srcNode, vector<hkGeometry*>& geometryAry)
{
  NiAVObjectRef           pChild    = NULL;
  vector<NiAVObjectRef>   childList = srcNode->GetChildren();
  Matrix44                transRoot = srcNode->GetLocalTransform();

  for (unsigned int i=0; i < childList.size(); ++i)
  {
    pChild = childList[i];

    //  try getting shape from collision objet
    NiTriShapeRef   pRef = DynamicCast<NiTriShape>(pChild);

    //  shape found
    if (pRef != NULL)
    {
      NiGeometryDataRef   pData = pRef->GetData();

      //  geometry shape
      if (pData != NULL)
      {
        hkGeometry::Triangle*           pTri       = NULL;
        hkGeometry*                     pTmpGeo    = NULL;
        NiTriShapeDataRef               pShapeData = DynamicCast<NiTriShapeData>(pData);
        Matrix44                        transNode  = pChild->GetLocalTransform();
        vector<Vector3>                 vertices   = pData->GetVertices();
        vector<Triangle>                triangles  = pShapeData->GetTriangles();
        hkArray<hkVector4>              vertAry;
        hkArray<hkGeometry::Triangle>   triAry;
        Vector3                         tVector;

        for (unsigned int idx(0); idx < vertices.size(); ++idx)
        {
          tVector  = vertices[idx];
          tVector  = transNode * tVector;
          tVector  = transRoot * tVector;
          tVector *= 0.0143f;

          vertAry.append(new hkVector4(tVector.x, tVector.y, tVector.z), 1);

          //vertAry.append(new hkVector4((hkReal) ((vertices[idx].x + pChild->GetLocalTranslation().x) * 0.0143), (hkReal) ((vertices[idx].y + pChild->GetLocalTranslation().y) * 0.0143), (hkReal) ((vertices[idx].z + pChild->GetLocalTranslation().z) * 0.0143)), 1);
        }

        for (unsigned int idx(0); idx < triangles.size(); ++idx)
        {
          pTri = new hkGeometry::Triangle();
          pTri->set(triangles[idx].v1, triangles[idx].v2, triangles[idx].v3);
          triAry.append(pTri, 1);
        }

        pTmpGeo = new hkGeometry();
        pTmpGeo->m_triangles = triAry;
        pTmpGeo->m_vertices  = vertAry;

        geometryAry.push_back(pTmpGeo);

      }  //  if (pData != NULL)
    }  //  if (pRef != NULL)
  }  //  for (i=0; i < childList.size(); ++i)

  return geometryAry.size();
}

bool NifConvertUtility::fillCollNodes(vector<hkGeometry*> &geometryAry, bhkMoppBvTreeShapeRef pMoppShape, bhkCompressedMeshShapeDataRef pData)
{
  if (pMoppShape == NULL)   return false;
  if (pData      == NULL)   return false;
  if (geometryAry.empty())  return false;

  //----  Havok  ----  START
  hkpCompressedMeshShape*         pCompMesh   (NULL);
  hkGeometry*                     pGeometry   (NULL); 
  hkpMoppCode*                    pMoppCode   (NULL);
  hkpMoppBvTreeShape*             pMoppBvTree (NULL);
  hkpCompressedMeshShapeBuilder   shapeBuilder;
  hkpMoppCompilerInput            mci;
  int                             subPartId   (0);

  //  initialize shape Builder
  shapeBuilder.m_stripperPasses = 5000;

  //  create compressedMeshShape
  pCompMesh = shapeBuilder.createMeshShape(0.001f, hkpCompressedMeshShape::MATERIAL_SINGLE_VALUE_PER_CHUNK);

  //  add geometries to compressedMeshShape
  for (vector<hkGeometry*>::iterator geoIter = geometryAry.begin(); geoIter != geometryAry.end(); geoIter++)
  {
    pGeometry = *geoIter;
    subPartId = shapeBuilder.beginSubpart(pCompMesh);
    shapeBuilder.addGeometry(*pGeometry, hkMatrix4::getIdentity(), pCompMesh);
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
    bhkCMSDChunk&  chunkNif = chunkListNif[chunkIdxNif];

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

