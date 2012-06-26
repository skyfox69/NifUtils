// ChunkMergeDlg.cpp : implementation file
//

#include "..\Common\stdafx.h"
#include "ChunkMerge.h"
#include "ChunkMergeDlg.h"
#include "..\Common\FDFileHelper.h"
#include "..\Common\NifCollisionUtility.h"
#include "..\Common\NifUtlMaterial.h"
#include "..\Common\version.h"




using namespace NifUtility;

extern CString					glPathSkyrim;
extern CString					glPathTemplate;
extern CChunkMergeDlg			dlg;
extern NifUtlMaterialList		glMaterialList;


//  static wrapper function
void logCallback(const int type, const char* pMessage)
{
	dlg.logMessage(type, pMessage);
}


// CChunkMergeDlg dialog



BEGIN_MESSAGE_MAP(CChunkMergeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_BUTTON_INPUT, &CChunkMergeDlg::OnBnClickedButtonInput)
  ON_BN_CLICKED(IDC_BUTTON_OUTPUT, &CChunkMergeDlg::OnBnClickedButtonOutput)
  ON_BN_CLICKED(IDOK, &CChunkMergeDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDC_RADIO_COLLISION_1, &CChunkMergeDlg::OnBnClickedRadioCollision)
  ON_BN_CLICKED(IDC_RADIO_COLLISION_2, &CChunkMergeDlg::OnBnClickedRadioCollision)
  ON_BN_CLICKED(IDC_RADIO_COLLISION_3, &CChunkMergeDlg::OnBnClickedRadioCollision)
END_MESSAGE_MAP()


CChunkMergeDlg::CChunkMergeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChunkMergeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChunkMergeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_logView);
}

void CChunkMergeDlg::parseDir(CString path, set<string>& directories, bool doDirs)
{
  CFileFind   finder;
  BOOL        result(FALSE);

  result = finder.FindFile(path + _T("\\*.*"));

  while (result)
  {
    result = finder.FindNextFileW();

    if (finder.IsDots())    continue;
    if (finder.IsDirectory() && doDirs)
    {
      CString   newDir(finder.GetFilePath());
      CString   tDir = newDir.Right(newDir.GetLength() - newDir.Find(_T("\\Textures\\")) - 1);

      directories.insert(CStringA(tDir).GetString());

      parseDir(newDir, directories);
    }
    else if (!finder.IsDirectory() && !doDirs)
    {
      CString   newDir(finder.GetFilePath());
      CString   tDir = newDir.Right(newDir.GetLength() - path.GetLength() - 1);

      directories.insert(CStringA(tDir).GetString());
    }
  }
}

// CChunkMergeDlg message handlers

BOOL CChunkMergeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_RADIO_COLLISION_2))->SetCheck(BST_CHECKED);
	((CButton*) GetDlgItem(IDC_RADIO_COLLMAT_1))  ->SetCheck(BST_CHECKED);

	//  temp. until working
	GetDlgItem(IDC_RADIO_COLLMAT_3)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_COLLMAT) ->EnableWindow(FALSE);

	m_logView.SetBackgroundColor(FALSE, RGB(0x00, 0x00, 0x00));
	m_logView.SetReadOnly       (TRUE);

	//  get templates
	CComboBox*  pCBox = (CComboBox*) GetDlgItem(IDC_COMBO_TEMPLATE);
	CString     pathTemplate(glPathTemplate);
	set<string> directories;

	parseDir(pathTemplate, directories, false);

	for (set<string>::iterator tIter = directories.begin(); tIter != directories.end(); tIter++)
	{
		pCBox->AddString(CString((*tIter).c_str()));
	}
	pCBox->SetCurSel(0);

	//  initialize materials
	map<string, NifUtlMaterial>		mapMap(glMaterialList.getMaterialMap());
	short							t(0);

	pCBox = (CComboBox*) GetDlgItem(IDC_COMBO_COLLMAT);

	for (map<string, NifUtlMaterial>::iterator pIter = mapMap.begin(); pIter != mapMap.end(); pIter++, t++)
	{
		pCBox->InsertString  (t, CString(pIter->second._name.c_str()));
		pCBox->SetItemDataPtr(t, (void*) (pIter->second._code));
	}
	pCBox->SelectString(-1, _T("Stone"));

	//  add messages from material list
	vector<string>  userMessages(glMaterialList.getUserMessages());

	for (vector<string>::iterator texIter = userMessages.begin(); texIter != userMessages.end(); texIter++)
	{
		logMessage(NCU_MSG_TYPE_INFO, texIter->c_str());
	}

	//  set title
	char	cbuffer[100];

	sprintf(cbuffer, "ChunkMerge  v%d.%d.%d.%04d", FD_VERSION, FD_SUBVERSION, FD_REVISION, FD_BUILD);
	SetWindowText(CString(cbuffer));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChunkMergeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChunkMergeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CChunkMergeDlg::OnBnClickedButtonInput()
{
  m_fileNameAry[0] = FDFileHelper::getFileOrFolder(m_fileNameAry[0], L"Nif Files (*.nif)|*.nif||", L"nif");
  GetDlgItem(IDC_EDIT_INPUT)->SetWindowText(m_fileNameAry[0]);
  if (!m_fileNameAry[0].IsEmpty() && !m_fileNameAry[1].IsEmpty())
  {
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }
}

void CChunkMergeDlg::OnBnClickedButtonOutput()
{
  m_fileNameAry[1] = FDFileHelper::getFileOrFolder(m_fileNameAry[1], L"Nif Files (*.nif)|*.nif|Object Files (*.obj)|*.obj||", L"nif");
  GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(m_fileNameAry[1]);
  if (!m_fileNameAry[0].IsEmpty() && !m_fileNameAry[1].IsEmpty())
  {
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }
}

void CChunkMergeDlg::OnBnClickedRadioCollision()
{
}

void CChunkMergeDlg::OnBnClickedOk()
{
  NifCollisionUtility		ncUtility(glMaterialList);
  map<int, unsigned int>	materialMap;
  unsigned short			ncReturn (NCU_OK);

  //  copy strings from input
  GetDlgItem(IDC_EDIT_INPUT)    ->GetWindowTextW(m_fileNameAry[0]);
  GetDlgItem(IDC_EDIT_OUTPUT)   ->GetWindowTextW(m_fileNameAry[1]);
  GetDlgItem(IDC_COMBO_TEMPLATE)->GetWindowTextW(m_fileNameAry[2]);

  //  set path
  ncUtility.setSkyrimPath ((CStringA(glPathSkyrim)).GetString());

  //  set callback for log info
  ncUtility.setLogCallback(logCallback);

  //  get material handling
  MaterialTypeHandling	matHandling((MaterialTypeHandling) (GetCheckedRadioButton(IDC_RADIO_COLLMAT_1, IDC_RADIO_COLLMAT_3) - IDC_RADIO_COLLMAT_1));

  switch (matHandling)
  {
	case NCU_MT_SINGLE:
	{
		CComboBox*	pCBox((CComboBox*) GetDlgItem(IDC_COMBO_COLLMAT));

		materialMap[-1] = ((unsigned int) pCBox->GetItemDataPtr(pCBox->GetCurSel()));
		break;
	}

	case NCU_MT_NITRISHAPE_NAME:
	{
		break;
	}

	case NCU_MT_MATMAP:
	{
		//  take material map from extra dialog
		break;
	}
  }

  //  set flags
  ncUtility.setCollisionNodeHandling((CollisionNodeHandling) (GetCheckedRadioButton(IDC_RADIO_COLLISION_1, IDC_RADIO_COLLISION_2) - IDC_RADIO_COLLISION_1));
  ncUtility.setMaterialTypeHandling (matHandling, materialMap);

  //  convert nif
  ncReturn = ncUtility.addCollision((CStringA(m_fileNameAry[1])).GetString(), (CStringA(m_fileNameAry[0])).GetString(), (CStringA(glPathTemplate + L"\\" + m_fileNameAry[2])).GetString());
  if (ncReturn != NCU_OK)
  {
    logMessage(NCU_MSG_TYPE_ERROR, "NifConverter returned code: " + ncReturn);
  }
  else
  {
    logMessage(NCU_MSG_TYPE_ERROR, "Nif converted successfully");
  }
	logMessage(NCU_MSG_TYPE_INFO, "- - - - - - - - - -");
}

void CChunkMergeDlg::logMessage(const int type, const char* pMessage)
{
	CHARFORMAT	charFormat;
	COLORREF	color       (RGB(0x00, 0xD0, 0x00));
	CString		text        (pMessage);
	int			lineCountOld(m_logView.GetLineCount());
	int			tType       (type);

	//  special handling of type settings
	if (pMessage[0] == '^')
	{
		tType = atoi(pMessage+1);
		text  = (pMessage+2);
	}

	//  append of newline necessary?
	if (pMessage[strlen(pMessage) - 1] != '\n')
	{
		text += _T("\r\n");
	}

	//  get color by type
	switch (tType)
	{
		case NCU_MSG_TYPE_ERROR:
		{
			color = RGB(0xFF, 0x00, 0x00);
			break;
		}

		case NCU_MSG_TYPE_WARNING:
		{
			color = RGB(0xFF, 0xFF, 0x00);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE:
		{
			color = RGB(0x50, 0x50, 0xFF);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE_MISS:
		{
			color = RGB(0xC0, 0x50, 0xFF);
			break;
		}

		case NCU_MSG_TYPE_SUB_INFO:
		{
			color = RGB(0x00, 0x60, 0x00);
			break;
		}
	}

	//  character format
	charFormat.cbSize      = sizeof(charFormat);
	charFormat.dwMask      = CFM_COLOR;
	charFormat.dwEffects   = 0;
	charFormat.crTextColor = color;

	//  select  nothing, set format and append new text
	m_logView.SetSel(-1, -1);
	m_logView.SetSelectionCharFormat(charFormat);
	m_logView.ReplaceSel(text);

	//  scroll to end of text
	m_logView.LineScroll(m_logView.GetLineCount() - lineCountOld);
}
