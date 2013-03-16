
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
#define MD_THRESHOLD 300
#define STATUS_REFRESH_TIMER 1
#define INPUT_STATUS_TIMER 2

typedef BOOL(*FUNC)(BOOL, DWORD, HWND);
typedef int(*FUND)();

struct threadInfo{
	CWnd* m_video;
	cv::CascadeClassifier haar_cascade;
};

volatile BOOL m_proScan;
volatile BOOL m_motionDetect;
volatile BOOL m_InputMonitor;
volatile BOOL m_ProStat;
volatile BOOL timerTest;
volatile BOOL m_ReqPosture;

BOOL isMoving;
BOOL isWorkingProcesses;
BOOL isInputing;
int posture;
int tPosture;
int keepCount;

CCriticalSection motionCS;
CCriticalSection workProCS;
CCriticalSection inputCS;
CCriticalSection postureCS;

BOOL tmpInputStatus;

HWND m_wnd;
CWinThread* pMDThread;
CWinThread* pPStatThread;
threadInfo Info;
static HINSTANCE hinstDLL; 
typedef BOOL (CALLBACK *inshook)(HWND m_wnd, BOOL *s); 
typedef BOOL (CALLBACK *unhook)();
unhook unstkbhook;
inshook instkbhook;

int status;

// CAboutDlg dialog used for App About
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
	ON_WM_TIMER()
	//ON_BN_CLICKED(IDC_BTN_OPEN, &CActivityRecognitionDesktopDlg::OnBnClickedBtnOpen)
	//ON_BN_CLICKED(IDC_BTN_CLOSE, &CActivityRecognitionDesktopDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_BTN_PROSCAN, &CActivityRecognitionDesktopDlg::OnBnClickedBtnProscan)
	ON_BN_CLICKED(IDC_BTN_MTANA, &CActivityRecognitionDesktopDlg::OnBnClickedBtnMtana)
	ON_BN_CLICKED(IDC_BTN_TEST, &CActivityRecognitionDesktopDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_IM, &CActivityRecognitionDesktopDlg::OnBnClickedBtnIm)
	ON_BN_CLICKED(IDC_BTN_PS, &CActivityRecognitionDesktopDlg::OnBnClickedBtnPs)
	ON_BN_CLICKED(IDOK, &CActivityRecognitionDesktopDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_PR, &CActivityRecognitionDesktopDlg::OnBnClickedBtnPr)
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

	m_proScan = FALSE;
	m_motionDetect = FALSE;
	m_InputMonitor = FALSE;
	m_ProStat = FALSE;
	m_ReqPosture = FALSE;
	status = 2;

	workProCS.Lock();
	isWorkingProcesses = FALSE;
	workProCS.Unlock();
	motionCS.Lock();
	isMoving = FALSE;
	motionCS.Unlock();
	inputCS.Lock();
	isInputing = FALSE;
	inputCS.Unlock();
	postureCS.Lock();
	posture = 0;
	postureCS.Unlock();

	SetTimer(STATUS_REFRESH_TIMER, 2000, NULL);

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

//void CActivityRecognitionDesktopDlg::FaceDetection()
//{
//	cv::Mat orgFrame;
//	m_faceDetect = TRUE;
//	cv::Mat gray;
//	mCap.read(orgFrame);
//	cv::cvtColor(orgFrame, gray, CV_BGR2GRAY);
//	orgFrame.release();
//	std::vector< cv::Rect_<int> > faces;
//    haar_cascade.detectMultiScale(gray, faces);
//	gray.release();
//	int num = faces.size();
//	CString faceNumStr;
//	faceNumStr.Format("%d", num);
//	GetDlgItem(IDC_LBL_MT)->SetWindowText(faceNumStr);
//}

UINT MotionDetectThread(LPVOID lpParam)
{
    threadInfo* pInfo=(threadInfo*)lpParam;
	CWnd* m_video = pInfo->m_video;
	cv::VideoCapture mCap;
	if(!mCap.open(0))
	{
		return -1;
	}
	cv::Mat m_cvImg;
	cv::Mat orgFrame;
	cv::Mat tmpFrame;
	BITMAPINFO*			m_bmi;
	BITMAPINFOHEADER*	m_bmih;
	unsigned int		m_buffer[sizeof(BITMAPINFOHEADER)];// + sizeof(RGBQUAD)*256];
	m_bmi = (BITMAPINFO*)m_buffer;
	m_bmih = &(m_bmi->bmiHeader);
	CRect rect;
	CDC *pDC = m_video->GetDC();
	m_video->GetClientRect(&rect);
	SetStretchBltMode(pDC->m_hDC,COLORONCOLOR);
	m_motionDetect = TRUE;
	cv::BackgroundSubtractorMOG m_bs;
	while(m_motionDetect)
	{
		mCap.read(orgFrame);
		m_bs(orgFrame,tmpFrame,0.1);
		std::vector<cv::Mat> bgr_planes;
		tmpFrame.copyTo(m_cvImg);
		//cv::split(tmpFrame, bgr_planes);
		int histSize = 256;
		// Set the ranges ( for B,G,R) )
		float range[] = { 0, 256 } ;
		const float* histRange = { range };
		bool uniform = true; bool accumulate = false;
		cv::Mat b_hist;//, g_hist, r_hist;
		/// Compute the histograms:
		calcHist( &tmpFrame, 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
		////calcHist( &bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
		////calcHist( &bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
		double maxV;
		cv::minMaxLoc(b_hist,0, &maxV);
		//*unsigned char test = b_hist.at<cv::Vec2b>(0,0).val[0];*/
		/*char str[20];
		sprintf(str,"%d",(int)(307200-maxV));*/
		//::SetDlgItemText(m_wnd,IDC_EDT_MT,str);
		int motionSum = tmpFrame.cols*tmpFrame.rows - (int)maxV;
		if(motionSum > MD_THRESHOLD)
		{
			motionCS.Lock();
			isMoving = TRUE;
			motionCS.Unlock();
		}
		else
		{
			motionCS.Lock();
			isMoving = FALSE;
			motionCS.Unlock();
		}
		//To display the final processed frames;
		memset(m_bmih, 0, sizeof(*m_bmih));
		m_bmih->biSize = sizeof(BITMAPINFOHEADER);
		m_bmih->biWidth = m_cvImg.cols;
		m_bmih->biHeight = -m_cvImg.rows;           // 在自下而上的位图中 高度为负
		m_bmih->biPlanes = 1;
		m_bmih->biCompression = BI_RGB;
		m_bmih->biBitCount = 8 * m_cvImg.channels();
		//Fit the frame size to the display window size
		StretchDIBits(      
			pDC->GetSafeHdc(),
			0, 0, rect.Width(), rect.Height(),
			0, 0, m_cvImg.cols, m_cvImg.rows,
			m_cvImg.data,
			(BITMAPINFO*) m_bmi,
			DIB_RGB_COLORS,
			SRCCOPY);
		Sleep(1000);
	}
	return 0;
}

//UINT FaceDetectionThread(LPVOID lpParam)
//{
//    threadInfo* pInfo=(threadInfo*)lpParam;
//	CWnd* m_video = pInfo->m_video;
//	cv::CascadeClassifier haar_cascade = pInfo->haar_cascade;
//	cv::VideoCapture mCap;
//	if(!mCap.open(0))
//	{
//		return -1;
//	}
//	cv::Mat m_cvImg;
//	cv::Mat orgFrame;
//	cv::Mat tmpFrame;
//	BITMAPINFO*			m_bmi;
//	BITMAPINFOHEADER*	m_bmih;
//	unsigned int		m_buffer[sizeof(BITMAPINFOHEADER)];// + sizeof(RGBQUAD)*256];
//	m_bmi = (BITMAPINFO*)m_buffer;
//	m_bmih = &(m_bmi->bmiHeader);
//	CRect rect;
//	CDC *pDC = m_video->GetDC();
//	m_video->GetClientRect(&rect);
//	SetStretchBltMode(pDC->m_hDC,COLORONCOLOR);
//	m_faceDetect = TRUE;
//	cv::Mat gray;
//	while(m_faceDetect)
//	{
//		mCap.read(m_cvImg);
//		cv::cvtColor(m_cvImg, gray, CV_BGR2GRAY);
//		std::vector< cv::Rect_<int> > faces;
//        haar_cascade.detectMultiScale(gray, faces);
//		int num = faces.size();
//
//		for(int i = 0; i < faces.size(); i++) {
//            // Process face by face:
//            cv::Rect face_i = faces[i];
//			cv::rectangle(m_cvImg, face_i, CV_RGB(0, 255,0), 1);
//		}
//
//		//To display the final processed frames;
//		memset(m_bmih, 0, sizeof(*m_bmih));
//		m_bmih->biSize = sizeof(BITMAPINFOHEADER);
//		m_bmih->biWidth = m_cvImg.cols;
//		m_bmih->biHeight = -m_cvImg.rows;           // 在自下而上的位图中 高度为负
//		m_bmih->biPlanes = 1;
//		m_bmih->biCompression = BI_RGB;
//		m_bmih->biBitCount = 8 * m_cvImg.channels();
//		//Fit the frame size to the display window size
//		StretchDIBits(      
//			pDC->GetSafeHdc(),
//			0, 0, rect.Width(), rect.Height(),
//			0, 0, m_cvImg.cols, m_cvImg.rows,
//			m_cvImg.data,
//			(BITMAPINFO*) m_bmi,
//			DIB_RGB_COLORS,
//			SRCCOPY);
//		m_cvImg.release();
//	}
//	return 0;
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
		{
			workProCS.Lock();
			isWorkingProcesses = TRUE;
			workProCS.Unlock();
		}
		else
		{
			workProCS.Lock();
			isWorkingProcesses = FALSE;
			workProCS.Unlock();
		}
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
	CWnd* m_video;
	m_video = GetDlgItem(IDC_ShowImg);
	Info.m_video = m_video;
	if(m_motionDetect == false)
	{
		pMDThread = AfxBeginThread(MotionDetectThread, &Info);
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

void CActivityRecognitionDesktopDlg::OnBnClickedBtnIm()
{
	// TODO: Add your control notification handler code here
	if(m_InputMonitor == FALSE)
	{
		if(hinstDLL=LoadLibrary((LPCTSTR)"KeyMouseHookDLL.dll"))
		{
			instkbhook=(inshook)GetProcAddress(hinstDLL, "InstallHook"); 
			unstkbhook=(unhook)GetProcAddress(hinstDLL, "UnHook");
		}
		if(instkbhook(m_wnd, &tmpInputStatus))
			GetDlgItem(IDC_LBL_IM)->SetWindowText("Montering...");
		else
			GetDlgItem(IDC_LBL_IM)->SetWindowText("Failed Hook...");
		m_InputMonitor = TRUE;
		GetDlgItem(IDC_BTN_IM)->SetWindowText("Stop");
		SetTimer(INPUT_STATUS_TIMER, 5000, NULL);
	}
	else
	{
		unstkbhook();
		m_InputMonitor = FALSE;
		GetDlgItem(IDC_BTN_IM)->SetWindowText("Start");
		GetDlgItem(IDC_LBL_IM)->SetWindowText("Idle...");
		FreeLibrary(hinstDLL);
		KillTimer(INPUT_STATUS_TIMER);
	}
}

UINT ReceiverThreadProc(LPVOID pParam)
{
	CSocket RecSock;
	CSocket Connect;
	char buffer[1024];
	if (!AfxSocketInit())  
    {  
        AfxMessageBox("Socket initialized failed!!!");  
        return FALSE;   
    }
	if(!RecSock.Create((UINT)23456, SOCK_STREAM, "127.0.0.1"))
	{
		return -1;
	}
	if (!RecSock.Listen())
	{
		return -1;
	}
    if(!RecSock.Accept(Connect))
    {        
        CString str;
        str.Format("Accept error code:%d",RecSock.GetLastError());
        AfxMessageBox(str);
        return 0;
    }
    while(true)
	{
		int len = Connect.Receive(buffer,sizeof(buffer));
		if(len == 0)
		{
			break;
		}
		else if (len<0)
		{        
			CString str;
			str.Format("Receive error code:%d",RecSock.GetLastError());
			AfxMessageBox(str);
			break;
		}
		
		//CTime t = CTime::GetCurrentTime();
		CString rstr, sstr;
		rstr.Format("%s",buffer);
		rstr = rstr.GetBufferSetLength(len);
		if(rstr == "req status")
		{
			switch(status)
			{
			case 1:sstr = "Sleeping";break;
			case 2:sstr = "Working";break;
			case 3:sstr = "Playing";break;
			default: sstr = "Internel error";break;
			}
		}
		else
		{
			sstr = "Bad request!";
		}
		Connect.Send(sstr.GetBuffer(0), sstr.GetLength());
		//str = t.Format("[%Y-%m-%d %H:%M:%S]") + str;
	}
    Connect.Close();
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnTest()
{


}

void CActivityRecognitionDesktopDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString tmp;
	switch(nIDEvent)
	{
	case INPUT_STATUS_TIMER: 
		inputCS.Lock();
		isInputing = tmpInputStatus;
		inputCS.Unlock();
		tmpInputStatus = FALSE;
		break;
	case STATUS_REFRESH_TIMER:
		if(isWorkingProcesses)
			GetDlgItem(IDC_EDT_SCAN)->SetWindowText("isWorkingPro...");
		else
			GetDlgItem(IDC_EDT_SCAN)->SetWindowText("noWorkingPro...");
		if(isMoving)
			GetDlgItem(IDC_EDT_MT)->SetWindowText("isMoving...");
		else
			GetDlgItem(IDC_EDT_MT)->SetWindowText("isStatic...");
		if(isInputing)
			GetDlgItem(IDC_EDT_IM)->SetWindowText("isInputing...");
		else
			GetDlgItem(IDC_EDT_IM)->SetWindowText("noInputing...");
		switch(posture)
		{
		case 0: 
			GetDlgItem(IDC_EDT_PR)->SetWindowText("unavailable...");break;
		case (int)'1': 
			GetDlgItem(IDC_EDT_PR)->SetWindowText("Left");break;
		case (int)'2': 
			GetDlgItem(IDC_EDT_PR)->SetWindowText("Right");break;
		}
		if(!isMoving && !isInputing)
		{
			GetDlgItem(IDC_LBL_STAT)->SetWindowText("Sleeping");
			status = 1;
		}
		else if(isWorkingProcesses)
		{
			GetDlgItem(IDC_LBL_STAT)->SetWindowText("Working");
			status = 2;
		}
		else
		{
			GetDlgItem(IDC_LBL_STAT)->SetWindowText("Playing");
			status = 3;
		}
		break;
	default: ;break;
	}
}

UINT RequestThreadProc(LPVOID pParam)
{
	if (!AfxSocketInit())  
	{  
		AfxMessageBox("Socket initial failed!!!");  
		return FALSE;   
	}
	/*char buf[1024];
	CSocket reqSock;
	int rlen;
	reqSock.Create(34567U,SOCK_STREAM,"127.0.0.1");
	if (!reqSock.Connect("127.0.0.1",23456U))
	{
		CString str;
		str.Format("Connect error code:%d",reqSock.GetLastError());
		AfxMessageBox(str);
		return 0;
	}
	while(m_ReqPosture)
	{
		CString reqCmdStr = "req posture";
		reqSock.Send(reqCmdStr,reqCmdStr.GetLength());
		rlen = reqSock.Receive(buf,sizeof(buf));
		CString t_disStr;
		t_disStr.Format("%s",buf);
		t_disStr = t_disStr.GetBufferSetLength(rlen);
		if(t_disStr == "normal")
		{
			postureCS.Lock();
			posture = 1;
			postureCS.Unlock();
		}
		Sleep(3000);
	}
	postureCS.Lock();
	posture = 0;
	postureCS.Unlock();
	reqSock.Close();
	return 1;*/
	//CSocket recSoc;
	////recSoc.Create(12345U,SOCK_DGRAM,"127.0.0.1");
	//char buf[1024];
	//CString serAddr("127.0.0.1");
	//recSoc.Bind(22222,"127.0.0.1");
	//while(true)
	//{
	//	int rlen = -1;
	//	while(rlen == -1)
	//	{
	//		rlen = recSoc.rec
	//	}
	//	postureCS.Lock();
	//	posture = rlen;
	//	postureCS.Unlock();
	//}
	WSADATA wsaData; 
	int error=WSAStartup(MAKEWORD(1,1),&wsaData);
	if(error!=0)
	{
		return -1;
	}
	if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return -1;
	}
	SOCKET s=socket(AF_INET,SOCK_DGRAM,0);

	SOCKADDR_IN sockSrc;
	sockSrc.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	sockSrc.sin_port=htons(22222);
	sockSrc.sin_family=AF_INET;

	bind(s,(SOCKADDR *)&sockSrc,sizeof(SOCKADDR));

	char recBuff[1024];
	memset(recBuff,0,1024);
 
	SOCKADDR_IN sockRec;
	int len=sizeof(SOCKADDR);
	while(m_ReqPosture)
	{
		int x=-1;
		while(x==-1)
		{
			x=recvfrom(s,recBuff,sizeof(recBuff),0,(sockaddr *)&sockRec,&len); 
		}
		postureCS.Lock();
		posture = (int)recBuff[0];
		if(posture == tPosture)
			keepCount++;
		else
		{
			tPosture = posture;
			keepCount = 0;
		}
		postureCS.Unlock();
	}
	closesocket(s);
	WSACleanup();
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnPs()
{
	// TODO: Add your control notification handler code here
	if(m_ProStat == false)
	{	
		pPStatThread = AfxBeginThread(ReceiverThreadProc, NULL);
		GetDlgItem(IDC_LBL_PS)->SetWindowText("Providing...");
		m_ProStat = true;
		GetDlgItem(IDC_BTN_PS)->SetWindowText("Stop");
	}
	else
	{
		m_ProStat = false;
		GetDlgItem(IDC_BTN_PS)->SetWindowText("Start");
		GetDlgItem(IDC_LBL_PS)->SetWindowText("Idle...");
	}
}

void CActivityRecognitionDesktopDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	KillTimer(STATUS_REFRESH_TIMER);
	CDialogEx::OnOK();
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnPr()
{
	// TODO: Add your control notification handler code here
	if(m_ReqPosture == false)
	{	
		pPStatThread = AfxBeginThread(RequestThreadProc, NULL);
		GetDlgItem(IDC_LBL_PR)->SetWindowText("Requesting...");
		m_ReqPosture = true;
		keepCount = 0;
		GetDlgItem(IDC_BTN_PR)->SetWindowText("Stop");
	}
	else
	{
		m_ReqPosture = false;
		postureCS.Lock();
		posture = 0;
		postureCS.Unlock();
		GetDlgItem(IDC_BTN_PR)->SetWindowText("Start");
		GetDlgItem(IDC_LBL_PR)->SetWindowText("Idle...");
		GetDlgItem(IDC_EDT_PR)->SetWindowText("unavailable...");
	}
}
