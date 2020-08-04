#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>
#include"template.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

GLuint Texture_Stone;
GLuint Texture_Kundali;
static float spin_angle = 0.0f;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL gbFullScreen = FALSE;
DWORD gdwSyle;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };
HDC ghdc = NULL;
HGLRC ghrc = NULL;
HWND ghwnd = NULL;
bool gbActive = false;
FILE* gpFile = NULL;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	int initialize(void);
	void display(void);
	int iRet = 0;
	bool bDone = false;

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");

	int width, height;

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can not be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "Log file is successfully created\n");
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
		TEXT("Texture Mapping"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		x,
		y,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	iRet = initialize();
	if (iRet == -1)
	{
		fprintf_s(gpFile, "ChoosePixelFormat failed\n");
		DestroyWindow(ghwnd);
	}
	else if (iRet == -2)
	{
		fprintf_s(gpFile, "SetPixelFormat failed\n");
		DestroyWindow(ghwnd);
	}
	else if (iRet == -3)
	{
		fprintf_s(gpFile, "wglCreateContext failed\n");
		DestroyWindow(ghwnd);
	}
	else if (iRet == -4)
	{
		fprintf_s(gpFile, "wglMakeCurrent failed\n");
		DestroyWindow(ghwnd);
	}
	else
	{
		fprintf_s(gpFile, "Initialization successful\n");
	}
	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(ghwnd);
	SetFocus(ghwnd);

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
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
			display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullScreen(void);
	void resize(int, int);
	void uninitialize(void);

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
		resize(LOWORD(lParam), HIWORD(lParam));
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
		uninitialize();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
	fprintf_s(gpFile, "In ToggleFullScreen().\n");
	MONITORINFO mInfo;

	if (gbFullScreen == FALSE)
	{
		gdwSyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (gdwSyle & WS_OVERLAPPEDWINDOW)
		{
			mInfo = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &gwpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mInfo))
			{
				SetWindowLong(ghwnd, GWL_STYLE, gdwSyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,
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
		SetWindowLong(ghwnd, GWL_STYLE, gdwSyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &gwpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
	fprintf_s(gpFile, "Out of ToggleFullScreen().\n");
}

BOOL LoadTexture(GLuint* texture, TCHAR ImageResourceID[])
{
	HBITMAP hBitmap = NULL;
	BITMAP bmp;
	BOOL bStatus = FALSE;
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ImageResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBitmap)
	{
		bStatus = TRUE;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
		DeleteObject(hBitmap);
	}
	return bStatus;
}

int initialize(void)
{
	fprintf_s(gpFile, "In initialize().\n");
	void resize(int, int);

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	//initialize pfd structure
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

	if (iPixelFormatIndex == 0)
	{
		return (-1);
	}
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return (-2);
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return (-3);
	}
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return (-4);
	}
	// add 3 call
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//for texture mapping
	glEnable(GL_TEXTURE_2D);
	LoadTexture(&Texture_Stone, MAKEINTRESOURCE(ID_STONE));
    LoadTexture(&Texture_Kundali, MAKEINTRESOURCE(ID_KUNDALI));
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	resize(WIN_WIDTH, WIN_HEIGHT);
	fprintf_s(gpFile, "Out of initialize().\n");
	return(0);
}

void resize(int width, int height)
{
	fprintf_s(gpFile, "In resize().\n");

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
	fprintf_s(gpFile, "Out of resize().\n");
}

void draw_cube()
{
	glBindTexture(GL_TEXTURE_2D, Texture_Kundali);
	glBegin(GL_QUADS);
	// Front face
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	// Right face
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// Back face
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);

	// Left face
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

	// Top face
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// Bottom face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();
}

void draw_pyramid()
{
	// trianle
	glBindTexture(GL_TEXTURE_2D, Texture_Stone);
	glBegin(GL_TRIANGLES);
	// Front face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);  // 1st vertex
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);  // 2nd vertex
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);  // 3rd vertex

	//Right face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);  // 1st vertex
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);  // 2nd vertex
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);  // 3rd vertex

	//Back face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);  // 1st vertex
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);  // 2nd vertex
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);  // 3rd vertex

	//Left face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);  // 1st vertex
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);  // 2nd vertex
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);  // 3rd vertex
	glEnd();
}

void update(void)
{
	spin_angle = spin_angle + 0.01f;
	if (spin_angle >= 360.0f)
	{
		spin_angle = 0.0f;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.0f, 0.0f, -4.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	glRotatef(spin_angle,1.0f,0.0f,0.0f);
	draw_cube();

	glLoadIdentity();
	glTranslatef(0.8f, 0.0f, -3.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	glRotatef(spin_angle, 0.0f, 1.0f, 0.0f);
	draw_pyramid();
	
	SwapBuffers(ghdc);
	update();
}

void uninitialize(void)
{
	fprintf_s(gpFile, "In uninitialize().\n");

	if (gbFullScreen == TRUE)
	{
		SetWindowLong(ghwnd, GWL_STYLE, gdwSyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &gwpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}
	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}
	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	if (gpFile)
	{
		fprintf_s(gpFile, "File closed successfully\n");
		fclose(gpFile);
		gpFile = NULL;
	}
	fprintf_s(gpFile, "Out of uninitialize().\n");

	//Remove allocated memory for texture
	glDeleteTextures(1, &Texture_Stone);
	glDeleteTextures(1, &Texture_Kundali);
}