// ip_hub.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "ip_hub.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>

#pragma comment(lib, "Ws2_32.lib")

// ���� �������
#define SERV_PORT 5000

struct SOCK_RECORD {
	// ����� �������
	SOCKET srv_socket;
	// ����� ��������������� ������
	int acc_sin_len;
	// ����� ��������������� ������
	SOCKADDR_IN acc_sin;
	// ����� �������
	SOCKADDR_IN dest_sin;
};
SOCK_RECORD s1,s2;

#define MAX_LOADSTRING 100

// ���������� ����������:
HINSTANCE hInst;								// ������� ���������
TCHAR szTitle[MAX_LOADSTRING];					// ����� ������ ���������
TCHAR szWindowClass[MAX_LOADSTRING];			// ��� ������ �������� ����

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void ServerStart(HWND, SOCKET *, int, int);
void ServerStop(HWND, SOCKET *);

// ��������� ��������� WSA_ACCEPT
void WndProc_OnWSAAccept(HWND, UINT, WPARAM, LPARAM, SOCK_RECORD *);
// ��������� ��������� WSA_NETEVENT
void WndProc_OnWSANetEvent_TCP(HWND, UINT, WPARAM, LPARAM);
void WndProc_OnWSANetEvent_UDP(HWND, UINT, WPARAM, LPARAM);

void SetConnection(HWND, SOCK_RECORD *, int);
void SendMsg(HWND, SOCK_RECORD *, int);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ���������� ��� �����.
	WSADATA WSAData;
	int rc = WSAStartup(MAKEWORD(1, 1), &WSAData);
	if(rc != 0)
	{
		MessageBox(NULL,L"WSAStartup Error", L"Error",MB_ICONERROR);
		return FALSE;
	}

	MSG msg;
	HACCEL hAccelTable;

	// ������������� ���������� �����
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IP_HUB, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ��������� ������������� ����������:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IP_HUB));

	// ���� ��������� ���������:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �������: MyRegisterClass()
//
//  ����������: ������������ ����� ����.
//
//  �����������:
//
//    ��� ������� � �� ������������� ���������� ������ � ������, ���� �����, ����� ������ ���
//    ��� ��������� � ��������� Win32, �� �������� ������� RegisterClassEx'
//    ������� ���� ��������� � Windows 95. ����� ���� ������� ����� ��� ����,
//    ����� ���������� �������� "������������" ������ ������ � ���������� �����
//    � ����.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IP_HUB));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_IP_HUB);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �������: InitInstance(HINSTANCE, int)
//
//   ����������: ��������� ��������� ���������� � ������� ������� ����.
//
//   �����������:
//
//        � ������ ������� ���������� ���������� ����������� � ���������� ����������, � �����
//        ��������� � ��������� �� ����� ������� ���� ���������.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��������� ���������� ���������� � ���������� ����������

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  �������: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����������:  ������������ ��������� � ������� ����.
//
//  WM_COMMAND	- ��������� ���� ����������
//  WM_PAINT	-��������� ������� ����
//  WM_DESTROY	 - ������ ��������� � ������ � ���������.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	// �������� ���������� ��������� WSA_ACCEPT
    case WSA_ACCEPT1:
		WndProc_OnWSAAccept(hWnd, message, wParam, lParam, &s1);
		break;
    case WSA_ACCEPT2:
		WndProc_OnWSAAccept(hWnd, message, wParam, lParam, &s2);
		break;
	// �������� ���������� ��������� WSA_NETEVENT
    case WSA_NETEVENT_TCP:
		WndProc_OnWSANetEvent_TCP(hWnd, message, wParam, lParam);
		break;
    case WSA_NETEVENT_UDP:
		WndProc_OnWSANetEvent_UDP(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// ��������� ����� � ����:
		switch (wmId)
		{
		case IDM_TCP_START1:
			ServerStart(hWnd, &s1.srv_socket, SOCK_STREAM, 1);
			break;
		case IDM_TCP_START2:
			ServerStart(hWnd, &s2.srv_socket, SOCK_STREAM, 2);
			break;
		case IDM_TCP_STOP1:
			ServerStop(hWnd, &s1.srv_socket);
			break;
		case IDM_TCP_STOP2:
			ServerStop(hWnd, &s2.srv_socket);
			break;
		case IDM_TCP_CONNECT1:
			SetConnection(hWnd, &s1, SOCK_STREAM);
			break;
		case IDM_TCP_CONNECT2:
			SetConnection(hWnd, &s2, SOCK_STREAM);
			break;
		case IDM_UDP_START1:
			ServerStart(hWnd, &s1.srv_socket, SOCK_DGRAM, 1);
			break;
		case IDM_UDP_START2:
			ServerStart(hWnd, &s2.srv_socket, SOCK_DGRAM, 2);
			break;
		case IDM_UDP_STOP1:
			ServerStop(hWnd, &s1.srv_socket);
			break;
		case IDM_UDP_STOP2:
			ServerStop(hWnd, &s2.srv_socket);
			break;
		case IDM_UDP_CONNECT1:
			SetConnection(hWnd, &s1, SOCK_DGRAM);
			break;
		case IDM_UDP_CONNECT2:
			SetConnection(hWnd, &s2, SOCK_DGRAM);
			break;
		case IDM_TCP_SEND1:
			SendMsg(hWnd, &s1, SOCK_STREAM);
			break;
		case IDM_TCP_SEND2:
			SendMsg(hWnd, &s2, SOCK_STREAM);
			break;
		case IDM_UDP_SEND1:
			SendMsg(hWnd, &s1, SOCK_DGRAM);
			break;
		case IDM_UDP_SEND2:
			SendMsg(hWnd, &s2, SOCK_DGRAM);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: �������� ����� ��� ���������...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		WSACleanup();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���������� ��������� ��� ���� "� ���������".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			for(int i=0;i<100;i++)
				SendMsg(hDlg, &s1, SOCK_DGRAM);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// -----------------------------------------------------
// ������� WndProc_OnWSAAccept
// -----------------------------------------------------
void WndProc_OnWSAAccept(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, SOCK_RECORD *sr)
{
	int rc;

	// ��� ������ �������� ����������� ���������
	// � ������� ���� ����������
	if(WSAGETSELECTERROR(lParam) != 0)
	{
		MessageBox(NULL, L"accept Error", L"Error", MB_OK);
		WSAAsyncSelect((*sr).srv_socket, hWnd, 0, 0);
		return;
	}

	// ���������� ������ ������ ������
	(*sr).acc_sin_len = sizeof((*sr).acc_sin);

	// ��������� ��������� ����������
	(*sr).srv_socket = accept((*sr).srv_socket, (LPSOCKADDR)&((*sr).acc_sin),
    (int FAR *)&((*sr).acc_sin_len));

	if((*sr).srv_socket == INVALID_SOCKET)
	{
		MessageBox(NULL, L"accept Error, invalid socket",L"Error", MB_OK);
		return;
	}

	// ���� �� ������ ������ �������� �������� ������ ��
	// �������, � ������� ���� ���������� �������� ��������� WSA_NETEVENT.
	// ��� �� ��������� �������� ��� ������� ����������
	rc = WSAAsyncSelect((*sr).srv_socket, hWnd, WSA_NETEVENT_TCP, FD_READ | FD_CLOSE);
	if(rc > 0)
	{
		closesocket((*sr).srv_socket);
		MessageBox(NULL, L"WSAAsyncSelect Error", L"Error", MB_OK);
	    return;
	}
}

// -----------------------------------------------------
// ������� WndProc_OnWSANetEvent
// -----------------------------------------------------

void WndProc_OnWSANetEvent_TCP(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char szTemp[256];
	int rc;

	// ���� �� ������ ����������� �������� ������,
	// ��������� � ���������� ��� ������ � ����
	// ��������� ������
	if(WSAGETSELECTEVENT(lParam) == FD_READ)
	{
		// ��������� ������
	    rc = recv((SOCKET)wParam, szTemp, 256, 0);
		if(rc)
		{
			szTemp[rc] = '\0';
			MessageBox(NULL, (LPCWSTR)(szTemp), L"Re�eived data", MB_OK);
		}
		return;
	}
  
	// ���� ���������� ���������, ������� �� ���� ���������
	else if(WSAGETSELECTEVENT(lParam) == FD_CLOSE)
	{
		MessageBox(NULL, L"Connection closed", L"Server", MB_OK);
	}
}
	
// -----------------------------------------------------
// ������� WndProc_OnWSANetEvent
// -----------------------------------------------------
void WndProc_OnWSANetEvent_UDP(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char szTemp[256];
	int rc;
	SOCKADDR_IN addr;
	int nAddrSize;
	char szBuf[80];
	LPSTR lpAddr;

	if(WSAGETSELECTEVENT(lParam) == FD_READ)
	{
		// ��������� ������
		rc = recvfrom((SOCKET)wParam, szTemp, 256, 0,
		(PSOCKADDR)&addr, &nAddrSize);
//		rc=WSAGetLastError();
		if(rc>0)
		{
			szTemp[rc] = '\0';
			strcpy_s(szBuf, "Received from ");
			// ��������������� ����� IP ���������� �������
			// � ��������� ������
			lpAddr = inet_ntoa(addr.sin_addr);
			strcat_s(szBuf, lpAddr);

			// ���������� ����� ���������� �������
			// � ���������� �� ���� ������
			MessageBox(NULL,(LPCWSTR) szTemp,(LPCWSTR) szBuf, MB_OK);
		}
		return;
	}
}

void ServerStop(HWND hWnd, SOCKET *srv_socket)
{
	// �������� ������ ����� ��������� � ������� �������
	// ���� ��� ������������� ����� �������, ���������
	// � �������� Windows Sockets
	WSAAsyncSelect(*srv_socket, hWnd, 0, 0);

	// ���� ����� ��� ������, ��������� ���
	if(*srv_socket != INVALID_SOCKET)
	{
		closesocket(*srv_socket);
	};
}

void ServerStart(HWND hWnd, SOCKET *srv_socket, int protocol, int num)
{
	struct sockaddr_in srv_address;
	int rc;
  
	// ������� ����� ������� ��� ������ � ������� ������
	*srv_socket = socket(AF_INET, protocol, 0);
	if(*srv_socket == INVALID_SOCKET)
	{
		MessageBox(NULL, L"socket Error", L"Error", MB_OK);
		return;
	}

	// ������������� ����� IP � ����� �����
	srv_address.sin_family = AF_INET;
	srv_address.sin_addr.s_addr = INADDR_ANY;
	srv_address.sin_port = htons(SERV_PORT);

	// ��������� ����� IP � �������  
	if(bind(*srv_socket, (LPSOCKADDR)&srv_address, 
		sizeof(srv_address)) == SOCKET_ERROR)
	{
		// ��� ������ ��������� �����
		closesocket(*srv_socket);
		MessageBox(NULL, L"bind Error", L"Error", MB_OK);
		return;
	}

	if(protocol==SOCK_STREAM){
		// ������������� ����� � ����� ������ ���
		// ���������� �������� ���������� � ��������
		if(listen(*srv_socket, 1) == SOCKET_ERROR)
		{
			closesocket(*srv_socket);
			MessageBox(NULL, L"listen Error", L"Error", MB_OK);
			return;
		}

		// ��� ������� ��������� ���������� ������� ���� ����������
		// ������� ��������� WSA_ACCEPT
		rc = WSAAsyncSelect(*srv_socket, hWnd, (num==1)?WSA_ACCEPT1:WSA_ACCEPT2, FD_ACCEPT);
	}
	else {
		// ���� �� ������ ������ �������� �������� ������ ��
		// �������, � ������� ���� ���������� �������� 
		// ��������� WSA_NETEVENT.
		rc = WSAAsyncSelect(*srv_socket, hWnd, WSA_NETEVENT_UDP, FD_READ);
	}

	if(rc > 0)
	{
		closesocket(*srv_socket);
		MessageBox(NULL, L"WSAAsyncSelect Error", L"Error", MB_OK);
		return;
	}
}

// -----------------------------------------------------
// ������� SetConnection
// -----------------------------------------------------
void SetConnection(HWND hWnd, SOCK_RECORD *sr, int protocol)
{
	PHOSTENT phe;

	// ������� ����� 
	(*sr).srv_socket = socket(AF_INET, protocol, 0);
	if((*sr).srv_socket == INVALID_SOCKET)
	{
		MessageBox(NULL, L"socket Error", L"Error", MB_OK);
		return;
	}

	// ��������� ����� IP � �������  
	(*sr).dest_sin.sin_family = AF_INET;
	(*sr).dest_sin.sin_addr.s_addr = INADDR_ANY;
	(*sr).dest_sin.sin_port = 0;

	if(bind((*sr).srv_socket, (LPSOCKADDR)&((*sr).dest_sin), 
		sizeof(((*sr).dest_sin))) == SOCKET_ERROR)
	{
		// ��� ������ ��������� �����
	    closesocket((*sr).srv_socket);
		MessageBox(NULL, L"bind Error", L"Error", MB_OK);
		return;
	}

	// ������������� ����� IP � ����� �����
	(*sr).dest_sin.sin_family = AF_INET;

	// ���������� ����� ����

	// ����� ���������� ���� ��� �������
	phe = gethostbyname("localhost");
  
	// ����� ���������� ����
	//phe = gethostbyname("maxsinev");
  
	if(phe == NULL)
	{
		closesocket((*sr).srv_socket);
		MessageBox(NULL, L"gethostbyname Error", L"Error", MB_OK);
	    return;
	}

	// �������� ����� ����
	memcpy((char FAR *)&((*sr).dest_sin.sin_addr), phe->h_addr,
	  phe->h_length);

	// ������ ������ �������� ������ ����
	//  (*sr).dest_sin.sin_addr.s_addr = inet_addr("200.200.200.201");

	// �������� ����� �����
	(*sr).dest_sin.sin_port = htons(SERV_PORT);

	if(protocol==SOCK_STREAM)
	{
		// ������������� ����������
		if(connect((*sr).srv_socket, (PSOCKADDR)&((*sr).dest_sin), 
			sizeof(((*sr).dest_sin))) < 0)
		{
			closesocket((*sr).srv_socket);
			MessageBox(NULL, L"connect Error", L"Error", MB_OK);
			return;
		}
	}
}

// -----------------------------------------------------
// ������� SendMsg
// -----------------------------------------------------

void SendMsg(HWND hWnd, SOCK_RECORD *sr, int protocol)
{
	struct {
		uint32_t counter;
		uint32_t mask;
		uint32_t charge;
	} m1;
	float quats[4];
	char szBuf[80];
	m1.counter=77;
	m1.mask=9;
	m1.charge=33;

	quats[0]=(float)0.1;
	quats[1]=(float)0.2;
	quats[2]=(float)0.3;
	quats[3]=(float)0.4;

//	strcpy_s(szBuf, "Test string");
	memcpy(&szBuf,&m1,sizeof(m1));
	memcpy(&szBuf[sizeof(m1)],quats,sizeof(quats));

	quats[0]=(float)0.4;
	quats[1]=(float)0.3;
	quats[2]=(float)0.2;
	quats[3]=(float)0.1;

	memcpy(&szBuf[sizeof(m1)+sizeof(quats)],quats,sizeof(quats));

	// �������� ���������
	if(protocol==SOCK_STREAM)
	{
//		send((*sr).srv_socket, szBuf, strlen(szBuf), 0);
		send((*sr).srv_socket, szBuf, sizeof(m1)+sizeof(quats)+sizeof(quats), 0);
	}
	else
	{
//		sendto((*sr).srv_socket, szBuf, strlen(szBuf), 0,
		sendto((*sr).srv_socket, szBuf, sizeof(m1)+sizeof(quats)+sizeof(quats), 0,
	    (PSOCKADDR)&((*sr).dest_sin), sizeof((*sr).dest_sin));
	}
}