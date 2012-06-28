// ChunkMerge.cpp : Defines the class behaviors for the application.
//

#include "..\Common\stdafx.h"
#include "ChunkMerge.h"
#include "ChunkMergeDlg.h"
#include "..\Common\NifUtlMaterial.h"
#include "..\Common\FDFileHelper.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>

#include <Common/Base/keycode.cxx>

#ifdef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_ANIMATION
#endif
#ifndef HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#endif
#include <Common/Base/Config/hkProductFeatures.cxx> 

static void HK_CALL errorReport(const char* msg, void* userArgGivenToInit)
{
	printf("%s", msg);
}

//  used namespaces
using namespace NifUtility;

#define HK_MAIN_CALL _cdecl


#if defined( HK_ATOM )
extern "C" int __cdecl ADP_Close( void );
#endif

CString					glPathSkyrim;
CString					glPathTemplate;
NifUtlMaterialList		glMaterialList;
CChunkMergeDlg			dlg;


// CChunkMergeApp

BEGIN_MESSAGE_MAP(CChunkMergeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChunkMergeApp construction

CChunkMergeApp::CChunkMergeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CChunkMergeApp object

CChunkMergeApp theApp;


// CChunkMergeApp initialization

BOOL CChunkMergeApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	//  initialize Havok  (500000 bytes of physics solver buffer)
	hkMemoryRouter*		pMemoryRouter(hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(500000)));
	hkBaseSystem::init(pMemoryRouter, errorReport);

  LPWSTR* argv;
  int     argc(0);

  argv = CommandLineToArgvW(m_lpCmdLine, &argc);

	if (argc >= 3)
	{
		//  initialize material map
		glMaterialList.initializeMaterialMap((const char*) CStringA(argv[2]));
	}
	else
	{
		CString	dir = FDFileHelper::getFileOrFolder(_T(""), L"Nif-XML (nif.xml)|nif.xml||", L"xml", false, false, _T("Please select Nif.xml file"));

		glMaterialList.initializeMaterialMap((const char*) CStringA(dir));
	}
	if (argc >= 2)
	{
		glPathTemplate = argv[1];
	}
	else
	{
		glPathTemplate = FDFileHelper::getFileOrFolder(_T(""), L"*.nif (*.nif)|*.nif||", L"nif", false, true, _T("Please select template directory"));
	}
	if ((argc >= 1) && (wcsstr(argv[0], L"ChunkMerge.exe") == NULL))
	{
		glPathSkyrim = argv[0];
	}
	else
	{
		glPathSkyrim = FDFileHelper::getFileOrFolder(_T(""), L"TESV.exe (TESV.exe)|TESV.exe||", L"exe", false, true, _T("Please select SKYRIM directory"));
	}

  LocalFree(argv);

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
