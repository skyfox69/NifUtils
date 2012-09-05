#include "../Common/stdafx.h"
#include "DirectXView.h"

using namespace NifUtility;

BEGIN_MESSAGE_MAP(CDirectXView, CWnd)
	ON_WM_PAINT()
//	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CDirectXView::CDirectXView()
{
}

CDirectXView::~CDirectXView()
{
}

void CDirectXView::OnPaint()
{
	ValidateRect(NULL);
}

void CDirectXView::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
		case 1:
		{
			dxBeginScene();
			dxRenderScene();
			dxEndScene();
			break;
		}

		default:
		{
			break;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

BOOL CDirectXView::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	_zoom += (float)0.002f * zDelta;

	if (_zoom < 0.02f)	_zoom = 0.02f;

	return FALSE;
}

void CDirectXView::OnMouseMove(UINT nFlags, CPoint point)
{
	int diffX = (int)(point.x - _posXLast);
	int diffY = (int)(point.y - _posYLast);

	_posXLast  = (float)point.x;
	_posYLast  = (float)point.y;

	// Left mouse button
	if (nFlags & MK_LBUTTON)
	{
		_rotX += (float)0.5f * diffY;

		if ((_rotX > 360.0f) || (_rotX < -360.0f))
		{
			_rotX = 0.0f;
		}

		_rotY += (float)0.5f * diffX;

		if ((_rotY > 360.0f) || (_rotY < -360.0f))
		{
			_rotY = 0.0f;
		}
	}

	// Right mouse button
	else if (nFlags & MK_RBUTTON)
	{
		_zoom -= (float)0.1f * diffY;
		if (_zoom < 0.02f)	_zoom = 0.02f;
	}

	// Middle mouse button
	else if (nFlags & MK_MBUTTON)
	{
		_posX += (float)0.05f * diffX;
		_posY -= (float)0.05f * diffY;
	}

	CWnd::OnMouseMove(nFlags, point);
}

int CDirectXView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)	return -1;

	if (!dxCreateRenderingContext(GetSafeHwnd(), _rectOrig.Width(), _rectOrig.Height()))
	{
		dxShutdown();
		return false;
	}

	if (!dxInitScene())
	{
		dxShutdown();
		return false;
	}

	return 0;
}

void CDirectXView::SetOwnTimer(UINT_PTR pTimer)
{
	_unpTimer = pTimer;
}
