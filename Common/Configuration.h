#pragma once

#include <string>

using namespace std;

namespace NifUtility
{
	class Configuration
	{
		public:
			string			_configName;
			string			_pathSkyrim;
			string			_pathTemplate;
			string			_pathNifXML;
			string			_lastTexture;
			string			_lastTemplate;
			string			_dirSource;
			string			_dirDestination;
			string			_dirCollision;
			string			_dirTexturePath;
			DWORD			_colorWireframe;
			DWORD			_colorWireCollision;
			DWORD			_colorBackground;
			int				_collMaterial;
			int				_matHandling;
			int				_vertColHandling;
			int				_collTypeHandling;
			bool			_upTangentSpace;
			bool			_reorderProperties;
			bool			_dxShowTexture;
			bool			_dxShowWireframe;
			bool			_dxShowColorWire;
			bool			_dxForceDDS;

		private:
			virtual	bool	readAttribute(const string& content, const string tag, string& attribute);
			virtual	bool	readAttribute(const string& content, const string tag, DWORD& attribute);
			virtual	bool	readAttribute(const string& content, const string tag, int& attribute);
			virtual	bool	readAttribute(const string& content, const string tag, bool& attribute);

		public:
							Configuration();
			virtual			~Configuration();

			virtual	bool	read (const string fileName);
			virtual	bool	write();
			virtual	bool	write(const string fileName);
	};
}
