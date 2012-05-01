#include <stdio.h>
#include <stdlib.h>
#include "FDMesh.h"

FDMesh::FDMesh()
	:	_pVertAry   (NULL),
		_pFaceAry   (NULL),
    _pFaceNumAry(NULL),
		_numVert    (0),
		_numFace    (0)
{
	_center._x = 0.0;
	_center._y = 0.0;
	_center._z = 0.0;
}

FDMesh::~FDMesh()
{
	if (_pVertAry != NULL)		delete[] _pVertAry;
	if (_pFaceAry != NULL)		delete[] _pFaceAry;
  if (_pFaceNumAry != NULL) delete[] _pFaceNumAry;
}

bool FDMesh::readMeshFromFile(const char *pFileName)
{
	FILE*	pFile = fopen(pFileName, "r");

	if (pFile == NULL)			return false;

	char	cbuffer[50000];
	char*	pChar  (NULL);
	float	minX   (999999);
	float	minY   (999999);
	float	minZ   (999999);
	float	maxX   (-999999);
	float	maxY   (-999999);
	float	maxZ   (-999999);
	short	idxVert(0);
	short	idxFace(0);

	while (true)
	{
		memset(cbuffer, 0, 50000*sizeof(char));
		pChar = fgets(cbuffer, 50000, pFile);

		if (pChar  == NULL)			break;
		if (*pChar == 0)			continue;
		if (*pChar == 0x1a)			continue;

		if (strncmp(cbuffer, "vertices:", 9) == 0)
		{
			_numVert  = atoi(cbuffer + 10);
			_pVertAry = new D3DCustomVertex[_numVert];
		}
		else if (strncmp(cbuffer, "v-", 2) == 0)
		{
			pChar = strchr(cbuffer, ':');
			sscanf(++pChar, "%f,%f,%f", &(_pVertAry[idxVert]._x), &(_pVertAry[idxVert]._z), &(_pVertAry[idxVert]._y));
			_pVertAry[idxVert]._color = TRI_DEF_COLOR;

			if (_pVertAry[idxVert]._x < minX)		minX = _pVertAry[idxVert]._x;
			if (_pVertAry[idxVert]._x > maxX)		maxX = _pVertAry[idxVert]._x;
			if (_pVertAry[idxVert]._y < minY)		minY = _pVertAry[idxVert]._y;
			if (_pVertAry[idxVert]._y > maxY)		maxY = _pVertAry[idxVert]._y;
			if (_pVertAry[idxVert]._z < minZ)		minZ = _pVertAry[idxVert]._z;
			if (_pVertAry[idxVert]._z > maxZ)		maxZ = _pVertAry[idxVert]._z;

			++idxVert;
		}
		else if (strncmp(cbuffer, "faces:", 6) == 0)
		{
			_numFace     = atoi(cbuffer + 7);
      _pFaceNumAry = new unsigned short[_numFace];
      _numFace    *= 3;
			_pFaceAry    = new unsigned short[_numFace];
		}
		else if (strncmp(cbuffer, "f-", 2) == 0)
		{
			int				      cnt    (0);
			unsigned int	  value  [3];
      unsigned short  faceNum(0);
			DWORD			      color  (0);

      faceNum = atoi(cbuffer + 2);
			pChar   = strchr(cbuffer, ':');
			cnt     = sscanf(++pChar, "%d,%d,%d,%x", &(value[0]), &(value[1]), &(value[2]), &color);
			_pFaceAry   [idxFace  ] = (unsigned short) value[0];
			_pFaceAry   [idxFace+1] = (unsigned short) value[1];
			_pFaceAry   [idxFace+2] = (unsigned short) value[2];
      _pFaceNumAry[idxFace/3] = faceNum;
			if (cnt == 4)
			{
				_pVertAry[_pFaceAry[idxFace  ]]._color = color;
				_pVertAry[_pFaceAry[idxFace+1]]._color = color;
				_pVertAry[_pFaceAry[idxFace+2]]._color = color;
			}
			idxFace += 3;
		}
	}  //  while (true)

	fclose(pFile);

	_center._x = (float) -((maxX - minX) / 2.0);
	_center._y = (float) -((maxY - minY) / 2.0);
	_center._z = (float) -((maxZ - minZ) / 2.0);

	return true;
}

unsigned short FDMesh::getFaceNumByIndex(unsigned short index)
{
  if (index < 0)                  return -1;
  if (index >= (_numFace / 3))    return -1;
  if (_pFaceNumAry == NULL)       return -1;

  return _pFaceNumAry[index];
}
