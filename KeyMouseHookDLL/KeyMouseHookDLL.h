// KeyMouseHookDLL.h : main header file for the KeyMouseHookDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CKeyMouseHookDLLApp
// See KeyMouseHookDLL.cpp for the implementation of this class
//
LRESULT  CALLBACK KeyboardProc(
                            int nCode, 
                            WPARAM wParam, 
                            LPARAM lParam);


BOOL __declspec(dllexport)__stdcall  InstallHook(HWND m_wnd, BOOL *s);

BOOL __declspec(dllexport)__stdcall UnHook();

int __declspec(dllexport)__stdcall  add(int a, int b);

class CKeyMouseHookDLLApp : public CWinApp
{
public:
	CKeyMouseHookDLLApp();

// Overrides
public:
	BOOL ExitInstance();
	BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
