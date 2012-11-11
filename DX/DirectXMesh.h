#pragma once

#include <d3dx9.h>
#include <string>

using namespace std;

namespace NifUtility
{
	class DirectXMesh
	{
		protected:
			struct AlphaState
			{
				DWORD	_source;
				DWORD	_destination;
				DWORD	_argument;
			};

		protected:
			AlphaState*				_pAlpha;		//  parameters used for alpha blending
			D3DXMATRIX				_transform;		//  transformation to global coord system
			D3DMATERIAL9			_material;		//  material of object
			LPDIRECT3DVERTEXBUFFER9	_pVBuffer;		//  VertexBuffer to hold vertices
			LPDIRECT3DINDEXBUFFER9	_pIBuffer;		//  IndexBuffer to hold face indexes
			string					_name;			//  name of NIF node
			string					_type;			//  type of NIF node
			unsigned int			_countVertices;	//  number of vertices
			unsigned int			_countIndices;	//  number of indices
			int						_blockNumber;	//  block number of NIF node
			bool					_doRender;		//  flag enabling rendering
			bool					_isSelected;	//  flag showing selected state

		public:
							DirectXMesh();
			virtual			~DirectXMesh();

			virtual	void	SetDoRender   (const bool doRender);
			virtual void	SetAlpha      (const DWORD source, const DWORD destination, const DWORD argument);
			virtual	bool	HasAlpha      () { return (_pAlpha != NULL); }

			virtual void	SetInfo       (const string name, const string type, const int blockNumber);
			virtual	string	GetName       () { return _name; }
			virtual string	GetType       () { return _type; }
			virtual int		GetBlockNumber() { return _blockNumber; }
			virtual void	SetSelected   (const bool selected);
			virtual	bool	IsSelected    () { return _isSelected; }

			virtual void	SetColorWireframe(DWORD color) = 0;

			virtual	bool	Render        (LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX& worldMatrix) = 0;
	};
}
