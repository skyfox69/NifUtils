#include "obj/bhkMoppBvTreeShape.h"
#include "obj/bhkCompressedMeshShapeData.h"

using namespace Niflib;

//  triangle shape defined by chunk data
struct FDTriangle
{
	FDTriangle*		pNext;
	Vector3*		  pP1;
	Vector3*		  pP2;
	Vector3*		  pP3;
	unsigned int	idx1;
	unsigned int	idx2;
	unsigned int	idx3;
	unsigned int	id;
	unsigned int	welding;
	int				    strip;

	FDTriangle() : pNext(NULL), pP1(NULL), pP2(NULL), pP3(NULL), idx1(0), idx2(0), idx3(0), id(0), welding(0), strip(-1) {};
	~FDTriangle() { if (pNext != NULL)  delete pNext; }
};

//  nif file utility class
class NifFile
{
  private:
    bhkCompressedMeshShapeDataRef           _pShapeData;
    bhkMoppBvTreeShapeRef                   _pMoppBvTree;
    NiObjectRef                             _pNifRoot;
    char*                                   _pCharBuffer;

    virtual char*                           formatIndent(const char* pRBuffer, const char* pCBuffer, const char* pABuffer, const short depth);
    virtual char*                           formatOutput(const char* pRBuffer, const char* pCBuffer, const char* pABuffer);
    virtual void                            printMoppCodeByIndent(FILE* pFileOut, const unsigned char* bbuffer, const int moppSize, int idxByte=0, const short depth=0, int triOff=0);
    virtual void                            printMoppCodeByLine  (FILE* pFileOut, const unsigned char* bbuffer, const int moppSize);
    virtual int                             buildTriangles(FDTriangle** ppActTri, Vector3** ppVertices, bhkCMSDChunk& chunkNif);

  public:
                                            NifFile          ();
    virtual                                 ~NifFile         ();
    virtual bool                            closeNif         ();
    virtual bool                            openNif          (const char* pFileName);
    virtual bool                            extractChunkData (const char* pFileName);
    virtual bool                            extractMoppData  (const char* pFileName);
    virtual bool                            extractDxGeometry(const char* pFileName);
    virtual bool                            extractFaceDefs  (const char* pFileName);
    virtual bool                            generateMoppCode (const char* pFileName);
    virtual bhkCompressedMeshShapeDataRef   getShapeData     () { return _pShapeData; }
    virtual bhkMoppBvTreeShapeRef           getMoppBvTree    () { return _pMoppBvTree; }
};
