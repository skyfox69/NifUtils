// MoppDecodeDlg.h : header file
//

#pragma once


// CMoppDecodeDlg dialog
class CMoppDecodeDlg : public CDialog
{
// Construction
public:
	CMoppDecodeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MOPPDECODE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
  CString m_fileNameAry[6];

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

  virtual void  CheckConvertButton();

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedButtonInput();
  afx_msg void OnBnClickedCheckMoppcode();
  afx_msg void OnBnClickedCheckMoppdata();
  afx_msg void OnBnClickedCheckModelfile();
  afx_msg void OnBnClickedCheckChunkdata();
  afx_msg void OnBnClickedButtonMoppcode();
  afx_msg void OnBnClickedButtonMoppdata();
  afx_msg void OnBnClickedButtonModelfile();
  afx_msg void OnBnClickedButtonChunkdata();
  afx_msg void OnBnClickedButtonconvert();
  afx_msg void OnBnClickedCheckFacedef();
  afx_msg void OnBnClickedButtonFacedef();
};
