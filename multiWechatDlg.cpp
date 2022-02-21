﻿
// multiWechatDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "multiWechat.h"
#include "multiWechatDlg.h"
#include "afxdialogex.h"
#include <aclapi.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmultiWechatDlg 对话框



CmultiWechatDlg::CmultiWechatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MULTIWECHAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_WECHAT);
}

void CmultiWechatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmultiWechatDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_STAR, &CmultiWechatDlg::StartWechat)
	ON_WM_DESTROY()
	ON_COMMAND(ID_ALL_EXIT, &CmultiWechatDlg::OnAllExit)
END_MESSAGE_MAP()


// CmultiWechatDlg 消息处理程序

void EnableMultiWeChat()
{
	HANDLE hMutex = CreateMutexW(NULL, FALSE, L"_WeChat_App_Instance_Identity_Mutex_Name");
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	PSID pEveryoneSID = NULL;
	char szBuffer[4096] = {0};
	PACL pAcl = (PACL)szBuffer;

	AllocateAndInitializeSid(
		&SIDAuthWorld,
		1,
		SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pEveryoneSID);

	InitializeAcl(pAcl, sizeof(szBuffer), ACL_REVISION);
	AddAccessDeniedAce(pAcl, ACL_REVISION, MUTEX_ALL_ACCESS, pEveryoneSID);
	SetSecurityInfo(hMutex, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pAcl, NULL);
}

void CmultiWechatDlg::LoadWechatPath()
{
	HKEY hKey = nullptr;
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WeChat"), 0, KEY_READ, &hKey))
	{
		AfxMessageBox(TEXT("打开注册表失败!"));
		return;
	}

	DWORD dwTpye = 0;
	DWORD dwValue = MAX_PATH;
	TCHAR szWechatPath[MAX_PATH + 1] = { 0 };
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, TEXT("DisplayIcon"), 0, &dwTpye, (LPBYTE)szWechatPath, &dwValue))
	{
		CloseHandle(hKey);
		AfxMessageBox(TEXT("读取注册表失败!"));
		return;
	}
	CloseHandle(hKey);
	
	m_strWechatPath = szWechatPath;
	m_strWechatPath.TrimLeft(L"\"");
	m_strWechatPath.TrimRight(L"\"");
}

void CmultiWechatDlg::StartWechat()
{	
	if (!PathFileExists(m_strWechatPath)) {
		AfxMessageBox(TEXT("本机微信未安装!"));
	}
	else {
		ShellExecute(NULL, TEXT("open"), m_strWechatPath, NULL, NULL, SW_SHOWNORMAL);
	}
}

BOOL CmultiWechatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

	EnableMultiWeChat();

	LoadWechatPath();
	//StartWechat();

	m_notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_notifyIcon.hIcon = m_hIcon;
	m_notifyIcon.hWnd = m_hWnd;
	lstrcpy(m_notifyIcon.szTip, _T("微信多开小助手"));
	m_notifyIcon.uCallbackMessage = WM_SYSTEMTRAY;
	m_notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, &m_notifyIcon);

	//ShowWindow(SW_HIDE);
	AfxGetApp()->m_pMainWnd->ShowWindow(SW_HIDE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmultiWechatDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmultiWechatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CmultiWechatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CmultiWechatDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	Shell_NotifyIcon(NIM_DELETE, &m_notifyIcon);
}

LRESULT CmultiWechatDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (message) {
	case WM_SYSTEMTRAY:
		if (lParam == WM_LBUTTONDBLCLK)
		{
			if (AfxGetApp()->m_pMainWnd->IsWindowVisible())
			{
				AfxGetApp()->m_pMainWnd->ShowWindow(SW_HIDE);
			}
			else
			{
				AfxGetApp()->m_pMainWnd->ShowWindow(SW_SHOW);
			}
		}
		else if (lParam == WM_RBUTTONDOWN)
		{
			CMenu menu;
			menu.LoadMenu(IDR_MENU_EXIT);
			CMenu* pMenu = menu.GetSubMenu(0);
			CPoint pos;
			GetCursorPos(&pos);
			pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
		}
		break;
	case WM_DESTROY:
		::Shell_NotifyIcon(NIM_DELETE, &m_notifyIcon);
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_MINIMIZE)
		{
			AfxGetApp()->m_pMainWnd->ShowWindow(SW_HIDE);
			return 0;
		}
		break;
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}


void CmultiWechatDlg::OnAllExit()
{
	PostMessage(WM_QUIT, 0, 0);
}
