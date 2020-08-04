#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

static float spin_angle = 0.0f;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL gbFullScreen = FALSE;
DWORD PMS_gdwSyle;
WINDOWPLACEMENT PMS_gwpPrev = { sizeof(WINDOWPLACEMENT) };
HDC PMS_ghdc = NULL;
HGLRC PMS_ghrc = NULL;
HWND PMS_ghwnd = NULL;
bool gbActive = false;
FILE* PMS_gpFile = NULL;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	int PMS_initialize(void);
	void PMS_display(void);
	int iRet = 0;
	bool PMS_bDone = false;

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");

	int width, height;

	if (fopen_s(&PMS_gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can not be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf_s(PMS_gpFile, "Log file is successfully created\n");
	}
	//Initialization of WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


	if (!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, TEXT("wndclass can not be registered."), TEXT("Error"), MB_OK);
		exit(0);
	}
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);
	int x = (width - 800) / 2;
	int y = (height - 600) / 2;

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("OGL - Perspective"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		x,
		y,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	PMS_ghwnd = hwnd;

	iRet = PMS_initialize();
	if (iRet == -1)
	{
		fprintf_s(PMS_gpFile, "ChoosePixelFormat failed\n");
		DestroyWindow(PMS_ghwnd);
	}
	else if (iRet == -2)
	{
		fprintf_s(PMS_gpFile, "SetPixelFormat failed\n");
		DestroyWindow(PMS_ghwnd);
	}
	else if (iRet == -3)
	{
		fprintf_s(PMS_gpFile, "wglCreateContext failed\n");
		DestroyWindow(PMS_ghwnd);
	}
	else if (iRet == -4)
	{
		fprintf_s(PMS_gpFile, "wglMakeCurrent failed\n");
		DestroyWindow(PMS_ghwnd);
	}
	else
	{
		fprintf_s(PMS_gpFile, "Initialization successful\n");
	}
	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(PMS_ghwnd);
	SetFocus(PMS_ghwnd);

	while (PMS_bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				PMS_bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActive == true)
			{
				//Here call update
			}
			//Here call display()
			PMS_display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullScreen(void);
	void PMS_resize(int, int);
	void PMS_uninitialize(void);

	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActive = true;
		break;
	case WM_KILLFOCUS:
		gbActive = false;
		break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		PMS_resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
			ToggleFullScreen();
			break;
		}
		break;
	case WM_DESTROY:
		PMS_uninitialize();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}



void ToggleFullScreen(void)
{
	fprintf_s(PMS_gpFile, "In ToggleFullScreen().\n");
	MONITORINFO mInfo;

	if (gbFullScreen == FALSE)
	{
		PMS_gdwSyle = GetWindowLong(PMS_ghwnd, GWL_STYLE);
		if (PMS_gdwSyle & WS_OVERLAPPEDWINDOW)
		{
			mInfo = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(PMS_ghwnd, &PMS_gwpPrev) &&
				GetMonitorInfo(MonitorFromWindow(PMS_ghwnd, MONITORINFOF_PRIMARY), &mInfo))
			{
				SetWindowLong(PMS_ghwnd, GWL_STYLE, PMS_gdwSyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(PMS_ghwnd,
					HWND_TOP,
					mInfo.rcMonitor.left,
					mInfo.rcMonitor.top,
					mInfo.rcMonitor.right - mInfo.rcMonitor.left,
					mInfo.rcMonitor.bottom - mInfo.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);

			}
		}
		ShowCursor(FALSE);
		gbFullScreen = TRUE;
	}
	else
	{
		SetWindowLong(PMS_ghwnd, GWL_STYLE, PMS_gdwSyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(PMS_ghwnd, &PMS_gwpPrev);
		SetWindowPos(PMS_ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
	fprintf_s(PMS_gpFile, "Out of ToggleFullScreen().\n");
}

int PMS_initialize(void)
{
	fprintf_s(PMS_gpFile, "In PMS_initialize().\n");
	void PMS_resize(int, int);

	PIXELFORMATDESCRIPTOR PMS_pfd;
	int PMS_iPixelFormatIndex;

	//initialize pfd structure
	ZeroMemory(&PMS_pfd, sizeof(PIXELFORMATDESCRIPTOR));

	PMS_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	PMS_pfd.nVersion = 1;
	PMS_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	PMS_pfd.iPixelType = PFD_TYPE_RGBA;
	PMS_pfd.cColorBits = 32;
	PMS_pfd.cRedBits = 8;
	PMS_pfd.cGreenBits = 8;
	PMS_pfd.cBlueBits = 8;
	PMS_pfd.cAlphaBits = 8;
	PMS_pfd.cDepthBits = 32;

	PMS_ghdc = GetDC(PMS_ghwnd);
	PMS_iPixelFormatIndex = ChoosePixelFormat(PMS_ghdc, &PMS_pfd);

	if (PMS_iPixelFormatIndex == 0)
	{
		return (-1);
	}
	if (SetPixelFormat(PMS_ghdc, PMS_iPixelFormatIndex, &PMS_pfd) == FALSE)
	{
		return (-2);
	}

	PMS_ghrc = wglCreateContext(PMS_ghdc);
	if (PMS_ghrc == NULL)
	{
		return (-3);
	}
	if (wglMakeCurrent(PMS_ghdc, PMS_ghrc) == FALSE)
	{
		return (-4);
	}
	// add 3 call
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	PMS_resize(WIN_WIDTH, WIN_HEIGHT);
	fprintf_s(PMS_gpFile, "Out of PMS_initialize().\n");
	return(0);
}

void PMS_resize(int width, int height)
{
	fprintf_s(PMS_gpFile, "In PMS_resize().\n");

	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,
		(GLfloat)width / (GLfloat)height,
		0.1f,
		100.0f);
	fprintf_s(PMS_gpFile, "Out of PMS_resize().\n");
}


void PMS_display(void)
{
	void update(void);
	void DrawCube(void);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);
	glScalef(0.5f, 0.5f, 0.5f);  // half the x,y,z of original side 
	glRotatef(spin_angle, 1.0f, 0.0f, 0.0f);
	glRotatef(spin_angle, 0.0f, 1.0f, 0.0f);
	glRotatef(spin_angle, 0.0f, 0.0f, 1.0f);

	DrawCube();

	SwapBuffers(PMS_ghdc);
	update();
}

void update(void)
{
	spin_angle = spin_angle + 0.1f;
	if (spin_angle >= 360.0f)
	{
		spin_angle = 0.0f;
	}
}

void DrawCube(void)
{
	glBegin(GL_QUADS);
	// Front face
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f,1.0f);
	glVertex3f(-1.0f, -1.0f,1.0f);
	glVertex3f(1.0f, -1.0f,1.0f);
	glVertex3f(1.0f, 1.0f,1.0f);

	// Right face
	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f,1.0f);
	glVertex3f(1.0f, -1.0f,1.0f);
	glVertex3f(1.0f, -1.0f,-1.0f);
	glVertex3f(1.0f, 1.0f,-1.0f);

	// Back face
	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f,-1.0f);
	glVertex3f(1.0f, -1.0f,-1.0f);
	glVertex3f(-1.0f, -1.0f,-1.0f);
	glVertex3f(-1.0f, 1.0f,-1.0f);

	// Left face
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

	// Top face
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// Bottom face
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();
}

void PMS_uninitialize(void)
{
	fprintf_s(PMS_gpFile, "In PMS_uninitialize().\n");

	if (gbFullScreen == TRUE)
	{
		SetWindowLong(PMS_ghwnd, GWL_STYLE, PMS_gdwSyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(PMS_ghwnd, &PMS_gwpPrev);
		SetWindowPos(PMS_ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}
	if (wglGetCurrentContext() == PMS_ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}
	if (PMS_ghrc)
	{
		wglDeleteContext(PMS_ghrc);
		PMS_ghrc = NULL;
	}
	if (PMS_ghdc)
	{
		ReleaseDC(PMS_ghwnd, PMS_ghdc);
		PMS_ghdc = NULL;
	}
	if (PMS_gpFile)
	{
		fprintf_s(PMS_gpFile, "File closed successfully\n");
		fclose(PMS_gpFile);
		PMS_gpFile = NULL;
	}
	fprintf_s(PMS_gpFile, "Out of PMS_uninitialize().\n");

}