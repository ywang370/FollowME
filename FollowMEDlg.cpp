// FollowMEDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FollowME.h"
#include "FollowMEDlg.h"
#include "Cvvimage.h"
#include "math.h"
#define PI 3.14159265
#define SQRT2 1.414

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NO_CONTROL -32768
#define M_PWM 0
#define M_POSITION 1
#define M_VELOCITY 2
#define cFULL_COUNT 32767
#define cWHOLE_RANGE 1200

#define TIMER_FRAME 1

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFollowMEDlg dialog




CFollowMEDlg::CFollowMEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFollowMEDlg::IDD, pParent)
	, is_connected(false)
	, m_ir0(0)
	, m_ir1(0)
	, m_ir2(0)
	, m_ir3(0)
	, m_ir4(0)
	, m_ir5(0)
	, m_ir6(0)
	, m_sonar0(0)
	, m_sonar1(0)
	, m_sonar2(0)
	, m_motion_direction(0)
	, m_encoder0(0)
	, m_encoder1(0)
	, m_speed(0)
	, m_direction(0)
	, pedestrain_thread_param(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFollowMEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VITAMINCTRL1, m_VitCtrl);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_connect);
	DDX_Control(pDX, IDC_EDIT_USER, m_user);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Control(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ip);
	DDX_Control(pDX, IDC_DRROBOTSDKCONTROLCTRL1, m_MOTSDK);
	DDX_Text(pDX, IDC_EDIT_IR0, m_ir0);
	DDX_Text(pDX, IDC_EDIT_IR1, m_ir1);
	DDX_Text(pDX, IDC_EDIT_IR2, m_ir2);
	DDX_Text(pDX, IDC_EDIT_IR3, m_ir3);
	DDX_Text(pDX, IDC_EDIT_IR4, m_ir4);
	DDX_Text(pDX, IDC_EDIT_IR5, m_ir5);
	DDX_Text(pDX, IDC_EDIT_IR6, m_ir6);
	DDX_Text(pDX, IDC_EDIT_SONAR0, m_sonar0);
	DDX_Text(pDX, IDC_EDIT_SONAR1, m_sonar1);
	DDX_Text(pDX, IDC_EDIT_SONAR2, m_sonar2);
	DDX_Control(pDX, IDC_SLIDER_SPEED, m_slider_speed);
	DDX_Control(pDX, IDC_SLIDER_DIRECTION, m_slider_direction);
	DDX_Control(pDX, IDC_EDIT_DIRECTION, m_edit_speed);
	DDX_Control(pDX, IDC_EDIT_SPEED, m_edit_direction);
}

BEGIN_MESSAGE_MAP(CFollowMEDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CFollowMEDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDOK, &CFollowMEDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD, &CFollowMEDlg::OnBnClickedButtonForward)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CFollowMEDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CFollowMEDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_BACKWARD, &CFollowMEDlg::OnBnClickedButtonBackward)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CFollowMEDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MOVE, &CFollowMEDlg::OnBnClickedButtonMove)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_STN_CLICKED(IDC_Image_View, &CFollowMEDlg::OnStnClickedImageView)
END_MESSAGE_MAP()


// CFollowMEDlg message handlers

BOOL CFollowMEDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	// set the camera
	is_connected=false;
	m_user.SetWindowTextA("root");
	m_password.SetWindowTextA("drrobot");
	m_port.SetWindowTextA("8081");
	m_ip.SetWindowTextA("192.168.0.199");

	// set the robot
	m_MOTSDK.connectRobot ("DrRobotMotion");
	m_slider_speed.SetRange(0, 100);
	m_slider_speed.SetPos(m_speed);
	char str[80];
	sprintf(str, "%d", m_speed);
	m_edit_speed.SetWindowTextA(str);
	m_slider_direction.SetRange(0, 360);
	m_slider_direction.SetPos(m_direction);
	sprintf(str, "%d", m_direction);
	m_edit_direction.SetWindowTextA(str);

	// for the human detector
	scanner=new DetectionScanner(HUMAN_height,HUMAN_width,HUMAN_xdiv,HUMAN_ydiv,256,0.8);
	LoadCascade(*scanner);
	pedestrain_thread_param=new PEDESTRAINTHREADPARAM;
	pedestrain_thread_param->scanner=scanner;
	pedestrain_thread_param->is_processing=false;
	pedestrain_thread_param->window=(void *)this;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFollowMEDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFollowMEDlg::OnPaint()
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
HCURSOR CFollowMEDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*
connect the camera
*/
void CFollowMEDlg::OnBnClickedButtonConnect()
{
	// TODO: Add your control notification handler code here
	// connect the camera
	if (is_connected)
	{
		m_VitCtrl.Disconnect();
		is_connected=false;
		m_connect.SetWindowTextA("Connect");
		KillTimer(TIMER_FRAME);
	}
	else
	{
		CString user;
		m_user.GetWindowTextA(user);
		m_VitCtrl.put_UserName((LPCTSTR) user);

		CString password;
		m_password.GetWindowTextA(password);
		m_VitCtrl.put_Password((LPCTSTR) password);

		CString ip;
		m_ip.GetWindowTextA(ip);
		m_VitCtrl.put_RemoteIPAddr((LPCTSTR) ip);

		CString port;
		m_port.GetWindowTextA(port);
		m_VitCtrl.put_HttpPort(atoi(port));

		m_VitCtrl.put_ServerModelType(2);
		m_VitCtrl.Connect();
		m_VitCtrl.RecallPtzPosition("followme5");

		// change the button caption
		is_connected=true;
		m_connect.SetWindowTextA("Stop");

		// start the timer, where we capture the frame and run pedestrain detection
		SetTimer(TIMER_FRAME, 200, NULL);

	}
}

void CFollowMEDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if (is_connected)
	{
		m_VitCtrl.Disconnect();
	}
	OnOK();
}

/*
those functions control the camera location
left, far controls the focusing of the camera
*/
void CFollowMEDlg::OnBnClickedButtonForward()
{
	// TODO: Add your control notification handler code here
	m_VitCtrl.SendCameraCommand("up", 30000);
}

void CFollowMEDlg::OnBnClickedButtonLeft()
{
	// TODO: Add your control notification handler code here
	m_VitCtrl.SendCameraCommand("left", 30000);
}

void CFollowMEDlg::OnBnClickedButtonRight()
{
	// TODO: Add your control notification handler code here
	m_VitCtrl.SendCameraCommand("right", 30000);
}

void CFollowMEDlg::OnBnClickedButtonBackward()
{
	// TODO: Add your control notification handler code here
	m_VitCtrl.SendCameraCommand("down", 30000);
}

void CFollowMEDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here
	m_MOTSDK.DisableDcMotor (0);
	m_MOTSDK.DisableDcMotor (1);
}
BEGIN_EVENTSINK_MAP(CFollowMEDlg, CDialog)
	ON_EVENT(CFollowMEDlg, IDC_DRROBOTSDKCONTROLCTRL1, 1, CFollowMEDlg::StandardSensorEventDrrobotsdkcontrolctrl1, VTS_NONE)
	ON_EVENT(CFollowMEDlg, IDC_DRROBOTSDKCONTROLCTRL1, 2, CFollowMEDlg::MotorSensorEventDrrobotsdkcontrolctrl1, VTS_NONE)
	ON_EVENT(CFollowMEDlg, IDC_DRROBOTSDKCONTROLCTRL1, 3, CFollowMEDlg::CustomSensorEventDrrobotsdkcontrolctrl1, VTS_NONE)
	ON_EVENT(CFollowMEDlg, IDC_VITAMINCTRL1, 1, CFollowMEDlg::OnClickVitaminctrl1, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CFollowMEDlg::StandardSensorEventDrrobotsdkcontrolctrl1()
{
	// TODO: Add your message handler code here
	m_sonar0= m_MOTSDK.GetSensorSonar1 ();
	m_sonar1= m_MOTSDK.GetSensorSonar2 ();
	m_sonar2= m_MOTSDK.GetSensorSonar3 ();

	m_ir0 = m_MOTSDK.GetSensorIRRange ();
	UpdateData(false);
}
/*
those sensors don't work properly
*/
void CFollowMEDlg::MotorSensorEventDrrobotsdkcontrolctrl1()
{
	// TODO: Add your message handler code here
	//m_ir1 = m_MOTSDK.GetCustomAD3 ();
	//m_ir2 = m_MOTSDK.GetCustomAD4 ();
	//m_ir3 = m_MOTSDK.GetCustomAD5 ();
	//m_ir4 = m_MOTSDK.GetCustomAD6 ();
	//m_ir5 = m_MOTSDK.GetCustomAD7 ();
	//m_ir6 = m_MOTSDK.GetCustomAD8 ();
	//UpdateData(false);
}

void CFollowMEDlg::CustomSensorEventDrrobotsdkcontrolctrl1()
{
	// TODO: Add your message handler code here
	m_encoder1 = m_MOTSDK.GetEncoderPulse2 ();
	m_encoder0 = m_MOTSDK.GetEncoderPulse1 ();
}

void CFollowMEDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	// check whether we need to grab a frame or not
	if (nIDEvent==TIMER_FRAME)
	{
		// we process a new frame, when the previous one is finished
		if (!pedestrain_thread_param->is_processing)
		{
			AfxBeginThread(PedestrainThreadFunction, pedestrain_thread_param);
		}
	}
	CDialog::OnTimer(nIDEvent);
}

// // this function converts this VARIANT to IplImage
int CFollowMEDlg::Variant2IpplImage(VARIANT vData , VARIANT vInfo, IplImage *frame)
{
	unsigned int width, height;
	width=*((long *)vInfo.parray[0].pvData+2);
	height=*((long *)vInfo.parray[0].pvData+6);
	IplImage* img_tmp;
	img_tmp=cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	if (frame==0)
	{
		frame=cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	}
	memcpy(img_tmp->imageData, (unsigned char *)vData.parray[0].pvData+54, width*height*3);
	cvConvertImage(img_tmp, frame, CV_CVTIMG_FLIP);
	cvReleaseImage(&img_tmp);
	return 0;
}

// this function shows the IplImage in the picture control
void CFollowMEDlg::ShowImage(IplImage* img, UINT ID)
{
	// get the DC
	CDC* pDC = GetDlgItem( ID ) ->GetDC();
	// get HDC
	HDC hDC = pDC ->GetSafeHdc();				
 
	CRect rect;
	GetDlgItem(ID) ->GetClientRect( &rect );
	// for device size
	int rw = rect.right - rect.left;			
	int rh = rect.bottom - rect.top;
	// and the image size
	int iw = img->width;	
	int ih = img->height;
	// center the image
	int tx = (int)(rw - iw)/2;	
	int ty = (int)(rh - ih)/2;
	SetRect( rect, tx, ty, tx+iw, ty+ih );
	// copy the image
	CvvImage cimg;
	cimg.CopyOf( img );
	// start drawing
	cimg.DrawToHDC( hDC, &rect );	
 
	ReleaseDC( pDC );
}

// this function implements the robot moving function
// direction: 0~pi*2
// speed: 0~100
void CFollowMEDlg::RobotMove(int direction, int speed)
{
	// compute the speed for each wheel
	short left=(short) SQRT2*speed*10*sin(direction*PI/180+PI/4);
	short right=(short) SQRT2*speed*10*sin(direction*PI/180-PI/4);
	m_MOTSDK.SetDcMotorControlMode (0,M_VELOCITY);
	m_MOTSDK.SetDcMotorControlMode (1,M_VELOCITY);
	m_MOTSDK.SetDcMotorVelocityControlPID (0, 10, 3, 100);
	m_MOTSDK.SetDcMotorVelocityControlPID (1, 10, 3, 100);
	m_MOTSDK.DcMotorVelocityNonTimeCtrAll (left, right,NO_CONTROL,NO_CONTROL,NO_CONTROL,NO_CONTROL); 
}

void CFollowMEDlg::OnBnClickedButtonMove()
{
	// TODO: Add your control notification handler code here
	RobotMove(m_direction, m_speed);
}

void CFollowMEDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	if (pScrollBar->GetDlgCtrlID()==IDC_SLIDER_DIRECTION)
	{
		m_direction=m_slider_direction.GetPos();
		char str[80];
		sprintf(str, "%d", m_direction);
		m_edit_direction.SetWindowTextA(str);
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFollowMEDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	if (pScrollBar->GetDlgCtrlID()==IDC_SLIDER_SPEED)
	{
		m_speed=m_slider_speed.GetPos();
		char str[80];
		sprintf(str, "%d", m_speed);
		m_edit_speed.SetWindowTextA(str);
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

// we will take a snapshot once the user have clicked the image view
void CFollowMEDlg::OnStnClickedImageView()
{
	// TODO: Add your control notification handler code here
	CTime theTime; 
	theTime=CTime::GetCurrentTime();
	m_VitCtrl.SaveSnapshot(2, theTime.Format("%Y-%M-%d-%H-%M-%S.bmp"));
	m_VitCtrl.SavePresetPosition("followme5");
}

// we will save the location of the camera
void CFollowMEDlg::OnClickVitaminctrl1(long lX, long lY)
{
	// TODO: Add your message handler code here
}

// this function implements pedstraint detection
UINT PedestrainThreadFunction(LPVOID pParam)
{
	PEDESTRAINTHREADPARAM *param=(PEDESTRAINTHREADPARAM *)pParam;
	CFollowMEDlg *dlg=(CFollowMEDlg *)param->window;
	param->is_processing=true;

	// grab frame from the camera
	VARIANT vData, vInfo;
	if (dlg->m_VitCtrl.GetSnapshot(2, &vData, &vInfo)!=S_OK)
	{
		// failed
		param->is_processing=false;	
		return -1;
	}
	// get the data from the buffer
	// we need to check the structure of variant
	IplImage *frame;
	dlg->Variant2IpplImage(vData, vInfo, frame);
	// release this buffer
	VariantClear(&vData);    // must release the buffer
	VariantClear(&vInfo);    // must release the buffer

	// perform the detection
	std::vector<CPedestrainRect> results=DetectHuman(frame,*(param->scanner));

	// send the command
	dlg->TrackPedestrain(results, frame);

	// save the result
	param->results.clear();
	for (int i=0; i<results.size(); i++)
	{
		param->results.push_back(results[i]);
	}

	// show the detection result
	for(unsigned int i=0;i<results.size();i++)
        cvRectangle(frame,cvPoint(results[i].left,results[i].top),cvPoint(results[i].right,results[i].bottom),CV_RGB(255,0,0),2);
	dlg->ShowImage(frame, IDC_Image_View);

	cvReleaseImage(&frame);
	param->is_processing=false;	
	return 0;
}
// this function computes the distance and orientation required for the robot
void CFollowMEDlg::TrackPedestrain(std::vector<CPedestrainRect> target, IplImage *frame)
{
	// we will utilize the location of the windows to decide the movement of the robot
	int width=frame->width;
	int height=frame->height;
	// currently we are only considering one pedestrain in view
	if (target.size()>0)
	{
		// we use the size of window to predict the distance
		double distance=59000.0/(target[0].right-target[0].left)/(target[0].bottom-target[0].top);
		// avoid the false detection
		if (distance>13)
		{
			return;
		}
		// the direction is related to the deviation of window center to the frame center
		double deviation=(target[0].left+target[0].right-width)/75;
		int direction=0;
		if (distance>=7)
		{
			// we need to move forward
			// asin returns value in [-pi/2,pi/2]
			direction=(int)(atan(deviation/7)*180/PI);
			if (direction<0)
			{
				direction=360+direction;
			}
		}
		else
		{
			// we need to move backward
			direction=180-(int)(atan(deviation/7)*180/PI);
		}
		// compute the moving time
		int duration=(int) (abs(distance-7)*2000/m_speed);
		// run the command
		RobotMoveTime(direction, m_speed, duration);
	}
}

// this function controls the movement of robot precisely
// duration: the duration of movement in millisecond
void CFollowMEDlg::RobotMoveTime(int direction, int speed, int duration)
{
	// compute the speed for each wheel
	short left=(short) SQRT2*speed*10*sin(direction*PI/180+PI/4);
	short right=(short) SQRT2*speed*10*sin(direction*PI/180-PI/4);
	m_MOTSDK.SetDcMotorControlMode (0,M_VELOCITY);
	m_MOTSDK.SetDcMotorControlMode (1,M_VELOCITY);
	m_MOTSDK.SetDcMotorVelocityControlPID (0, 10, 3, 100);
	m_MOTSDK.SetDcMotorVelocityControlPID (1, 10, 3, 100);
	m_MOTSDK.DcMotorVelocityTimeCtrAll (left, right,NO_CONTROL,NO_CONTROL,NO_CONTROL,NO_CONTROL, (short)duration); 
}
