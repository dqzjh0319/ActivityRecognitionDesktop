
// ActivityRecognitionDesktopDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ActivityRecognitionDesktop.h"
#include "ActivityRecognitionDesktopDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_SHOWTASK WM_USER+11

// CAboutDlg dialog used for App About

volatile BOOL m_proScan;
volatile BOOL m_motionDetect;
HWND m_wnd;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CActivityRecognitionDesktopDlg dialog




CActivityRecognitionDesktopDlg::CActivityRecognitionDesktopDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CActivityRecognitionDesktopDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CActivityRecognitionDesktopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CActivityRecognitionDesktopDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	//ON_BN_CLICKED(IDC_BTN_OPEN, &CActivityRecognitionDesktopDlg::OnBnClickedBtnOpen)
	//ON_BN_CLICKED(IDC_BTN_CLOSE, &CActivityRecognitionDesktopDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_PROSCAN, &CActivityRecognitionDesktopDlg::OnBnClickedBtnProscan)
	ON_BN_CLICKED(IDC_BTN_MTANA, &CActivityRecognitionDesktopDlg::OnBnClickedBtnMtana)
END_MESSAGE_MAP()


// CActivityRecognitionDesktopDlg message handlers

BOOL CActivityRecognitionDesktopDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_nid.cbSize  = (DWORD)sizeof(NOTIFYICONDATA);
    m_nid.hWnd    = this->m_hWnd;
    m_nid.uID     = IDR_MAINFRAME;
    m_nid.uFlags  = NIF_ICON | NIF_MESSAGE | NIF_TIP ;
    m_nid.uCallbackMessage = WM_SHOWTASK;             // 自定义的消息名称
    m_nid.hIcon   = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	char charray[128] = "服务器程序";                // 信息提示条为"服务器程序"
	strcpy(m_nid.szTip, charray);
    Shell_NotifyIcon(NIM_ADD, &m_nid);                // 在托盘区添加图标
	m_wnd = AfxGetMainWnd()->m_hWnd;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CActivityRecognitionDesktopDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CActivityRecognitionDesktopDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CActivityRecognitionDesktopDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CActivityRecognitionDesktopDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{
	if(wParam != IDR_MAINFRAME)
		return 1;
    switch(lParam){
		case WM_RBUTTONUP:                                        // 右键起来时弹出菜单
		{
			LPPOINT lpoint = new tagPOINT;
			::GetCursorPos(lpoint);                    // 得到鼠标位置
			CMenu menu;
			menu.CreatePopupMenu();                    // 声明一个弹出式菜单
			LPCTSTR lpsz = "关闭";
			menu.AppendMenu(MF_STRING, WM_DESTROY, lpsz);
			menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x ,lpoint->y, this);
			HMENU hmenu = menu.Detach();
			menu.DestroyMenu();
			delete lpoint;
		}break;
        case WM_LBUTTONDBLCLK:                                 // 双击左键的处理
		{
			this->ShowWindow(SW_SHOWNORMAL);         // 显示主窗口
		}break;
	}
	return 0;
}

void CActivityRecognitionDesktopDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if(nType == SIZE_MINIMIZED)  
	{  
		ShowWindow(SW_HIDE); // 当最小化市，隐藏主窗口              
	}  
}

BOOL CActivityRecognitionDesktopDlg::DestroyWindow()
{
	// 在托盘区删除图标
	Shell_NotifyIcon(NIM_DELETE, &m_nid);             
	return CDialog::DestroyWindow();
}

//void CActivityRecognitionDesktopDlg::OnBnClickedBtnOpen()
//{
//	// TODO: Add your control notification handler code here
//	hThread=CreateThread(NULL,
//        0,
//        (LPTHREAD_START_ROUTINE)ThreadFunc,
//        NULL,
//        0,
//        &ThreadID);
//    GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);
//    GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(TRUE);    
//}


//void CActivityRecognitionDesktopDlg::OnBnClickedBtnClose()
//{
//	// TODO: Add your control notification handler code here
//	m_proScan=FALSE;
//    GetDlgItem(IDC_BTN_OPEN)->EnableWindow(TRUE);
//    GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
//}

void CActivityRecognitionDesktopDlg::DrawPicToHDC(cv::Mat m_cvImg, UINT ID)//IplImage *img, UINT ID)
{
	//CDC *pDC = GetDlgItem(ID)->GetDC();
	//HDC hDC= pDC->GetSafeHdc();
	//CRect rect;
	//GetDlgItem(ID)->GetClientRect(&rect);
	//CvvImage cimg;
	//cimg.CopyOf( img ); // 复制图片
	//cimg.DrawToHDC( hDC, &rect ); // 将图片绘制到显示控件的指定区域内
	//ReleaseDC( pDC );

	BITMAPINFO*			m_bmi;
	BITMAPINFOHEADER*	m_bmih;
	unsigned int		m_buffer[sizeof(BITMAPINFOHEADER)];// + sizeof(RGBQUAD)*256];

	m_bmi = (BITMAPINFO*)m_buffer;
    m_bmih = &(m_bmi->bmiHeader);
    memset(m_bmih, 0, sizeof(*m_bmih));
    m_bmih->biSize = sizeof(BITMAPINFOHEADER);

	m_bmih->biWidth = m_cvImg.cols;
    m_bmih->biHeight = -m_cvImg.rows;           // 在自下而上的位图中 高度为负
    m_bmih->biPlanes = 1;
    m_bmih->biCompression = BI_RGB;
    m_bmih->biBitCount = 8 * m_cvImg.channels();
    CRect rect;
	CDC *pDC = GetDlgItem(ID)->GetDC();
    GetDlgItem(ID)->GetClientRect(&rect);
	SetStretchBltMode(pDC->m_hDC,COLORONCOLOR);
    StretchDIBits(      
        pDC->GetSafeHdc(),
        0, 0, rect.Width(), rect.Height(),
        0, 0, m_cvImg.cols, m_cvImg.rows,
        m_cvImg.data,
        (BITMAPINFO*) m_bmi,
        DIB_RGB_COLORS,
        SRCCOPY);
}

//void ThreadFunc()
//{
//    CTime time;
//    CString strTime;
//    m_proScan=TRUE;
//    while(m_proScan)
//    {
//        time=CTime::GetCurrentTime();
//        strTime=time.Format("%H:%M:%S");
//        ::SetDlgItemText(m_wnd,IDC_TIME,strTime);
//        Sleep(1000);
//    }
//}

void ProcessesScanThread()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;//用来存储进程的相关信息
	DWORD dwPriorityClass;
	m_proScan = TRUE;
	BOOL isWorkProcesses;
	while(m_proScan)
    {
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统进程的快照
		if(hProcessSnap == INVALID_HANDLE_VALUE)
		{
			return;
		}
		if(Process32First( hProcessSnap, &pe32) == NULL)
		{
			CloseHandle(hProcessSnap);
		}
		isWorkProcesses = FALSE;
		do{
			if(strcmp(pe32.szExeFile, "WINWORD.EXE")==0)
			{
				isWorkProcesses = TRUE;
				break;
			}
		}while( Process32Next( hProcessSnap, &pe32) );
		//t_str.Format("%s",pe32.szExeFile);
		if(isWorkProcesses)
			::SetDlgItemText(m_wnd,IDC_EDT_SCAN,"isWorkingPro");
		else
			::SetDlgItemText(m_wnd,IDC_EDT_SCAN,"noWorkingPro");
		Sleep(1000);
		//Process32Next( hProcessSnap, &pe32);
	}
}
void CActivityRecognitionDesktopDlg::OnBnClickedBtnProscan()
{
	// TODO: Add your control notification handler code here
	if(m_proScan == false)
	{
		
		hPSThread=CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ProcessesScanThread,
			NULL,
			0,
			&PSThreadID);
		GetDlgItem(IDC_BTN_PROSCAN)->SetWindowText("Stop");
		GetDlgItem(IDC_LBL_SCAN)->SetWindowText("Scanning...");
	}
	else
	{
		m_proScan = false;
		GetDlgItem(IDC_BTN_PROSCAN)->SetWindowText("Start");
		GetDlgItem(IDC_LBL_SCAN)->SetWindowText("Idle...");
	}
}

void MotionDetectThread(){
	cv::VideoCapture myCap;
	if(!myCap.open(0))
	{
		::SetDlgItemText(m_wnd,IDC_EDT_MT,"Can't open camera!!!");
		return;
	}
	cv::Mat orgFrame;
	m_motionDetect = TRUE;
	while(m_motionDetect){
		if(!myCap.read(orgFrame))
			continue;
		DrawPicToHDC(orgFrame, IDC_ShowImg);
	}
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnMtana()
{
	// TODO: Add your control notification handler code here
	
 //   IplImage *image=NULL; //原始图像
	//if(image) cvReleaseImage(&image);
	//image = cvLoadImage("C:\\Users\\Public\\Pictures\\Sample Pictures\\Jellyfish.jpg",1); //显示图片
	/*cv::Mat temp;
	temp = cv::imread("C:\\Users\\Public\\Pictures\\Sample Pictures\\Desert.jpg");
	DrawPicToHDC(temp, IDC_ShowImg);
	hThread=CreateThread(NULL,
						0,
						(LPTHREAD_START_ROUTINE)MotionDetectThread,
						NULL,
						0,
						&ThreadID);*/
	if(m_motionDetect == false)
	{
		hMDThread=CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)MotionDetectThread,
			NULL,
			0,
			&MDThreadID);
		GetDlgItem(IDC_BTN_MTANA)->SetWindowText("Stop");
		GetDlgItem(IDC_LBL_MT)->SetWindowText("Detecting...");
	}
	else
	{
		m_motionDetect = false;
		GetDlgItem(IDC_BTN_MTANA)->SetWindowText("Start");
		GetDlgItem(IDC_LBL_MT)->SetWindowText("Idle...");
	}
}
