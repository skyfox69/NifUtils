#pragma once

#include "afxwin.h"
#include <d3dx9.h>
#include <vector>

using namespace std;

namespace NifUtility
{
	class DirectXMesh;

	enum  DirecXCameraPos { DX_CAM_POS_TOP = 0, DX_CAM_POS_FRONT, DX_CAM_POS_SIDE };

	class CDirectXGraphics
	{
		protected:
			LPDIRECT3D9				_pD3D;				//  Used to create the D3DDevice
			LPDIRECT3DDEVICE9		_pd3dDevice;		//  Our rendering device
			D3DXVECTOR3				_vecViewCam;		//  Position of camera
			vector<DirectXMesh*>	_meshList;
			CRect					_rectOrig;
			float					_posX;
			float					_posY;
			float					_posXLast;
			float					_posYLast;
			float					_zoom;
			float					_rotX;
			float					_rotY;
			bool					_showAxes;
			bool					_showModel;

			virtual	bool					dxCreateRenderingContext(HWND hWnd, const int width, const int height);
			virtual	bool					dxDestroyRenderingContext();
			virtual	bool					dxInitScene();
			virtual	bool					dxCleanupScene();
			virtual	bool					dxBeginScene();
			virtual	bool					dxEndScene();
			virtual	bool					dxRenderScene();

		public:
											CDirectXGraphics();
			virtual							~CDirectXGraphics();

			virtual	void					dxCreate      (CRect rect, CWnd* pParent, CWnd* pSelf);
			virtual bool					dxShutdown    ();
			virtual void					dxSetCameraPos(const DirecXCameraPos pos);
			virtual	void					dxSetShowAxes (bool show);
			virtual bool					dxGetShowAxes () { return _showAxes; }
			virtual	void					dxSetShowModel(bool show);
			virtual bool					dxGetShowModel() { return _showModel; }

			virtual void					dxAddMesh    (DirectXMesh* pMesh);
			virtual	vector<DirectXMesh*>&	dxGetMeshList() { return _meshList; }
			virtual	void					dxClearModel ();
	};
}
