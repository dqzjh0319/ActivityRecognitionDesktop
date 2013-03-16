// KeyMouseHookDLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "KeyMouseHookDLL.h"
#include "stdio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma data_seg(".SHARDAT")
static HHOOK hkb=NULL;
static HHOOK hm=NULL;
#pragma data_seg()
HINSTANCE hins=NULL;
HWND myhwnd;
BOOL *status;

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CKeyMouseHookDLLApp

BEGIN_MESSAGE_MAP(CKeyMouseHookDLLApp, CWinApp)
END_MESSAGE_MAP()

LRESULT CALLBACK KeyboardProc(int nCode,WPARAM wParam,LPARAM lParam)
{	
	if(nCode < 0) 
		return ::CallNextHookEx(hkb, nCode, wParam, lParam); 
    if(lParam & 0x40000000) // 消息重复就交给下一个hook链 
	{ 
		return ::CallNextHookEx(hkb, nCode, wParam, lParam); 
	} 
	/*FILE* fstream; 
	char str[10]; 
	if(!(fstream=fopen("key.txt","a+"))) 
		return 0;     
	::GetKeyNameText(lParam, str, 10); 
	fprintf(fstream,"123"); 
	fclose(fstream);  
	::SetDlgItemText(myhwnd,1014,"test");*/
	*status = TRUE;
	LRESULT RetVal = CallNextHookEx( hkb, nCode, wParam, lParam );	
	return  RetVal;
}

LRESULT CALLBACK MouseProc(int nCode,WPARAM wParam,LPARAM lParam)
{	
	if(nCode < 0) 
		return ::CallNextHookEx(hm, nCode, wParam, lParam); 
    if(lParam & 0x40000000) // 消息重复就交给下一个hook链 
	{ 
		return ::CallNextHookEx(hm, nCode, wParam, lParam); 
	} 
	/*FILE* fstream; 
	char str[10]; 
	if(!(fstream=fopen("Mouse.txt","a+"))) 
		return 0;     
	::GetKeyNameText(lParam, str, 10); 
	fprintf(fstream,"89"); 
	fclose(fstream); */ 
	*status = TRUE;
	LRESULT RetVal = CallNextHookEx( hm, nCode, wParam, lParam );	
	return  RetVal;
}

BOOL __declspec(dllexport)__stdcall InstallHook(HWND m_wnd, BOOL* s)
{
	myhwnd = m_wnd;
	status = s;
	hkb=SetWindowsHookEx(WH_KEYBOARD_LL,KeyboardProc, 0, 0);
	hm=SetWindowsHookEx(WH_MOUSE_LL,MouseProc, 0, 0);
	return TRUE;
}

BOOL __declspec(dllexport)__stdcall UnHook()
{   	
	BOOL unhooked = UnhookWindowsHookEx(hkb);
	BOOL unhooked2 = UnhookWindowsHookEx(hm);
	return unhooked&unhooked2;
} 
// CKeyMouseHookDLLApp construction

CKeyMouseHookDLLApp::CKeyMouseHookDLLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

int __declspec(dllexport)__stdcall  add(int a, int b)
{
	return a+b;
}

// The one and only CKeyMouseHookDLLApp object

CKeyMouseHookDLLApp theApp;


// CKeyMouseHookDLLApp initialization

BOOL CKeyMouseHookDLLApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CKeyMouseHookDLLApp::ExitInstance ()
{
	return TRUE;
}
