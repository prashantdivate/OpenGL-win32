#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>
#include<math.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


#define WIN_WIDTH 800
#define WIN_HEIGHT 600
//#define PI 4.0
#define PI 3.141592653589793238f

void graph(void);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL gbFullScreen = FALSE;
DWORD PMS_gdwSyle;
WINDOWPLACEMENT PMS_gwpPrev = { sizeof(WINDOWPLACEMENT) };
HDC PMS_ghdc = NULL;
HGLRC PMS_ghrc = NULL;
HWND PMS_ghwnd = NULL;
bool gbActive = false;
FILE* PMS_gpFile = NULL;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine,
	int iCmdShow)
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
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // 3rd memory management- fixed,movable and discard memory. next we dont use windows DC don't discard my device context and keep it for next
	// dc context to rendering context
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
		TEXT("Graph"), // Title of window
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
	MONITORINFO mInfo;  // gets monitor info in mInfo

	if (gbFullScreen == FALSE)
	{
		PMS_gdwSyle = GetWindowLong(PMS_ghwnd, GWL_STYLE);
		if (PMS_gdwSyle & WS_OVERLAPPEDWINDOW)
		{
			mInfo = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(PMS_ghwnd, &PMS_gwpPrev) &&
				GetMonitorInfo(MonitorFromWindow(PMS_ghwnd, MONITORINFOF_PRIMARY), &mInfo))  // prev window placement co-ordinates stores in prev variable
			{
				SetWindowLong(PMS_ghwnd, GWL_STYLE, PMS_gdwSyle & ~WS_OVERLAPPEDWINDOW); // set window parameters but destroy prev co-ordinates
				SetWindowPos(PMS_ghwnd,
					HWND_TOP,
					mInfo.rcMonitor.left,
					mInfo.rcMonitor.top,
					mInfo.rcMonitor.right - mInfo.rcMonitor.left,
					mInfo.rcMonitor.bottom - mInfo.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);     // NOZORDER means if window is on top stay it on top

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
			0,   // assign default parameters here
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

	PIXELFORMATDESCRIPTOR PMS_pfd; // DC to rendering context conversion
	int PMS_iPixelFormatIndex;

	//initialize pfd structure
	ZeroMemory(&PMS_pfd, sizeof(PIXELFORMATDESCRIPTOR));  // var to zero

	PMS_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);  // size 
	PMS_pfd.nVersion = 1; // related to opengl version
	PMS_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // 1-draw context on window
	PMS_pfd.iPixelType = PFD_TYPE_RGBA;  // 
	PMS_pfd.cColorBits = 32;  //
	PMS_pfd.cRedBits = 8;
	PMS_pfd.cGreenBits = 8;
	PMS_pfd.cBlueBits = 8;
	PMS_pfd.cAlphaBits = 8;

	PMS_ghdc = GetDC(PMS_ghwnd); // this draw/paint in client area and non-client area gives gives DC
	PMS_iPixelFormatIndex = ChoosePixelFormat(PMS_ghdc, &PMS_pfd);  //pfd str send to choos 1-32 pixel formats madhe closeset match honara context return krto

	if (PMS_iPixelFormatIndex == 0)
	{
		return (-1);
	}
	if (SetPixelFormat(PMS_ghdc, PMS_iPixelFormatIndex, &PMS_pfd) == FALSE) // kapde applyala ghDC la chadvayche
	{
		return (-2);
	}

	PMS_ghrc = wglCreateContext(PMS_ghdc);  // actual dc pasun rendering context milel
	if (PMS_ghrc == NULL)
	{
		return (-3);
	}
	if (wglMakeCurrent(PMS_ghdc, PMS_ghrc) == FALSE) // use rc not dc globale handle
	{
		return (-4);
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	// draw graph
	graph();

	// draw circle
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0f, 1.0f, 0.0f);
	for (float i = 0.1f; i <= 2.0f * PI; i = i + 0.19f) 
	{
		glVertex2f(cos(i), sin(i));
		//glVertex2f(cos(i+0.19f), sin(i+0.19f));  // GL_LES 2nd vertex and not needed in LINELOOP
	}
	glEnd();

	SwapBuffers(PMS_ghdc);
}

void graph(void)
{
	// draw graph

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	for (float y = -1.0f; y <= 1.1f; y = y + 0.1f) //draw horizontal lines
	{
		glVertex2f(1.0f, y);
		glVertex2f(-1.0f, y);
	}

	for (float x = -1.0f; x <= 1.1f; x = x + 0.1f)   //draw vertical lines
	{
		glVertex2f(x, 1.0f);
		glVertex2f(x, -1.0f);
	}
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);   // x-axis coloured
	glVertex2f(1.0f, 0.0f);
	glVertex2f(-1.0f, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);  // y-axis coloured
	glVertex2f(0.0f, 1.0f);
	glVertex2f(0.0f, -1.0f);
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
		wglMakeCurrent(NULL, NULL);  // back to dc
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
