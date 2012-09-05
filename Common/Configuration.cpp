#include "..\Common\stdafx.h"
#include "Configuration.h"
#include <iostream>
#include <fstream>

using namespace NifUtility;


Configuration::Configuration()
	:	_collMaterial     (3741512247),
		_matHandling      (0),
		_vertColHandling  (0),
		_collTypeHandling (1),
		_upTangentSpace   (true),
		_reorderProperties(true),
		_dxShowTexture    (true),
		_dxShowWireframe  (false)
{
}

Configuration::~Configuration()
{
}

bool Configuration::readAttribute(const string& content, const string tag, string& attribute)
{
	size_t	posStart(0);
	size_t	posEnd  (0);
	bool	isOK    (false);

	posStart = content.find(tag);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = content.substr(posStart, posEnd - posStart - 2);
			isOK = true;
		}
	}

	return isOK;
}

bool Configuration::readAttribute(const string& content, const string tag, int& attribute)
{
	size_t	posStart(0);
	size_t	posEnd  (0);
	bool	isOK    (false);

	posStart = content.find(tag);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = atoi(content.substr(posStart, posEnd - posStart - 2).c_str());
			isOK = true;
		}
	}

	return isOK;
}

bool Configuration::readAttribute(const string& content, const string tag, bool& attribute)
{
	size_t	posStart(0);
	size_t	posEnd  (0);
	bool	isOK    (false);

	posStart = content.find(tag);
	if (posStart != string::npos)
	{
		posStart += tag.length();
		posEnd = content.find(tag, posStart);
		if (posEnd != string::npos)
		{
			attribute = (atoi(content.substr(posStart, posEnd - posStart - 2).c_str()) == 1);
			isOK = true;
		}
	}

	return isOK;
}

bool Configuration::read(const string fileName)
{
	ifstream	iStr    (fileName.c_str());
	string		content;
	string		search;
	bool		isOK    (false);

	//  file opened successfully
	if (iStr.is_open())
	{
		while (iStr.good())
		{
			//iStr >> content;
			getline(iStr, content);

			//  fetch attributes
			readAttribute(content, "PathSkyrim>", _pathSkyrim);
			readAttribute(content, "PathNifXML>", _pathNifXML);
			readAttribute(content, "PathTemplate>", _pathTemplate);
			readAttribute(content, "MatHandling>", _matHandling);
			readAttribute(content, "VertexColorHandling>", _vertColHandling);
			readAttribute(content, "UpdateTangentSpace>", _upTangentSpace);
			readAttribute(content, "ReorderProperties>", _reorderProperties);
			readAttribute(content, "CollTypeHandling>", _collTypeHandling);
			readAttribute(content, "CollMaterial>", _collMaterial);
			readAttribute(content, "LastTexture>", _lastTexture);
			readAttribute(content, "LastTemplate>", _lastTemplate);
			readAttribute(content, "DirSource>", _dirSource);
			readAttribute(content, "DirDestination>", _dirDestination);
			readAttribute(content, "DirCollision>", _dirCollision);

			readAttribute(content, "ShowTexture>", _dxShowTexture);
			readAttribute(content, "ShowWireframe>", _dxShowWireframe);



		}  //  while (iStr.good())

		//  close file
		iStr.close();

		isOK = true;

	}  //  if (oStr.is_open())

	//  remember file name
	_configName = fileName;

	return isOK;
}

bool Configuration::write()
{
	if (_configName.empty())	return false;

	return write(_configName);
}

bool Configuration::write(const string fileName)
{
	ofstream	oStr(fileName.c_str(), ios::out | ios::trunc);
	bool		isOK(false);

	//  file opened successfully
	if (oStr.is_open())
	{
		oStr << "<Config>";
		oStr << "<PathSkyrim>" << _pathSkyrim << "</PathSkyrim>";
		oStr << "<PathNifXML>" << _pathNifXML << "</PathNifXML>";
		oStr << "<PathTemplate>" << _pathTemplate << "</PathTemplate>";
		oStr << "<LastTexture>" << _lastTexture << "</LastTexture>";
		oStr << "<LastTemplate>" << _lastTemplate << "</LastTemplate>";
		oStr << "<DirSource>" << _dirSource << "</DirSource>";
		oStr << "<DirDestination>" << _dirDestination << "</DirDestination>";
		oStr << "<DirCollision>" << _dirCollision << "</DirCollision>";
		oStr << "<MatHandling>" << _matHandling << "</MatHandling>";
		oStr << "<VertexColorHandling>" << _vertColHandling << "</VertexColorHandling>";
		oStr << "<UpdateTangentSpace>" << (_upTangentSpace ? 1 : 0) << "</UpdateTangentSpace>";
		oStr << "<ReorderProperties>" << (_reorderProperties ? 1 : 0) << "</ReorderProperties>";
		oStr << "<CollTypeHandling>" << _collTypeHandling << "</CollTypeHandling>";
		oStr << "<CollMaterial>" << _collMaterial << "</CollMaterial>";

		oStr << "<DirectXView>";
		oStr << "<ShowTexture>" << _dxShowTexture << "</ShowTexture>";
		oStr << "<ShowWireframe>" << _dxShowWireframe << "</ShowWireframe>";
		oStr << "</DirectXView>";


		oStr << "</Config>";

		//  close file
		oStr.close();

		isOK = true;

	}  //  if (oStr.is_open())

	return isOK;
}

