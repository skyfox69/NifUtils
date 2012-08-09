// ChunkMergeDlg.cpp : implementation file
//

#include "..\Common\stdafx.h"
#include "ChunkMerge.h"
#include "ChunkMergeDlg.h"
#include "..\Common\Configuration.h"
#include "..\Common\FDFileHelper.h"
#include "..\Common\NifCollisionUtility.h"
#include "..\Common\NifUtlMaterial.h"
#include "version.h"




using namespace NifUtility;

extern CChunkMergeDlg			dlg;
extern NifUtlMaterialList		glMaterialList;
extern Configuration			glConfig;

//  ToolTip definitions
struct FDToolTipText
{
	int		_uid;
	string	_text;
};

static FDToolTipText	glTTText[] = {{IDC_RADIO_VCADD, "Add default vertext colors of 0xffffff in case of set flag SLSF2_Vertex_Colors"},
									  {IDC_RADIO_VCREMOVE, "Remove flag SLSF2_Vertex_Colors in case of missing vertex colors"},
									  {IDC_RADIO_COLLISION_1, "Use RootCollisionNode as source for collision shapes"},
									  {IDC_RADIO_COLLISION_2, "Use RootCollisionNode as source for collision shapes if not existing use NiTriShapes"},
									  {IDC_RADIO_COLLISION_3, "Use NiTriShapes as source for collision shapes"},
									  {IDC_RADIO_COLLMAT_1, "Use single material for collision data"},
									  {IDC_RADIO_COLLMAT_2, "Use multiple materials defined by name of NiTriShapes for collision data (e.g. HAV_MAT_STONE"},
									  {IDC_RADIO_COLLMAT_3, "Use multiple materials as defined by additional dialog for collision data"},
									  {IDC_COMBO_TEMPLATE, "Define NIF file used as template for converting and adding collision data"},
									  {IDC_COMBO_COLLMAT, "Define material used for single collision material"},
									  {IDC_EDIT_INPUT, "Path to source NIF-file to be converted"},
									  {IDC_EDIT_OUTPUT, "Path to destination NIF-file"},
									  {IDC_BUTTON_INPUT, "Choose source NIF-file to be converted"},
									  {IDC_BUTTON_OUTPUT, "Choose destination NIF-file"},
									  {IDC_BUTTON_TEMPLATE, "Choose path to template files and scan recursively"},
									  {IDC_RICHEDIT_LOG, "Some log output"},
									  {-1, ""}
									 };

//  static wrapper function
void logCallback(const int type, const char* pMessage)
{
	dlg.logMessage(type, pMessage);
}


// CChunkMergeDlg dialog



BEGIN_MESSAGE_MAP(CChunkMergeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_BUTTON_INPUT, &CChunkMergeDlg::OnBnClickedButtonInput)
  ON_BN_CLICKED(IDC_BUTTON_OUTPUT, &CChunkMergeDlg::OnBnClickedButtonOutput)
  ON_BN_CLICKED(IDOK, &CChunkMergeDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDC_RADIO_COLLISION_1, &CChunkMergeDlg::OnBnClickedRadioCollision)
  ON_BN_CLICKED(IDC_RADIO_COLLISION_2, &CChunkMergeDlg::OnBnClickedRadioCollision)
  ON_BN_CLICKED(IDC_RADIO_COLLISION_3, &CChunkMergeDlg::OnBnClickedRadioCollision)
	ON_BN_CLICKED(IDC_RADIO_COLLMAT_1, &CChunkMergeDlg::OnBnClickedRadioCollmat)
	ON_BN_CLICKED(IDC_RADIO_COLLMAT_2, &CChunkMergeDlg::OnBnClickedRadioCollmat)
	ON_BN_CLICKED(IDC_RADIO_COLLMAT_3, &CChunkMergeDlg::OnBnClickedRadioCollmat)
	ON_COMMAND(ID_DEFAULT_SAVESETTINGS, &CChunkMergeDlg::OnDefaultSavesettings)
	ON_COMMAND(ID_DEFAULT_RELOADDIRECTORIES, &CChunkMergeDlg::OnDefaultReloaddirectories)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE, &CChunkMergeDlg::OnBnClickedButtonTemplate)
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

BOOL CChunkMergeDlg::PreTranslateMessage(MSG* pMsg)
{
     m_toolTip.RelayEvent(pMsg);

     return CDialog::PreTranslateMessage(pMsg);
}

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
	OnBnClickedRadioCollmat();

	//  temp. until working
	GetDlgItem(IDC_RADIO_COLLMAT_3)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_COLLMAT) ->EnableWindow(FALSE);

	//  prepare log view
	CHARFORMAT	cf = { 0 };

	cf.cbSize    = sizeof(cf);
	cf.dwMask    = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_PROTECTED;
	cf.dwEffects = 0;
	cf.yHeight   = 127;
	lstrcpy(cf.szFaceName, _T("Small Fonts"));

	m_logView.SetDefaultCharFormat(cf);
	m_logView.SetBackgroundColor(FALSE, RGB(0x00, 0x00, 0x00));
	m_logView.SetReadOnly       (TRUE);

	//  prepare tool tips
	if (m_toolTip.Create(this, TTS_BALLOON))
	{
		for (short i(0); glTTText[i]._uid != -1; ++i)
		{
			m_toolTip.AddTool(GetDlgItem(glTTText[i]._uid), CString(glTTText[i]._text.c_str()));
		}

		m_toolTip.Activate(TRUE);
	}

	//  scan directories and fill combo boxes
	OnDefaultReloaddirectories();

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
	if (m_fileNameAry[0].IsEmpty())
	{
		m_fileNameAry[0] = glConfig._dirDestination.c_str();
	}
  m_fileNameAry[0] = FDFileHelper::getFileOrFolder(m_fileNameAry[0], L"Nif Files (*.nif)|*.nif||", L"nif");
  GetDlgItem(IDC_EDIT_INPUT)->SetWindowText(m_fileNameAry[0]);
  if (!m_fileNameAry[0].IsEmpty() && !m_fileNameAry[1].IsEmpty())
  {
    GetDlgItem(IDOK)->EnableWindow(TRUE);
  }
}

void CChunkMergeDlg::OnBnClickedButtonOutput()
{
	if (m_fileNameAry[1].IsEmpty())
	{
		m_fileNameAry[1] = glConfig._dirCollision.c_str();
	}
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

void CChunkMergeDlg::OnBnClickedButtonTemplate()
{
	CString		pathTmpl(glConfig._pathTemplate.c_str());

	//  get new path to templates
	pathTmpl = FDFileHelper::getFileOrFolder(pathTmpl + _T("\\"), L"*.nif (*.nif)|*.nif||", L"nif", false, true);

	//  if path given - reload templates
	if (!pathTmpl.IsEmpty())
	{
		CComboBox*	pCBox = (CComboBox*) GetDlgItem(IDC_COMBO_TEMPLATE);
		set<string> directories;

		//  reset selection box
		pCBox->ResetContent();

		//  parse directory
		parseDir(pathTmpl, directories, false);

		//  in case of existing data
		if (directories.size() > 0)
		{
			//  add files to selection box
			for (set<string>::iterator tIter = directories.begin(); tIter != directories.end(); tIter++)
			{
				if ((*tIter).rfind(".nif") == string::npos)		continue;
				pCBox->AddString(CString((*tIter).c_str()));
			}
			pCBox->SetCurSel(0);

			//  set new path to config
			glConfig._pathTemplate = CStringA(pathTmpl).GetString();

			//  reset last choosen template
			glConfig._lastTemplate = *directories.begin();
		}
	}
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
  ncUtility.setSkyrimPath(glConfig._pathSkyrim);

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
  ncUtility.setCollisionNodeHandling((CollisionNodeHandling) (GetCheckedRadioButton(IDC_RADIO_COLLISION_1, IDC_RADIO_COLLISION_3) - IDC_RADIO_COLLISION_1));
  ncUtility.setMaterialTypeHandling (matHandling, materialMap);

  //  convert nif
  ncReturn = ncUtility.addCollision((CStringA(m_fileNameAry[1])).GetString(), (CStringA(m_fileNameAry[0])).GetString(), (CStringA(CString(glConfig._pathTemplate.c_str()) + L"\\" + m_fileNameAry[2])).GetString());
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

void CChunkMergeDlg::OnBnClickedRadioCollmat()
{
	switch (GetCheckedRadioButton(IDC_RADIO_COLLMAT_1, IDC_RADIO_COLLMAT_3))
	{
		case IDC_RADIO_COLLMAT_1:
		{
			GetDlgItem(IDC_BUTTON_COLLMAT)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_COLLMAT) ->EnableWindow(TRUE);
			break;
		}

		case IDC_RADIO_COLLMAT_2:
		{
			GetDlgItem(IDC_BUTTON_COLLMAT)                ->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_COLLMAT)                 ->EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_RADIO_COLLISION_1))->SetCheck(BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_RADIO_COLLISION_2))->SetCheck(BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_RADIO_COLLISION_3))->SetCheck(BST_CHECKED);
			break;
		}

		case IDC_RADIO_COLLMAT_3:
		{
			GetDlgItem(IDC_BUTTON_COLLMAT)                ->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_COLLMAT)                 ->EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_RADIO_COLLISION_1))->SetCheck(BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_RADIO_COLLISION_2))->SetCheck(BST_UNCHECKED);
			((CButton*) GetDlgItem(IDC_RADIO_COLLISION_3))->SetCheck(BST_CHECKED);
			break;
		}
	}
}

void CChunkMergeDlg::OnContextMenu(CWnd *pWnd, CPoint point)
{
	CMenu*	pPopUp(NULL);
	CMenu	menuBar;

	menuBar.LoadMenu(IDR_MENU_POPUP);

	pPopUp = menuBar.GetSubMenu(0);
	pPopUp->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CChunkMergeDlg::OnDefaultSavesettings()
{
	//  write last used options
	CComboBox*	pCBox((CComboBox*) GetDlgItem(IDC_COMBO_COLLMAT));

	//  copy strings from input
	GetDlgItem(IDC_EDIT_INPUT)    ->GetWindowTextW(m_fileNameAry[0]);
	GetDlgItem(IDC_EDIT_OUTPUT)   ->GetWindowTextW(m_fileNameAry[1]);
	GetDlgItem(IDC_COMBO_TEMPLATE)->GetWindowTextW(m_fileNameAry[2]);

	//  modify settings
	glConfig._collMaterial      = ((int) pCBox->GetItemDataPtr(pCBox->GetCurSel()));
	glConfig._matHandling       = (GetCheckedRadioButton(IDC_RADIO_COLLMAT_1, IDC_RADIO_COLLMAT_3) - IDC_RADIO_COLLMAT_1);
	glConfig._collTypeHandling  = (GetCheckedRadioButton(IDC_RADIO_COLLISION_1, IDC_RADIO_COLLISION_3) - IDC_RADIO_COLLISION_1);
	glConfig._lastTemplate      = CStringA(m_fileNameAry[2]).GetString();
	glConfig._lastTexture       = CStringA(m_texturePath).GetString();
	glConfig._dirDestination    = CStringA(m_fileNameAry[0].Left(m_fileNameAry[0].ReverseFind('\\') + 1)).GetString();
	glConfig._dirCollision      = CStringA(m_fileNameAry[1].Left(m_fileNameAry[1].ReverseFind('\\') + 1)).GetString();

	//  write last config
	glConfig.write();
}

void CChunkMergeDlg::OnDefaultReloaddirectories()
{
	logMessage(NCU_MSG_TYPE_INFO, "scanning directories...");

	//  re-scan templates
	CComboBox*	pCBox = (CComboBox*) GetDlgItem(IDC_COMBO_TEMPLATE);
	CString		pathTmpl(glConfig._pathTemplate.c_str());
	set<string> directories;

	//  reset selection box
	pCBox->ResetContent();

	//  parse directory
	parseDir(pathTmpl, directories, false);

	//  in case of existing data
	if (directories.size() > 0)
	{
		//  add files to selection box
		for (set<string>::iterator tIter = directories.begin(); tIter != directories.end(); tIter++)
		{
			if ((*tIter).rfind(".nif") == string::npos)		continue;
			pCBox->AddString(CString((*tIter).c_str()));
		}
		pCBox->SelectString(-1, CString(glConfig._lastTemplate.c_str()));

		//  make sure one entry is selected
		GetDlgItem(IDC_COMBO_TEMPLATE) ->GetWindowTextW(pathTmpl);
		if (pathTmpl.IsEmpty())
		{
			pCBox->SetCurSel(0);
		}

		//  reset last choosen template
		GetDlgItem(IDC_COMBO_TEMPLATE)->GetWindowTextW(pathTmpl);
		glConfig._lastTemplate = CStringA(pathTmpl).GetString();
	}

	//  re-load material
	glMaterialList.initializeMaterialMap(glConfig._pathNifXML);

	map<string, NifUtlMaterial>		mapMap(glMaterialList.getMaterialMap());
	short							t     (0);
	short							selIdx(0);

	pCBox = (CComboBox*) GetDlgItem(IDC_COMBO_COLLMAT);

	for (map<string, NifUtlMaterial>::iterator pIter = mapMap.begin(); pIter != mapMap.end(); pIter++, t++)
	{
		pCBox->InsertString  (t, CString(pIter->second._name.c_str()));
		pCBox->SetItemDataPtr(t, (void*) (pIter->second._code));

		if (pIter->second._code == ((unsigned int) glConfig._collMaterial))
		{
			selIdx = t;
		}
	}
	pCBox->SetCurSel(selIdx);

	//  add messages from material list
	vector<string>  userMessages(glMaterialList.getUserMessages());

	for (vector<string>::iterator texIter = userMessages.begin(); texIter != userMessages.end(); texIter++)
	{
		logMessage(NCU_MSG_TYPE_INFO, texIter->c_str());
	}

	logMessage(NCU_MSG_TYPE_INFO, "scan done.");
}
