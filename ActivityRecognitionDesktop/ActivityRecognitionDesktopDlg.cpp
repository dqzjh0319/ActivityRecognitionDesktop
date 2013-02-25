
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

typedef BOOL(*FUNC)(BOOL, DWORD, HWND);
typedef int(*FUND)();

struct threadInfo{
	CWnd* m_video;
};

volatile BOOL m_proScan;
volatile BOOL m_motionDetect;
volatile BOOL m_InputMonitor;
volatile BOOL m_CommListen;

BOOL moving;
BOOL inputing;
BOOL working;

CCriticalSection sec_moving;
CCriticalSection sec_inputing;
CCriticalSection sec_working;

HWND m_wnd;
CWinThread* pMDThread;
threadInfo Info;
static HINSTANCE hinstDLL; 
typedef BOOL (CALLBACK *inshook)(HWND m_wnd); 
typedef BOOL (CALLBACK *unhook)();
unhook unstkbhook;
inshook instkbhook;

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
	, m_EditReceive(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CActivityRecognitionDesktopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSCOMM, m_mscomm);
	DDX_Text(pDX, IDC_STATIC_RD, m_EditReceive);
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
	ON_BN_CLICKED(IDC_BTN_TEST, &CActivityRecognitionDesktopDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_IM, &CActivityRecognitionDesktopDlg::OnBnClickedBtnIm)
	ON_BN_CLICKED(IDC_BTN_CL, &CActivityRecognitionDesktopDlg::OnBnClickedBtnCl)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CActivityRecognitionDesktopDlg, CDialogEx)
	ON_EVENT(CActivityRecognitionDesktopDlg, IDC_MSCOMM, 1, CActivityRecognitionDesktopDlg::OnCommMscomm, VTS_NONE)
END_EVENTSINK_MAP()

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
	moving = TRUE;
	inputing = TRUE;
	working = FALSE;
	sec_moving.Unlock();
	sec_inputing.Unlock();
	sec_working.Unlock();
	GetDlgItem(IDC_EDT_SCAN)->SetWindowText("N/A");
	GetDlgItem(IDC_EDT_MT)->SetWindowText("N/A");
	GetDlgItem(IDC_EDT_IM)->SetWindowText("N/A");

	m_nid.cbSize  = (DWORD)sizeof(NOTIFYICONDATA);
    m_nid.hWnd    = this->m_hWnd;
    m_nid.uID     = IDR_MAINFRAME;
    m_nid.uFlags  = NIF_ICON | NIF_MESSAGE | NIF_TIP ;
    m_nid.uCallbackMessage = WM_SHOWTASK;             // �Զ������Ϣ����
    m_nid.hIcon   = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	char charray[128] = "����������";                // ��Ϣ��ʾ��Ϊ"����������"
	strcpy(m_nid.szTip, charray);
    Shell_NotifyIcon(NIM_ADD, &m_nid);                // �����������ͼ��
	m_wnd = AfxGetMainWnd()->m_hWnd;
	m_InputMonitor = false;
	m_CommListen = false;

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
		case WM_RBUTTONUP:                                        // �Ҽ�����ʱ�����˵�
		{
			LPPOINT lpoint = new tagPOINT;
			::GetCursorPos(lpoint);                    // �õ����λ��
			CMenu menu;
			menu.CreatePopupMenu();                    // ����һ������ʽ�˵�
			LPCTSTR lpsz = "�ر�";
			menu.AppendMenu(MF_STRING, WM_DESTROY, lpsz);
			menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x ,lpoint->y, this);
			HMENU hmenu = menu.Detach();
			menu.DestroyMenu();
			delete lpoint;
		}break;
        case WM_LBUTTONDBLCLK:                                 // ˫������Ĵ���
		{
			this->ShowWindow(SW_SHOWNORMAL);         // ��ʾ������
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
		ShowWindow(SW_HIDE); // ����С���У�����������              
	}  
}

BOOL CActivityRecognitionDesktopDlg::DestroyWindow()
{
	// ��������ɾ��ͼ��
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
	//cimg.CopyOf( img ); // ����ͼƬ
	//cimg.DrawToHDC( hDC, &rect ); // ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������
	//ReleaseDC( pDC );

	BITMAPINFO*			m_bmi;
	BITMAPINFOHEADER*	m_bmih;
	unsigned int		m_buffer[sizeof(BITMAPINFOHEADER)];// + sizeof(RGBQUAD)*256];

	m_bmi = (BITMAPINFO*)m_buffer;
    m_bmih = &(m_bmi->bmiHeader);
    memset(m_bmih, 0, sizeof(*m_bmih));
    m_bmih->biSize = sizeof(BITMAPINFOHEADER);

	m_bmih->biWidth = m_cvImg.cols;
    m_bmih->biHeight = -m_cvImg.rows;           // �����¶��ϵ�λͼ�� �߶�Ϊ��
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
			::SetDlgItemText(m_wnd,IDC_EDT_MT,"isMoving...");
			sec_moving.Lock();
			moving = TRUE;
			sec_moving.Unlock();
		}
		else
		{
			::SetDlgItemText(m_wnd,IDC_EDT_MT,"isStatic...");
			sec_moving.Lock();
			moving = FALSE;
			sec_moving.Unlock();
		}
		//To display the final processed frames;
		memset(m_bmih, 0, sizeof(*m_bmih));
		m_bmih->biSize = sizeof(BITMAPINFOHEADER);
		m_bmih->biWidth = m_cvImg.cols;
		m_bmih->biHeight = -m_cvImg.rows;           // �����¶��ϵ�λͼ�� �߶�Ϊ��
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
	}
	return 0;
}

void ProcessesScanThread()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;//�����洢���̵������Ϣ
	DWORD dwPriorityClass;
	m_proScan = TRUE;
	BOOL isWorkProcesses;
	while(m_proScan)
    {
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//���ϵͳ���̵Ŀ���
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
			::SetDlgItemText(m_wnd,IDC_EDT_SCAN,"isWorkingPro");
			sec_working.Lock();
			working = TRUE;
			sec_working.Unlock();
		}
		else
		{
			::SetDlgItemText(m_wnd,IDC_EDT_SCAN,"noWorkingPro");
			sec_working.Lock();
			working = FALSE;
			sec_working.Unlock();
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
		GetDlgItem(IDC_EDT_SCAN)->SetWindowText("N/A");
	}
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnMtana()
{
	// TODO: Add your control notification handler code here
	
 //   IplImage *image=NULL; //ԭʼͼ��
	//if(image) cvReleaseImage(&image);
	//image = cvLoadImage("C:\\Users\\Public\\Pictures\\Sample Pictures\\Jellyfish.jpg",1); //��ʾͼƬ
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

void CActivityRecognitionDesktopDlg::OnBnClickedBtnTest()
{
	// TODO: Add your control notification handler code here
	//CWnd* m_video;
	//m_video = GetDlgItem(IDC_ShowImg);
	////cv::Mat tmpImg;
	////tmpImg = cv::imread("C:\\Users\\Public\\Pictures\\Sample Pictures\\Jellyfish.jpg");
	///*if (tmpImg == NULL) 
	//{
	//	GetDlgItem(IDC_BTN_MTANA)->SetWindowText("Can't open the image!!!");
	//	return;
	//}*/
	///*if (!pCapture.open(0)) 
	//{
	//	MessageBox("Failed to open the camera!!!");
	//	return;
	//}*/
	////Info.cvImg = tmpImg;
	//Info.m_video = m_video;
	/*const char* dllName = "KeyHookLib.dll"; 
	const char* funName3 = "installHook";
	HMODULE hDLL = LoadLibrary(dllName);
	FUNC fp3 = FUNC(GetProcAddress(hDLL,funName3));*/
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnIm()
{
	// TODO: Add your control notification handler code here
	if(m_InputMonitor == false)
	{
		if(hinstDLL=LoadLibrary((LPCTSTR)"KeyMouseHookDLL.dll"))
		{
			instkbhook=(inshook)GetProcAddress(hinstDLL, "InstallHook"); 
			unstkbhook=(unhook)GetProcAddress(hinstDLL, "UnHook");
		}
		if(instkbhook(m_wnd))
			GetDlgItem(IDC_LBL_IM)->SetWindowText("Montering...");
		else
			GetDlgItem(IDC_LBL_IM)->SetWindowText("Failed Hook...");
		m_InputMonitor = true;
		GetDlgItem(IDC_BTN_IM)->SetWindowText("Stop");
	}
	else
	{
		unstkbhook();
		m_InputMonitor = false;
		GetDlgItem(IDC_BTN_IM)->SetWindowText("Start");
		GetDlgItem(IDC_LBL_IM)->SetWindowText("Idle...");
		FreeLibrary(hinstDLL);
	}
}

void CActivityRecognitionDesktopDlg::OnCommMscomm()
{
	// TODO: Add your message handler code here
	static unsigned int cnt=0;
    VARIANT variant_inp;
    COleSafeArray safearray_inp;
    long len,k;
    unsigned intdata[1024]={0};
    byte rxdata[1024];//����BYTE����
    CString strtemp;
    if (m_mscomm.get_CommEvent()==2)//ֵΪ2��ʾ���ջ����������ַ�
    {
        cnt++;
        variant_inp = m_mscomm.get_Input();//��ȡ������
        safearray_inp = variant_inp;//����ת��
        len = safearray_inp.GetOneDimSize();//�õ���Ч�����ݳ���
        for (k=0;k<len;k++)
        {
            safearray_inp.GetElement(&k,rxdata+k);
        }
        char c_char;
        for (k=0;k<len;k++)
        {
            strtemp.Format(_T("%c"),*(rxdata+k));
            m_EditReceive+=strtemp;
        }
        //CString temp=_T("\r\r\n");//����
        //m_EditReceive+=temp;
    }
    UpdateData(FALSE);
}

void CActivityRecognitionDesktopDlg::OnBnClickedBtnCl()
{
	// TODO: Add your control notification handler code here
	if(!m_CommListen)
	{
		if (m_mscomm.get_PortOpen())//��������Ǵ򿪵ģ���رմ���
		{
			m_mscomm.put_PortOpen(FALSE);
		}
		m_mscomm.put__CommPort(3);//ѡ��COM3,�ұʼǱ�����û���ڣ�usbת�����Ĵ��ں�Ϊ3���ɸ���ʵ��ѡ�񣬵����ڹرյ�ҲҪ����Ӧ����
		m_mscomm.put_InBufferSize(1024);//���ջ�����
		m_mscomm.put_OutBufferSize(1024);//���ͻ�����
		m_mscomm.put_InputLen(0);//���õ�ǰ���������ݳ���Ϊ0����ʾȫ����ȡ
		m_mscomm.put_InputMode(1);//�Զ����Ʒ�ʽ��д����
		m_mscomm.put_RThreshold(1);//���ջ�������1����1�������ַ�ʱ���������������ݵ�OnComm�¼�
		m_mscomm.put_Settings(_T("9600,n,8,1"));//������9600����У�飬8������λ��1��ֹͣ1λ
		if (!m_mscomm.get_PortOpen())//�������û�д�
		{
			m_mscomm.put_PortOpen(TRUE);//�򿪴���
			GetDlgItem(IDC_LBL_CL)->SetWindowText("Listening...");
			GetDlgItem(IDC_BTN_CL)->SetWindowText("Close");
			m_CommListen = TRUE;
		}
		else
		{
			m_mscomm.put_OutBufferCount(0);//
			//AfxMessageBox(_T("Port 3 open successful!"));
			GetDlgItem(IDC_LBL_CL)->SetWindowText("Open failed!!!");
			GetDlgItem(IDC_EDT_CL)->SetWindowText("N/A");
			GetDlgItem(IDC_BTN_CL)->SetWindowText("Open");
			m_CommListen = FALSE;
		}
	}
	else
	{
		m_mscomm.put_PortOpen(FALSE);  
        AfxMessageBox(_T("����3�ѹرգ�"));  
		GetDlgItem(IDC_LBL_CL)->SetWindowText("Port closed...");
		GetDlgItem(IDC_EDT_CL)->SetWindowText("N/A");
		GetDlgItem(IDC_BTN_CL)->SetWindowText("Open");
		m_CommListen = FALSE;
	}
}

