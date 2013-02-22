
// ActivityRecognitionDesktopDlg.h : header file
//

#pragma once

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <windows.h>
#include <tlhelp32.h>
#include "D:\\opencv\\build\\my\\install\\CvvImage.h"
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\video\video.hpp>

// CActivityRecognitionDesktopDlg dialog
class CActivityRecognitionDesktopDlg : public CDialogEx
{
// Construction
public:
	CActivityRecognitionDesktopDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ACTIVITYRECOGNITIONDESKTOP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void DrawPicToHDC(cv::Mat m_cvImg, UINT ID);//IplImage *img, UINT ID);


// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_nid;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP();
	LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);
	void OnSize(UINT nType, int cx, int cy);
	BOOL DestroyWindow();
	HANDLE hPSThread;
	HANDLE hMDThread;
	DWORD PSThreadID;
	DWORD MDThreadID;

public:
	//afx_msg void OnBnClickedBtnOpen();
	//afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnProscan();
	afx_msg void OnBnClickedBtnMtana();
	friend void ProcessesScanThread();
	friend void MotionDetectThread();
};

//void ThreadFunc();

//void ProcessesScanThread();

//void MotionDetectThread();

//void FrameProcess_Callback();