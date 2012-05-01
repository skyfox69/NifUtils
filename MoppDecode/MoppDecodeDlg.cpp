#include "stdafx.h"
#include "MoppDecode.h"
#include "MoppDecodeDlg.h"
#include "FDFileHelper.h"
#include "NifFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMoppDecodeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_BUTTON_INPUT, &CMoppDecodeDlg::OnBnClickedButtonInput)
  ON_BN_CLICKED(IDC_CHECK_MOPPCODE, &CMoppDecodeDlg::OnBnClickedCheckMoppcode)
  ON_BN_CLICKED(IDC_CHECK_MOPPDATA, &CMoppDecodeDlg::OnBnClickedCheckMoppdata)
  ON_BN_CLICKED(IDC_CHECK_MODELFILE, &CMoppDecodeDlg::OnBnClickedCheckModelfile)
  ON_BN_CLICKED(IDC_CHECK_CHUNKDATA, &CMoppDecodeDlg::OnBnClickedCheckChunkdata)
  ON_BN_CLICKED(IDC_BUTTON_MOPPCODE, &CMoppDecodeDlg::OnBnClickedButtonMoppcode)
  ON_BN_CLICKED(IDC_BUTTON_MOPPDATA, &CMoppDecodeDlg::OnBnClickedButtonMoppdata)
  ON_BN_CLICKED(IDC_BUTTON_MODELFILE, &CMoppDecodeDlg::OnBnClickedButtonModelfile)
  ON_BN_CLICKED(IDC_BUTTON_CHUNKDATA, &CMoppDecodeDlg::OnBnClickedButtonChunkdata)
  ON_BN_CLICKED(IDC_BUTTONCONVERT, &CMoppDecodeDlg::OnBnClickedButtonconvert)
  ON_BN_CLICKED(IDC_CHECK_FACEDEF, &CMoppDecodeDlg::OnBnClickedCheckFacedef)
  ON_BN_CLICKED(IDC_BUTTON_FACEDEF, &CMoppDecodeDlg::OnBnClickedButtonFacedef)
END_MESSAGE_MAP()



CMoppDecodeDlg::CMoppDecodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoppDecodeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMoppDecodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CMoppDecodeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

  GetDlgItem(IDC_EDIT_MOPPCODE)   ->EnableWindow(FALSE);
  GetDlgItem(IDC_EDIT_MOPPDATA)   ->EnableWindow(FALSE);
  GetDlgItem(IDC_EDIT_MODELFILE)  ->EnableWindow(FALSE);
  GetDlgItem(IDC_EDIT_CHUNKDATA)  ->EnableWindow(FALSE);
  GetDlgItem(IDC_EDIT_FACEDEF)    ->EnableWindow(FALSE);
  GetDlgItem(IDC_CHECK_MOPPCODE)  ->EnableWindow(FALSE);
  GetDlgItem(IDC_CHECK_MOPPDATA)  ->EnableWindow(FALSE);
  GetDlgItem(IDC_CHECK_MODELFILE) ->EnableWindow(FALSE);
  GetDlgItem(IDC_CHECK_CHUNKDATA) ->EnableWindow(FALSE);
  GetDlgItem(IDC_CHECK_FACEDEF)   ->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_MOPPCODE) ->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_MOPPDATA) ->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_MODELFILE)->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_CHUNKDATA)->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_FACEDEF)->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTONCONVERT)   ->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMoppDecodeDlg::OnPaint()
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

HCURSOR CMoppDecodeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMoppDecodeDlg::OnBnClickedCheckMoppcode()
{
  bool  isChecked(IsDlgButtonChecked(IDC_CHECK_MOPPCODE) != 0);

  GetDlgItem(IDC_EDIT_MOPPCODE)  ->EnableWindow(isChecked);
  GetDlgItem(IDC_BUTTON_MOPPCODE)->EnableWindow(isChecked);
  CheckConvertButton();
  if (!isChecked)
  {
    GetDlgItem(IDC_EDIT_MOPPCODE)->SetWindowTextW(L"");
  }
}

void CMoppDecodeDlg::OnBnClickedCheckMoppdata()
{
  bool  isChecked(IsDlgButtonChecked(IDC_CHECK_MOPPDATA) != 0);

  GetDlgItem(IDC_EDIT_MOPPDATA)  ->EnableWindow(isChecked);
  GetDlgItem(IDC_BUTTON_MOPPDATA)->EnableWindow(isChecked);
  CheckConvertButton();
  if (!isChecked)
  {
    GetDlgItem(IDC_EDIT_MOPPDATA)->SetWindowTextW(L"");
  }
}

void CMoppDecodeDlg::OnBnClickedCheckModelfile()
{
  bool  isChecked(IsDlgButtonChecked(IDC_CHECK_MODELFILE) != 0);

  GetDlgItem(IDC_EDIT_MODELFILE)  ->EnableWindow(isChecked);
  GetDlgItem(IDC_BUTTON_MODELFILE)->EnableWindow(isChecked);
  CheckConvertButton();
  if (!isChecked)
  {
    GetDlgItem(IDC_EDIT_MODELFILE)->SetWindowTextW(L"");
  }
}

void CMoppDecodeDlg::OnBnClickedCheckChunkdata()
{
  bool  isChecked(IsDlgButtonChecked(IDC_CHECK_CHUNKDATA) != 0);

  GetDlgItem(IDC_EDIT_CHUNKDATA)  ->EnableWindow(isChecked);
  GetDlgItem(IDC_BUTTON_CHUNKDATA)->EnableWindow(isChecked);
  CheckConvertButton();
  if (!isChecked)
  {
    GetDlgItem(IDC_EDIT_CHUNKDATA)->SetWindowTextW(L"");
  }
}

void CMoppDecodeDlg::OnBnClickedCheckFacedef()
{
  bool  isChecked(IsDlgButtonChecked(IDC_CHECK_FACEDEF) != 0);

  GetDlgItem(IDC_EDIT_FACEDEF)  ->EnableWindow(isChecked);
  GetDlgItem(IDC_BUTTON_FACEDEF)->EnableWindow(isChecked);
  CheckConvertButton();
  if (!isChecked)
  {
    GetDlgItem(IDC_EDIT_FACEDEF)->SetWindowTextW(L"");
  }
}

void CMoppDecodeDlg::OnBnClickedButtonInput()
{
  m_fileNameAry[0] = FDFileHelper::getFileOrFolder(m_fileNameAry[0], L"Nif Files (*.nif)|*.nif||", L"nif");

  if (!m_fileNameAry[0].IsEmpty())
  {
    GetDlgItem(IDC_EDIT_INPUT)     ->SetWindowText(m_fileNameAry[0]);
    GetDlgItem(IDC_CHECK_MOPPCODE) ->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_MOPPDATA) ->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_MODELFILE)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_CHUNKDATA)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_FACEDEF)  ->EnableWindow(TRUE);
  }
}

void CMoppDecodeDlg::OnBnClickedButtonMoppcode()
{
  bool  enableConvert(true);

  m_fileNameAry[1] = FDFileHelper::getFileOrFolder(m_fileNameAry[1], L"Code Files (*.txt)|*.txt||", L"txt", true);
  GetDlgItem(IDC_EDIT_MOPPCODE)->SetWindowText(m_fileNameAry[1]);
  CheckConvertButton();
}

void CMoppDecodeDlg::OnBnClickedButtonMoppdata()
{
  bool  enableConvert(true);

  m_fileNameAry[2] = FDFileHelper::getFileOrFolder(m_fileNameAry[2], L"Code Files (*.txt)|*.txt||", L"txt", true);
  GetDlgItem(IDC_EDIT_MOPPDATA)->SetWindowText(m_fileNameAry[2]);
  CheckConvertButton();
}

void CMoppDecodeDlg::OnBnClickedButtonModelfile()
{
  bool  enableConvert(true);

  m_fileNameAry[3] = FDFileHelper::getFileOrFolder(m_fileNameAry[3], L"Model Files (*.txt)|*.txt||", L"txt", true);
  GetDlgItem(IDC_EDIT_MODELFILE)->SetWindowText(m_fileNameAry[3]);
  CheckConvertButton();
}

void CMoppDecodeDlg::OnBnClickedButtonChunkdata()
{
  bool  enableConvert(true);

  m_fileNameAry[4] = FDFileHelper::getFileOrFolder(m_fileNameAry[4], L"Data Files (*.txt)|*.txt||", L"txt", true);
  GetDlgItem(IDC_EDIT_CHUNKDATA)->SetWindowText(m_fileNameAry[4]);
  CheckConvertButton();
}

void CMoppDecodeDlg::OnBnClickedButtonFacedef()
{
  bool  enableConvert(true);

  m_fileNameAry[5] = FDFileHelper::getFileOrFolder(m_fileNameAry[5], L"Data Files (*.txt)|*.txt||", L"txt", true);
  GetDlgItem(IDC_EDIT_FACEDEF)->SetWindowText(m_fileNameAry[5]);
  CheckConvertButton();
}

void CMoppDecodeDlg::CheckConvertButton()
{
  bool  enable (!m_fileNameAry[0].IsEmpty());
  bool  disable(false);

  for (int i(0); i < 5; ++i)
  {
    if (IsDlgButtonChecked(IDC_CHECK_MOPPCODE + i))
    {
      enable &= !m_fileNameAry[i+1].IsEmpty();
    }
    if (!disable)
    {
      disable |= (IsDlgButtonChecked(IDC_CHECK_MOPPCODE + i) != 0);
    }
  }

  disable &= enable;
  GetDlgItem(IDC_BUTTONCONVERT)->EnableWindow(disable);
}

void CMoppDecodeDlg::OnBnClickedButtonconvert()
{
  CStatic*        pStatic;
  CProgressCtrl*  pProgress;
  wchar_t         sBuffer[40];
  NifFile         theNif;
  CStringA        tmpStr;
  short           doCount  (0);
  short           isCount  (0);
  bool            doProcess[5];

  pStatic   = (CStatic*) GetDlgItem(IDC_STATIC_PROGRESS);
  pProgress = (CProgressCtrl*) GetDlgItem(IDC_PROGRESS);

  for (int i(0); i < 5; ++i)
  {
    doProcess[i] = false;
    GetDlgItem(IDC_EDIT_MOPPCODE+i)->GetWindowTextW(m_fileNameAry[i+1]);
    if (IsDlgButtonChecked(IDC_CHECK_MOPPCODE + i))
    {
      doProcess[i] = true;
      ++doCount;
    }
  }

  pProgress->SetRange(0, doCount);
  pProgress->SetStep(1);

  //  update step counter
  swprintf_s(sBuffer, 40, L"Step 0/%d", doCount);
  pStatic->SetWindowText(sBuffer);

  //  read nif file
  tmpStr = m_fileNameAry[0];
  theNif.openNif((const char*) tmpStr);

  //  process each selected step
  for (short step(0); step < 5; ++step)
  {
    if (doProcess[step])
    {
      //  update step counter
      swprintf_s(sBuffer, 40, L"Step %d/%d", (isCount + 1), doCount);
      pStatic->SetWindowText(sBuffer);
      pProgress->StepIt();

      switch (step)
      {
        case 0:   //  generate Mopp Code
        {
          tmpStr = m_fileNameAry[1];
          theNif.generateMoppCode((const char*) tmpStr);
          ++isCount;
          break;
        }

        case 1:   //  extract Mopp Data ???
        {
          tmpStr = m_fileNameAry[2];
          theNif.extractMoppData((const char*) tmpStr);
          ++isCount;
          break;
        }

        case 2:   //  generate DXView file
        {
          tmpStr = m_fileNameAry[3];
          theNif.extractDxGeometry((const char*) tmpStr);
          ++isCount;
          break;
        }

        case 3:   //  extract Chunk Data
        {
          tmpStr = m_fileNameAry[4];
          theNif.extractChunkData((const char*) tmpStr);
          ++isCount;
          break;
        }

        case 4:   //  extract face definition
        {
          tmpStr = m_fileNameAry[5];
          theNif.extractFaceDefs((const char*) tmpStr);
          ++isCount;
          break;
        }
      }  //  switch (step)
    }  //  if (doProcess[step])
  }  //  for (short step(0); step < 5; ++step)

  //  close nif file
  theNif.closeNif();

  //  clean up strings
  for (short i(0); i < 6; ++i)
  {
    m_fileNameAry[i].Empty();
  }
}
