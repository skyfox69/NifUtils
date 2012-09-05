#pragma once

#include "DirectXGraphics.h"

namespace NifUtility
{
	class CDirectXView : public CWnd, public CDirectXGraphics
	{
		protected:
			UINT_PTR		_unpTimer;

		public:
							CDirectXView();
			virtual			~CDirectXView();

			afx_msg	void	OnTimer(UINT nIDEvent);
			afx_msg	void	OnPaint();
			afx_msg	int		OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg	void	OnMouseMove(UINT nFlags, CPoint point);
			afx_msg	BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint point);

			virtual	void	SetOwnTimer(UINT_PTR pTimer);

			DECLARE_MESSAGE_MAP()
	};
}
