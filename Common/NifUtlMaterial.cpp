#include "NifUtlMaterial.h"

#include <iostream>
#include <fstream>

//  used namespaces
using namespace NifUtility;


/*---------------------------------------------------------------------------*/
NifUtlMaterialList::NifUtlMaterialList()
{
}

/*---------------------------------------------------------------------------*/
NifUtlMaterialList::~NifUtlMaterialList()
{
}

/*---------------------------------------------------------------------------*/
map<string, NifUtlMaterial>& NifUtlMaterialList::getMaterialMap()
{
	return _materialMap;
}

/*---------------------------------------------------------------------------*/
unsigned int NifUtlMaterialList::getMaterialCode(string matDefName)
{
	return (_materialMap.count(matDefName) > 0) ? _materialMap[matDefName]._code : 0;
}

/*---------------------------------------------------------------------------*/
void NifUtlMaterialList::initializeMaterialMap(string pathToXML)
{
	ifstream	streamIn;
	char		cbuffer[10000] = {0};
	bool		isMaterialSection(false);

	//  open nif.xml
	streamIn.open(pathToXML.c_str(), ifstream::in);

	sprintf(cbuffer, "^%cOpening '%s': %s", (streamIn.good() ? '0' : '2'), pathToXML.c_str(), (streamIn.good() ? "OK" : "FAILED"));
	_userMessages.push_back(cbuffer);

	//  on valid input
	while (streamIn.good())
	{
		//  read next row
		streamIn.getline(cbuffer, 10000);

		//  search start of material definition
		if (strstr(cbuffer, "<enum name=\"HavokMaterial\" storage=\"uint\">") != NULL)
		{
			isMaterialSection = true;
		}

		//  row within material definition section
		if (isMaterialSection)
		{
			//  valid material definition for SKYrim?
			if ((strstr(cbuffer, "<option value=\"") != NULL) &&
				(strstr(cbuffer, "name=\"SKY_HAV_") != NULL))
			{
				NifUtlMaterial	matNew;
				char*			pStart(strstr(cbuffer, "value="));
				char*			pEnd  (strstr(cbuffer, "\" name="));

				//  read material code
				matNew._code = atol(pStart + 7);

				//  parse and read definition name
				pStart = pEnd + 8;
				pEnd   = strstr(pStart, "\">");
				*pEnd  = 0;
				matNew._defName = pStart;

				//  parse and read user readable name
				pStart = pEnd + 2;
				pEnd   = strstr(pStart, "</option>");
				*pEnd  = 0;
				if (strncmp(pStart, "Material", 8) == 0)		pStart += 8;	//  skip leading 'Material'
				if (pStart[0] == ' ')							pStart += 1;	//  skip leading space

				matNew._name = pStart;

				//  append material to map
				_materialMap[matNew._defName] = matNew;

				_userMessages.push_back("^4added: " + matNew._defName + " => '" + matNew._name + "'");
			}
			//  early break at end of material definition
			else if (strstr(cbuffer, "</enum>") != NULL)
			{
				break;
			}
		}  //  if (isMaterialSection)
	}  //  while (streamIn.good())

	sprintf(cbuffer, "^%cmaterials found: %d", ((_materialMap.size() > 0) ? '0' : '1'), _materialMap.size());
	_userMessages.push_back(cbuffer);

	streamIn.close();
}

/*---------------------------------------------------------------------------*/
vector<string>& NifUtlMaterialList::getUserMessages()
{
	return _userMessages;
}

