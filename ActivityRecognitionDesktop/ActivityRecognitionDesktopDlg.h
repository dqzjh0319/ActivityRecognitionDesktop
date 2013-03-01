
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
#include <opencv2\video\background_segm.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <afxsock.h>

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
	DWORD PSThreadID;

public:
	//afx_msg void OnBnClickedBtnOpen();
	//afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnProscan();
	afx_msg void OnBnClickedBtnMtana();
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnIm();
	afx_msg void OnBnClickedBtnPs();
};
UINT MotionDetectThread(LPVOID lpParam);

void ProcessesScanThread();

int coutfile(LPARAM lParam);

UINT ReceiverThreadProc(LPVOID pParam);

//UINT SenderThreadProc(LPVOID pParam);
//void FrameProcess_Callback();