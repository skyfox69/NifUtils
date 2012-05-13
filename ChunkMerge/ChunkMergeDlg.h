// ChunkMergeDlg.h : header file
//

#pragma once

#include <set>

using namespace std;

// CChunkMergeDlg dialog
class CChunkMergeDlg : public CDialog
{
// Construction
public:
	CChunkMergeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CHUNKMERGE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON           m_hIcon;
  CString         m_fileNameAry[3];
  CString         m_texturePath;
  CString         m_collObjPath;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

  virtual void  parseDir(CString path, set<string>& directories, bool doDirs=true);

public:
  afx_msg void OnBnClickedButtonInput();
  afx_msg void OnBnClickedButtonOutput();
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedRadioCollision();
};
