#include "NifFile.h"
#include "niflib.h"
#include "obj/niobject.h"
#include "obj/bsfadenode.h"
#include "obj/bhkcollisionobject.h"
#include "obj/bhkrigidbodyt.h"
#include "obj/bhkrigidbody.h"
#include "obj/bhkCompressedMeshShape.h"

#define INDENT              2
#define MAX_BUFFER_SIZE 10000
#define TMP_BUFFER_SIZE  2000

//  array of named parameters
const char*	glParaAry[] = { "x", "y", "z", "(y + z)", "(y - z)", "(x + z)", "(x - z)", "(x + y)", "(x - y)", "(x + y + z)", "(x + y - z)", "(x -y + z)", "( x - y - z)" };

// ctor
NifFile::NifFile()
  : _pShapeData (NULL),
    _pMoppBvTree(NULL),
    _pNifRoot   (NULL),
    _pCharBuffer(NULL)
{
}

//  dtor
NifFile::~NifFile()
{
  if (_pNifRoot != NULL)
  {
    closeNif();
  }
  if (_pCharBuffer != NULL)
  {
    delete[] _pCharBuffer;
  }
}

//  close open nif file (nothing to do in case of ReadNifTree())
bool NifFile::closeNif()
{
  _pNifRoot = NULL;
  return true;
}

//  read nif file using ReadNifTree(), extract bhkMoppBcTreeShape and bhkCompressedMeshShapeData
bool NifFile::openNif(const char *pFileName)
{
  //  read from nif file
  _pNifRoot = ReadNifTree(pFileName);

  //  BSFadeNode?
  BSFadeNodeRef pFadeNode = DynamicCast<BSFadeNode>(_pNifRoot);

  //  no FadeNode
  if (pFadeNode == NULL)    return false;

  //  Collision Object?
  NiObjectRef           pTObj   = pFadeNode->GetCollisionObject();
  bhkCollisionObjectRef pColObj = DynamicCast<bhkCollisionObject>(pTObj);

  //  no collision node
  if (pColObj == NULL)      return false;

  //  Body of Collision Object?
  pTObj                      = pColObj->GetBody();
  bhkRigidBodyTRef  pRigBody = DynamicCast<bhkRigidBodyT>(pTObj);

  if (pRigBody == NULL)
  {
    bhkRigidBodyRef  pRig1Body = DynamicCast<bhkRigidBody>(pTObj);
    if (pRig1Body == NULL)  return false;

    pTObj = pRig1Body->GetShape();
  }
  else
  {
    pTObj = pRigBody->GetShape();
  }

  //  Shape?
  _pMoppBvTree = DynamicCast<bhkMoppBvTreeShape>(pTObj);

  if (_pMoppBvTree == NULL)   return false;

  pTObj = _pMoppBvTree->GetShape();
  bhkCompressedMeshShapeRef pMeshShape = DynamicCast<bhkCompressedMeshShape>(pTObj);

  if (pMeshShape == NULL)   return false;

  //  ShapeData?
  std::list<NiObjectRef>  refList = pMeshShape->GetRefs();

  for (std::list<NiObjectRef>::iterator listIter = refList.begin(); listIter != refList.end(); listIter++)
  {
    _pShapeData = DynamicCast<bhkCompressedMeshShapeData>(*listIter);
    break;
  }

  return (_pShapeData != NULL);
}

//  write data stored in chunk to file in readable form
bool NifFile::extractChunkData(const char *pFileName)
{
  FILE*                       pFile(NULL);
  std::vector<bhkCMSDChunk>   chunkListNif = _pShapeData->GetChunks();

  if ((pFileName == NULL) || (*pFileName == 0))
  {
    return false;
  }

  fopen_s(&pFile, pFileName, "w");
  fprintf(pFile, "chunks: %d\n", chunkListNif.size());

  for (unsigned short chunkIdx(0); chunkIdx < chunkListNif.size(); ++chunkIdx)
  {
    fprintf(pFile, "\n\nchunk: %d\n", chunkIdx);

    bhkCMSDChunk&  chunkNif = chunkListNif[chunkIdx];

    fprintf(pFile, "\n\nvertices: %d\n", chunkNif.vertices.size());
    for (unsigned int i(0); i < chunkNif.vertices.size(); ++i)
    {
      fprintf(pFile, "%u,", chunkNif.vertices[i]);
    }

    fprintf(pFile, "\n\nindices: %d\n", chunkNif.indices.size());
    for (unsigned int i(0); i < chunkNif.indices.size(); ++i)
    {
      fprintf(pFile, "%u,", chunkNif.indices[i]);
    }

    fprintf(pFile, "\n\nstrips: %d\n", chunkNif.strips.size());
    for (unsigned int i(0); i < chunkNif.strips.size(); ++i)
    {
      fprintf(pFile, "%u,", chunkNif.strips[i]);
    }

    fprintf(pFile, "\n\nindices2: %d\n", chunkNif.indices2.size());
    for (unsigned int i(0); i < chunkNif.indices2.size(); ++i)
    {
      fprintf(pFile, "%u,", chunkNif.indices2[i]);
    }
  }  //  for (short chunkIdx(0); chunkIdx < chunkListNif.size(); ++chunkIdx)

  fflush(pFile);
  fclose(pFile);

  return true;
}

//  write mopp code as bytes to file
bool NifFile::extractMoppData(const char *pFileName)
{
  FILE*                       pFile   (NULL);
  std::vector<Niflib::byte>   tByteVec(_pMoppBvTree->GetMoppCode());

  if ((pFileName == NULL) || (*pFileName == 0))
  {
    return false;
  }

  fopen_s(&pFile, pFileName, "w");

  for (unsigned int i(1); i < tByteVec.size(); ++i)
  {
    fprintf(pFile, "%u,", tByteVec[i]);
  }
  fprintf(pFile, "%u,", tByteVec[0]);

  fflush(pFile);
  fclose(pFile);

  return true;
}

//  build triangle shapes from chunk data
int NifFile::buildTriangles(FDTriangle** ppActTri, Vector3** ppVertices, bhkCMSDChunk& chunkNif)
{
  Vector3*		pVertices(NULL);
	int         offIndex (0);

  //  build vertices
  pVertices = new Vector3[chunkNif.numVertices / 3];

  for (unsigned int i(0); i < chunkNif.numVertices; i+=3)
  {
    pVertices[i/3].x = chunkNif.vertices[i];
    pVertices[i/3].y = chunkNif.vertices[i+1];
    pVertices[i/3].z = chunkNif.vertices[i+2];
  }

  *ppVertices = pVertices;

  //  faces defined by strip groups
  for (unsigned int idxStrip(0); idxStrip < chunkNif.numStrips; ++idxStrip)
  {
    unsigned int    stripCnt(chunkNif.strips[idxStrip]);

    for (unsigned int idxInd(offIndex); idxInd < (offIndex + stripCnt - 2); ++idxInd)
    {
      *ppActTri = new FDTriangle();

      (*ppActTri)->id      = idxInd;
      (*ppActTri)->strip   = idxStrip;
      (*ppActTri)->pP1     = &(pVertices[chunkNif.indices[idxInd]]);
      (*ppActTri)->pP2     = &(pVertices[chunkNif.indices[idxInd + 1]]);
      (*ppActTri)->pP3     = &(pVertices[chunkNif.indices[idxInd + 2]]);
      (*ppActTri)->idx1    = chunkNif.indices[idxInd];
      (*ppActTri)->idx2    = chunkNif.indices[idxInd + 1];
      (*ppActTri)->idx3    = chunkNif.indices[idxInd + 2];
      (*ppActTri)->welding = chunkNif.indices2[idxInd];

      //  set next ptr.
      ppActTri = &(*ppActTri)->pNext;

    }  //  for (unsigned int idxInd(offIndex); idxInd < (offIndex + stripCnt - 2); ++idxInd)

    offIndex += stripCnt;

  }  //  for (unsigned int idxStrip(0); idxStrip < chunkNif.numStrips; ++idxStrip)

  //  faces defined by indices only
  for (unsigned int idxInd(offIndex); idxInd < chunkNif.numIndices; idxInd += 3, offIndex += 3)
  {
    *ppActTri = new FDTriangle();

    (*ppActTri)->id      = idxInd;
    (*ppActTri)->pP1     = &(pVertices[chunkNif.indices[idxInd]]);
    (*ppActTri)->pP2     = &(pVertices[chunkNif.indices[idxInd + 1]]);
    (*ppActTri)->pP3     = &(pVertices[chunkNif.indices[idxInd + 2]]);
    (*ppActTri)->idx1    = chunkNif.indices[idxInd];
    (*ppActTri)->idx2    = chunkNif.indices[idxInd + 1];
    (*ppActTri)->idx3    = chunkNif.indices[idxInd + 2];
    (*ppActTri)->welding = chunkNif.indices2[idxInd];

    //  set next ptr.
    ppActTri = &(*ppActTri)->pNext;

  }  //  for (unsigned int idxInd(offIndex); idxInd < chunkNif.numIndices; idxInd += 3, offIndex += 3)

  return offIndex;
}

//  write vertices and shape definitions to file readable for DxView utility
bool NifFile::extractDxGeometry(const char *pFileName)
{
  FILE*                       pFile       (NULL);
  Vector3*		                pVertices   (NULL);
  FDTriangle*                 pTriangle   (NULL);
  std::vector<bhkCMSDChunk>   chunkListNif(_pShapeData->GetChunks());
	int			                    offIndex    (0);

  if ((pFileName == NULL) || (*pFileName == 0))
  {
    return false;
  }

  fopen_s(&pFile, pFileName, "w");

  for (unsigned short chunkIdx(0); chunkIdx < chunkListNif.size(); ++chunkIdx)
  {
    if (chunkIdx > 0)
    {
      fprintf(pFile, "\n\n");
    }
    fprintf(pFile, "chunk: %d", chunkIdx);

    bhkCMSDChunk&  chunkNif = chunkListNif[chunkIdx];

    offIndex = buildTriangles(&pTriangle, &pVertices, chunkListNif[chunkIdx]);

    fprintf(pFile, "\n\nvertices: %d\n", (chunkNif.numVertices / 3));
    for (unsigned int i(0); i < (chunkNif.numVertices / 3); ++i)
    {
	    fprintf(pFile, "v-%d:%d,%d,%d\n", i, pVertices[i].x, pVertices[i].y, pVertices[i].z);
    }

    fprintf(pFile, "\n\nfaces: %d\n", offIndex);
    for (FDTriangle* pTri(pTriangle); pTri != NULL; pTri = pTri->pNext)
    {
	    fprintf(pFile, "f-%d:%d,%d,%d\n", pTri->id, pTri->idx1, pTri->idx2, pTri->idx3);
    }

    if (pTriangle != NULL)    delete   pTriangle;
    if (pVertices != NULL)    delete[] pVertices;

  }  //  for (short chunkIdx(0); chunkIdx < chunkListNif.size(); ++chunkIdx)

  fflush(pFile);
  fclose(pFile);

  return true;
}

//  write decoded mopp code to file in readable form
bool NifFile::generateMoppCode(const char *pFileName)
{
  FILE*               pFile   (NULL);
  unsigned char*      pMBuffer(NULL);
  std::vector<byte>   moppCode;
  unsigned int        mIdx    (0);

  if ((pFileName == NULL) || (*pFileName == 0))
  {
    return false;
  }

  _pCharBuffer = new char[MAX_BUFFER_SIZE];
  memset(_pCharBuffer, 0, MAX_BUFFER_SIZE * sizeof(char));

  moppCode = _pMoppBvTree->GetMoppCode();
  pMBuffer = new unsigned char[moppCode.size()];
  memset(pMBuffer, 0, moppCode.size() * sizeof(unsigned char));

  //  rotate mopp code by 1 byte offset
  for (;mIdx < (moppCode.size() - 1); ++mIdx)
  {
    pMBuffer[mIdx] = moppCode[mIdx + 1];
  }
  pMBuffer[moppCode.size() - 1] = moppCode[0];

  fopen_s(&pFile, pFileName, "w");

  //  write mopp code as readable code with indents
  printMoppCodeByIndent(pFile, pMBuffer, moppCode.size());

  fflush(pFile);
  fclose(pFile);

  if (pMBuffer != NULL)
  {
    delete[] pMBuffer;
  }
  if (_pCharBuffer != NULL)
  {
    delete[] _pCharBuffer;
    _pCharBuffer = NULL;
  }

  return true;
}

//  format indents fro writing mopp code
char* NifFile::formatIndent(const char* pRBuffer, const char* pCBuffer, const char* pABuffer, const short depth)
{
	char*	pStart(_pCharBuffer);
  int   indent(depth * INDENT);

	memset(_pCharBuffer, 0, MAX_BUFFER_SIZE);

	//  print row and code
	sprintf(pStart, "%s", pRBuffer);
	pStart += strlen(pStart);

	//  fill with blanks
	for (int t(42 - strlen(pRBuffer)); t > 0; t--)
	{
		*pStart++ = ' ';
	}
  *pStart++ = ((pRBuffer[0] != 0) ? ':' : ' ');
  *pStart++ = ' ';
  for (int t(0); t < indent; ++t)
  {
    *pStart++ = ' ';
  }

  //  skip pRBuffer for following rows
  indent += 44;

	//  print description
  for (const char* pChar(pCBuffer); *pChar != 0; ++pChar)
  {
    if (*pChar == '{')
    {
      *pStart++ = '\n';
      for (int t(0); t < indent; ++t)
      {
        *pStart++ = ' ';
      }
      *pStart++ = *pChar;
      *pStart++ = '\n';
    }
    else if (*pChar == '}')
    {
      *pStart++ = *pChar;
      *pStart++ = '\n';
      if (pChar[1] != 0)
      {
        for (int t(0); t < indent; ++t)
        {
          *pStart++ = ' ';
        }
      }
    }
    else
    {
      *pStart++ = *pChar;
    }
  }

  //  increase indent
  if (pABuffer[0] != 0)
  {
    indent += INDENT;

    if (pABuffer[0] != '}')
    {
      for (int t(0); t < indent; ++t)
      {
        *pStart++ = ' ';
      }
    }

	  //  print add-text if filled
    for (const char* pChar(pABuffer); *pChar != 0; ++pChar)
    {
      if ((*pChar == '}') && (pChar != pABuffer))
      {
        *pStart++ = '\n';
        indent -= INDENT;
        for (int t(0); t < indent; ++t)
        {
          *pStart++ = ' ';
        }
      }
      *pStart++ = *pChar;
    }
  }  //  if (pABuffer[0] != 0)

	//  new line
  if (pStart[-1] != '\n')
  {
	  sprintf(pStart, "\n\0");
  }

  return _pCharBuffer;
}

char* NifFile::formatOutput(const char* pRBuffer, const char* pCBuffer, const char* pABuffer)
{
	char*	pStart(_pCharBuffer);

	memset(_pCharBuffer, 0, MAX_BUFFER_SIZE);

	//  print row and code
	sprintf(pStart, "%s", pRBuffer);
	pStart += strlen(pStart);

	//  fill with blanks
	for (int t(40 - strlen(pRBuffer)); t > 0; t--, ++pStart)
	{
		*pStart = ' ';
	}

	//  print description
	if (pCBuffer[0] != 0)
	{
		sprintf(pStart, ": %s", pCBuffer);
		pStart += strlen(pStart);
	}

	//  print add-text if filled
	if (pABuffer[0] != 0)
	{
		//  fill with blanks
		for (int t(40 - strlen(pCBuffer)); t > 0; t--, ++pStart)
		{
			*pStart = ' ';
		}

		//  print add-on
		sprintf(pStart, "%s", pABuffer);
		pStart += strlen(pStart);

	}  //  if (abuffer[0] != 0)

	//  new line
	sprintf(pStart, "\n\0");

	return _pCharBuffer;
}

//  decode mopp code using indents
void NifFile::printMoppCodeByIndent(FILE* pFileOut, const unsigned char* bbuffer, const int moppSize, int idxByte, const short depth, int triOff)
{
	char			rbuffer[TMP_BUFFER_SIZE];
	char			cbuffer[TMP_BUFFER_SIZE];
	char			abuffer[TMP_BUFFER_SIZE];
	unsigned char	opcode (0);
  bool			doRet  (false);

	while (idxByte < moppSize)
	{
		opcode     = bbuffer[idxByte];
		cbuffer[0] = 0;
		rbuffer[0] = 0;
		abuffer[0] = 0;

		switch (opcode)
		{
			case 0x01:	//  ???
			case 0x02:	//  ???
			case 0x03:	//  ???
			case 0x04:	//  ???
			{
				const char*	pVar = glParaAry[opcode - 0x01];

        sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3]);
        sprintf(cbuffer, "unknown cmd 0x%02x  -  (%s) dec: %3d %3d %3d  off: %04d %04d %04d", opcode, pVar, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], (idxByte + 4 + bbuffer[idxByte + 1]), (idxByte + 4 + bbuffer[idxByte + 2]), (idxByte + 4 + bbuffer[idxByte + 3]));
				idxByte += 4;
				break;
			}

			case 0x05:	//  byte jump
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1]);
				sprintf(cbuffer, "jump -> %04d", (idxByte + 2 + bbuffer[idxByte + 1]));
				idxByte += 2;
				break;
			}

			case 0x06:	//  short jump
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "jump -> %04d", (idxByte + 3 + (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2])));
				idxByte += 3;
				break;
			}

			case 0x09:	//  byte increment triangle offset
			{
				triOff += bbuffer[idxByte + 1];
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1]);
				sprintf(cbuffer, "triOffset += %d; triOffset = %d", bbuffer[idxByte + 1], triOff);
				idxByte += 2;
				break;
			}

			case 0x0A:	//  short increment triangle offset
			{
				triOff += (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2]);
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "triOffset += %d; triOffset = %d", (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2]), triOff);
				idxByte += 3;
				break;
			}

			case 0x0B:	//  short set triangle offset
			{
				triOff = (bbuffer[idxByte + 3] * 256 + bbuffer[idxByte + 4]);
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4]);
				sprintf(cbuffer, "set triOffset = %d", triOff);
				idxByte += 5;
				break;
			}

			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1A:
			case 0x1B:
			case 0x1C:
			{
				const char*	pVar = glParaAry[opcode - 0x10];
				char*		    pBuf = new char[MAX_BUFFER_SIZE];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3]);
        sprintf(cbuffer, "if (%s <= %d){", pVar, bbuffer[idxByte + 1]);
        if (pFileOut != NULL)
        {
  				fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        else
        {
  				printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        printMoppCodeByIndent(pFileOut, bbuffer, moppSize, (idxByte + 4), (depth + 1), triOff);
				rbuffer[0] = 0;
        sprintf(cbuffer, "}if (%s >= %d){", pVar, bbuffer[idxByte + 2]);
        if (pFileOut != NULL)
        {
  				fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        else
        {
  				printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        printMoppCodeByIndent(pFileOut, bbuffer, moppSize, (idxByte + 4 + bbuffer[idxByte + 3]), (depth + 1), triOff);
				rbuffer[0] = 0;
				cbuffer[0] = 0;
        sprintf(abuffer, "}");
				delete[] pBuf;
				idxByte += 4;
        doRet = true;
				break;
			}

			case 0x20:	//  byte if-then-else
			case 0x21:
			case 0x22:
			{
				const char*	pVar = glParaAry[opcode - 0x23];
				char*		    pBuf = new char[MAX_BUFFER_SIZE];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
        sprintf(cbuffer, "if (%s ? %d){", pVar, bbuffer[idxByte + 1]);
        if (pFileOut != NULL)
        {
  				fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        else
        {
  				printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        printMoppCodeByIndent(pFileOut, bbuffer, moppSize, (idxByte + 3), (depth + 1), triOff);
				rbuffer[0] = 0;
        sprintf(cbuffer, "}else{");
        if (pFileOut != NULL)
        {
  				fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        else
        {
  				printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        printMoppCodeByIndent(pFileOut, bbuffer, moppSize, (idxByte + 3 + bbuffer[idxByte + 2]), (depth + 1), triOff);
				rbuffer[0] = 0;
				cbuffer[0] = 0;
        sprintf(abuffer, "}");
				delete[] pBuf;
				idxByte += 3;
        doRet = true;
				break;
			}

			case 0x23:	//  short if-then-else
			case 0x24:
			case 0x25:
			{
				const char*	pVar = glParaAry[opcode - 0x23];
				char*		    pBuf = new char[MAX_BUFFER_SIZE];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4], bbuffer[idxByte + 5], bbuffer[idxByte + 6]);
        sprintf(cbuffer, "if (%s ? %d){", pVar, bbuffer[idxByte + 1]);
        if (pFileOut != NULL)
        {
  				fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        else
        {
  				printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        printMoppCodeByIndent(pFileOut, bbuffer, moppSize, (idxByte + 7 + bbuffer[idxByte + 3] * 256 + bbuffer[idxByte + 4]), (depth + 1), triOff);
				rbuffer[0] = 0;
        sprintf(cbuffer, "}else if (%s ? %d){", pVar, bbuffer[idxByte + 2]);
        if (pFileOut != NULL)
        {
  				fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        else
        {
  				printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
        }
        printMoppCodeByIndent(pFileOut, bbuffer, moppSize, (idxByte + 7 + bbuffer[idxByte + 5] * 256 + bbuffer[idxByte + 6]), (depth + 1), triOff);
				rbuffer[0] = 0;
				cbuffer[0] = 0;
        sprintf(abuffer, "}");
				delete[] pBuf;
				idxByte += 7;
        doRet = true;
				break;
			}

      case 0x26:	//  bounds
			case 0x27:
			case 0x28:
			{
				const char*	pVar = glParaAry[opcode - 0x26];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
        sprintf(cbuffer, "if ((%s < %d) || (%s > %d)){", pVar, bbuffer[idxByte + 1], pVar, bbuffer[idxByte + 2]);
        sprintf(abuffer, "exit;}");
				idxByte += 3;
				break;
			}

			case 0x50:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1]);
				sprintf(cbuffer, "select triangle %d", (triOff + bbuffer[idxByte + 1]));
				idxByte += 2;
        doRet = true;
				break;
			}

			case 0x51:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "select triangle %d", (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2] + triOff));
				idxByte += 3;
        doRet = true;
				break;
			}

			case 0x52:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3]);
				sprintf(cbuffer, "select triangle %d", (bbuffer[idxByte + 2] * 256 + bbuffer[idxByte + 3]));
				idxByte += 4;
        doRet = true;
				break;
			}

			case 0x53:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4]);
				sprintf(cbuffer, "select triangle %d", (bbuffer[idxByte + 3] * 256 + bbuffer[idxByte + 4] + triOff));
				idxByte += 5;
        doRet = true;
				break;
			}

			case 0x70:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4]);
				sprintf(cbuffer, "unknown cmd 0x70");
				idxByte += 5;
				break;
			}

			default:
			{
				//  select triangle?
				if ((opcode >= 0x30) && (opcode <= 0x4F))
				{
					sprintf(rbuffer, "%04d:: 0x%02x", idxByte, opcode);
					sprintf(cbuffer, "select triangle %d", (opcode - 0x30 + triOff));
          doRet = true;
				}
				else
				{
					sprintf(rbuffer, "%04d:: 0x%02x ", idxByte, opcode);
					sprintf(cbuffer, "unknown cmd");
				}
				idxByte += 1;
				break;
			}
		}  //  switch (opcode)

    if (pFileOut != NULL)
    {
      fprintf(pFileOut, "%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
    }
    else
    {
      printf("%s", formatIndent(rbuffer, cbuffer, abuffer, depth));
    }

    if (doRet)
    {
      return;
    }

	}  //  while (idxByte < moppSize)
}

void NifFile::printMoppCodeByLine(FILE* pFileOut, const unsigned char* bbuffer, const int moppSize)
{
	char			rbuffer[2000];
	char			cbuffer[2000];
	char			abuffer[2000];
	int 			idxByte(0);
	int				triOff (0);
	unsigned char	opcode (0);

	while (idxByte < moppSize)
	{
		opcode     = bbuffer[idxByte];
		cbuffer[0] = 0;
		rbuffer[0] = 0;
		abuffer[0] = 0;

		switch (opcode)
		{
			case 0x05:	//  byte jump
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1]);
				sprintf(cbuffer, "jump -> %04d", (idxByte + 2 + bbuffer[idxByte + 1]));
				idxByte += 2;
				break;
			}

			case 0x06:	//  short jump
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "jump -> %04d", (idxByte + 3 + (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2])));
				idxByte += 3;
				break;
			}

			case 0x09:	//  byte increment triangle offset
			{
				triOff += bbuffer[idxByte + 1];
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1]);
				sprintf(cbuffer, "triOffset += %d;", bbuffer[idxByte + 1]);
				sprintf(abuffer, "triOffset = %d", triOff);
				idxByte += 2;
				break;
			}

			case 0x0A:	//  short increment triangle offset
			{
				triOff += (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2]);
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "triOffset += %d;", (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2]));
				sprintf(abuffer, "triOffset = %d", triOff);
				idxByte += 3;
				break;
			}

			case 0x0B:	//  short set triangle offset
			{
				triOff = (bbuffer[idxByte + 3] * 256 + bbuffer[idxByte + 4]);
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4]);
				sprintf(cbuffer, "set triOffset = %d", triOff);
				idxByte += 5;
				break;
			}

			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1A:
			case 0x1B:
			case 0x1C:
			{
				const char*	pVar = glParaAry[opcode - 0x10];
				char*		pBuf = new char[10000];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3]);
				sprintf(cbuffer, "if (%s < %d)", pVar, bbuffer[idxByte + 2]);
				sprintf(abuffer, "goto %04d", (idxByte + 4));
				sprintf(pBuf, "%s", formatOutput(rbuffer, cbuffer, abuffer));
				rbuffer[0] = 0;
				sprintf(cbuffer, "else if (%s > %d)", pVar, bbuffer[idxByte + 1]);
				sprintf(abuffer, "goto %04d", (idxByte + 4 + bbuffer[idxByte + 3]));
				sprintf((pBuf + strlen(pBuf)), "%s", formatOutput(rbuffer, cbuffer, abuffer));
				sprintf(cbuffer, "else if (%d <= %s <= %d)", bbuffer[idxByte + 2], pVar, bbuffer[idxByte + 1]);
				sprintf(abuffer, "goto %04d, then goto %04d", (idxByte + 4), (idxByte + 4 + bbuffer[idxByte + 3]));
				sprintf((pBuf + strlen(pBuf)), "%s", formatOutput(rbuffer, cbuffer, abuffer));
				pBuf[strlen(pBuf) - 1] = 0;
				sprintf(rbuffer, "%s", pBuf);
				cbuffer[0] = 0;
				abuffer[0] = 0;
				delete[] pBuf;
				idxByte += 4;
				break;
			}

			case 0x20:	//  byte if-then-else
			case 0x21:
			case 0x22:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "if (? -> %d)", bbuffer[idxByte + 1]);
				sprintf(abuffer, "goto %04d and/or %04d", (idxByte + 3), (idxByte + 3 + bbuffer[idxByte + 2]));
				idxByte += 3;
				break;
			}

			case 0x23:	//  short if-then-else
			case 0x24:
			case 0x25:
			{
				const char*	pVar = glParaAry[opcode - 0x23];
				char*		pBuf = new char[10000];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4], bbuffer[idxByte + 5], bbuffer[idxByte + 6]);
				sprintf(cbuffer, "if (%s <= %d)", pVar, bbuffer[idxByte + 1]);
				sprintf(abuffer, "goto %04d", (idxByte + 7 + bbuffer[idxByte + 3] * 256 + bbuffer[idxByte + 4]));
				sprintf(pBuf, "%s", formatOutput(rbuffer, cbuffer, abuffer));
				rbuffer[0] = 0;
				sprintf(cbuffer, "if (%s > %d)", pVar, bbuffer[idxByte + 2]);
				sprintf(abuffer, "goto %04d", (idxByte + 7 + bbuffer[idxByte + 5] * 256 + bbuffer[idxByte + 6]));
				sprintf((pBuf + strlen(pBuf)), "%s", formatOutput(rbuffer, cbuffer, abuffer));
				pBuf[strlen(pBuf) - 1] = 0;
				sprintf(rbuffer, "%s", pBuf);
				delete[] pBuf;
				idxByte += 7;
				break;
			}

			case 0x26:	//  bounds
			case 0x27:
			case 0x28:
			{
				const char*	pVar = glParaAry[opcode - 0x26];

				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "if ((%s < %d) || (%s > %d))", pVar, bbuffer[idxByte + 1], pVar, bbuffer[idxByte + 2]);
				sprintf(abuffer, "exit");
				idxByte += 3;
				break;
			}

			case 0x50:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1]);
				sprintf(cbuffer, "select triangle %d", (triOff + bbuffer[idxByte + 1]));
				sprintf(abuffer, "exit");
				idxByte += 2;
				break;
			}

			case 0x51:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2]);
				sprintf(cbuffer, "select triangle %d", (bbuffer[idxByte + 1] * 256 + bbuffer[idxByte + 2] + triOff));
				sprintf(abuffer, "exit");
				idxByte += 3;
				break;
			}

			case 0x52:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3]);
				sprintf(cbuffer, "select triangle %d", (bbuffer[idxByte + 2] * 256 + bbuffer[idxByte + 3]));
				sprintf(abuffer, "exit");
				idxByte += 4;
				break;
			}

			case 0x53:
			{
				sprintf(rbuffer, "%04d:: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", idxByte, opcode, bbuffer[idxByte + 1], bbuffer[idxByte + 2], bbuffer[idxByte + 3], bbuffer[idxByte + 4]);
				sprintf(cbuffer, "select triangle %d", (bbuffer[idxByte + 3] * 256 + bbuffer[idxByte + 4] + triOff));
				sprintf(abuffer, "exit");
				idxByte += 5;
				break;
			}

			default:
			{
				//  select triangle?
				if ((opcode >= 0x30) && (opcode <= 0x4F))
				{
					sprintf(rbuffer, "%04d:: 0x%02x", idxByte, opcode);
					sprintf(cbuffer, "select triangle %d", (opcode - 0x30 + triOff));
					sprintf(abuffer, "exit");
				}
				else
				{
					sprintf(rbuffer, "%04d:: 0x%02x ", idxByte, opcode);
					sprintf(cbuffer, "unknown cmd");
				}
				idxByte += 1;
				break;
			}
		}  //  switch (opcode)

		//  print formatted text
    if (pFileOut != NULL)
    {
		  fprintf(pFileOut, "%s", formatOutput(rbuffer, cbuffer, abuffer));
    }
    else
    {
		  printf("%s", formatOutput(rbuffer, cbuffer, abuffer));
    }

	}  //  while (idxByte < moppSize)
}

//  write face definitions from chunks in readable form into file
bool NifFile::extractFaceDefs(const char *pFileName)
{
  FILE*                       pFile       (NULL);
  Vector3*		                pVertices   (NULL);
  FDTriangle*                 pTriangle   (NULL);
  std::vector<bhkCMSDChunk>   chunkListNif(_pShapeData->GetChunks());
	int			                    offIndex    (0);

  if ((pFileName == NULL) || (*pFileName == 0))
  {
    return false;
  }

  fopen_s(&pFile, pFileName, "w");

  for (unsigned short chunkIdx(0); chunkIdx < chunkListNif.size(); ++chunkIdx)
  {
    if (chunkIdx > 0)
    {
      fprintf(pFile, "\n\n");
    }
    fprintf(pFile, "chunk: %d", chunkIdx);

    bhkCMSDChunk&  chunkNif = chunkListNif[chunkIdx];

    offIndex = buildTriangles(&pTriangle, &pVertices, chunkListNif[chunkIdx]);

    fprintf(pFile, "\n\nfaces: %d\n", offIndex);
    for (FDTriangle* pTri(pTriangle); pTri != NULL; pTri = pTri->pNext)
    {
	    fprintf(pFile, "%3d:: str:%2d   p1x:%5d, p1y:%5d, p1z:%5d   p2x:%5d, p2y:%5d, p2z:%5d   p3x:%5d, p3y:%5d, p3z:%5d   w: 0x%06X => 0x%02X 0x%02x 0x%02x\n",
	           pTri->id, pTri->strip, pTri->pP1->x, pTri->pP1->y, pTri->pP1->z, pTri->pP2->x, pTri->pP2->y, pTri->pP2->z, pTri->pP3->x, pTri->pP3->y, pTri->pP3->z, pTri->welding, ((pTri->welding & 0x7B00) >> 10), ((pTri->welding &0x03E0) >> 5), (pTri->welding & 0x001f));
    }

    if (pTriangle != NULL)    delete   pTriangle;
    if (pVertices != NULL)    delete[] pVertices;

  }  //  for (short chunkIdx(0); chunkIdx < chunkListNif.size(); ++chunkIdx)

  fflush(pFile);
  fclose(pFile);

  return true;
}