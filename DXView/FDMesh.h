#include <windows.h>

#define	TRI_DEF_COLOR	0x606060
#define TRI_HIGH_COLOR	0xffff00


struct D3DCustomVertex
{
  float   _x;
  float   _y;
  float   _z;
  DWORD   _color;
};

class FDMesh
{
	private:
		D3DCustomVertex*	_pVertAry;
		unsigned short*		_pFaceAry;
    unsigned short*   _pFaceNumAry;
		D3DCustomVertex		_center;
		short				      _numVert;
		short				      _numFace;

	public:
									            FDMesh();
		virtual						        ~FDMesh();
		virtual	bool				      readMeshFromFile(const char* pFileName);
		virtual D3DCustomVertex*	getVertices()		{ return _pVertAry; }
		virtual unsigned short*		getIndices()		{ return _pFaceAry; }
		virtual	short				      getNumVertices()	{ return _numVert; }
		virtual	short				      getNumIndices()		{ return _numFace; }
		virtual	D3DCustomVertex		getCenter()			{ return _center; }
    virtual unsigned short    getFaceNumByIndex(unsigned short index);
};
