#pragma once

#include "obj/nitrishape.h"
#include "obj/rootcollisionnode.h"
#include "obj/bhkcollisionobject.h"
#include "obj/bsfadenode.h"
#include "obj/nialphaproperty.h"
#include "obj/bhkMoppBvTreeShape.h"
#include "obj/bhkCompressedMeshShapeData.h"
#include <set>

#include <Common/Base/Types/Geometry/hkGeometry.h>


using namespace Niflib;

//  return codes
#define   NCU_OK                            0x00
#define   NCU_ERROR_MISSING_FILE_NAME       0x01
#define   NCU_ERROR_MISSING_TEXTURE_NAME    0x02
#define   NCU_ERROR_CANT_OPEN_INPUT         0x03
#define   NCU_ERROR_CANT_OPEN_TEMPLATE      0x04
#define   NCU_ERROR_CANT_OPEN_OUTPUT        0x05

//  vertex color handling
#define   NCU_VC_REMOVE_FLAG                0x00
#define   NCU_VC_ADD_COLORS                 0x01



class NifConvertUtility
{
  private:
    set<string>     _usedTextures;
    vector<string>  _userMessages;
    string          _pathTexture;
    unsigned char   _vcHandling;


    virtual NiTriShapeRef           convertNiTriShape(NiTriShapeRef srcNode, NiTriShapeRef tmplNode, NiAlphaPropertyRef tmplAlpha=NULL);
    virtual NiNodeRef               convertNiNode(NiNodeRef srcNode, NiTriShapeRef tmplNode, NiAlphaPropertyRef tmplAlpha=NULL);
    virtual bhkCollisionObjectRef   convertCollNode(RootCollisionNodeRef srcNode, bhkCollisionObjectRef tmplNode, NiAVObjectRef rootOutput);

    virtual unsigned int            getGeometryFromCollNode(RootCollisionNodeRef srcNode, vector<hkGeometry*>& geometryAry);
    virtual bool                    fillCollNodes(vector<hkGeometry*>& geometryAry, bhkMoppBvTreeShapeRef pMoppShape, bhkCompressedMeshShapeDataRef pData);

  public:
                                    NifConvertUtility();
    virtual                         ~NifConvertUtility();

    virtual unsigned short          convertNif(string fileNameSrc, string fileNameTmpl, string fileNameOut, string pathTexture, unsigned char vcHandling=NCU_VC_REMOVE_FLAG);

    virtual set<string>             getUsedTextures();
    virtual vector<string>          getUserMessages();
};
