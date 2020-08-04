#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>
#include<math.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

void graph(void);
void ToggleFullScreen(void);
void draw_hyperbola(void);
void draw_finalA(void);

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
		TEXT("Dynamic INDIA"), // Title of window
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
		/*default:
			gbFullScreen = FALSE;
			ToggleFullScreen();
			break;*/
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
	ToggleFullScreen();  // autoenable fullscreen
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

void hyperbola()
{
	//co-ordinates of mid point of curve
	GLfloat midY = 0;
	GLfloat midX = 0.0;

	GLfloat y = 0;

	glBegin(GL_POINTS);
	//up-ward parabola
	for (GLfloat x = -30.0; x <= 30.5; x += 0.0005) // end co-ordinates of curves
	{
		if (x > 0.0f)
		{
			y = x * x;
			glColor3f(1.0f, 0.5f, 0.0f); // orange colour
			glVertex2f(midX + x, midY + y);
			glColor3f(0.0f, 0.5f, 0.0f); // green colour
			glVertex2f(midX + x, -(midY + y));
		}
		else
		{
			glVertex2f(midX + x, midY + y);
			glVertex2f(midX + x, -(midY + y));
		}
	}
	glEnd();
}

void PMS_display(void)
{

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	static float x_pos = -20.0f;
	static float x2_pos = -3.0f;

	x2_pos = x2_pos + 0.001f;
	if (x2_pos >= 5.0f)
	{
		x2_pos = x2_pos;
	}

	x_pos = x_pos + 0.001f;
	if (x_pos >= 5.0f)
	{
		x_pos = x_pos;
	}
	
	static float angle = 0.0f;
	static float I_xpos = -3.0f;
	static float N_ypos = 5.0f;

	// orange colour
	static float R_val = -2.3f;
	static float G_val = -0.7f;
	static float B_val = -0.5f;
	// green colour
	static float G2_val = 0.0f;

	static float I_ypos = -7.0f;
	static float A_xpos = 4.0f;

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	//graph();
	// I
	glLoadIdentity();
	glTranslatef(I_xpos, 0, -3.0f);
	//glTranslatef(-1.5f, 0.0f, -3.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);   // 1st vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glEnd();

	// N
	glLoadIdentity();     // |
	//glTranslatef(-1.2f, 0.0f, -3.0f);
	glTranslatef(-1.2f, N_ypos, -3.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);  // 1st vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);   // 3rd vertex anticlockwise


	glColor3f(1.0f, 0.5f, 0.0f);    //   \              /
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);  // 1st vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.5f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.6f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glEnd();

	glLoadIdentity();           //  |
	//glTranslatef(-0.7f, 0.0f, -3.0f); 
	glTranslatef(-0.7f, N_ypos, -3.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);  // 1st vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glEnd();

	// D
	glLoadIdentity();
	glTranslatef(-0.4f, 0.0f, -3.0f);
	glBegin(GL_QUADS);
	//glColor3f(1.0f, 0.5f, 0.0f);  // 1st vertical line of D
	glColor3f(R_val, G_val, B_val);
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);  // 1st vertex anticlockwise
	//glColor3f(0.0f, 0.5f, 0.0f);
	glColor3f(0.0f, G_val, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glEnd();

	glLoadIdentity();           //  |
	glTranslatef(0.1f, 0.0f, -3.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);   // top horizontal line of D
	glColor3f(R_val, G_val, B_val);
	glVertex3f(-0.6f, 0.5f, 0.0f);  // 1st vertex anticlockwise
	glVertex3f(-0.6f, 0.4f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, 0.4f, 0.0f);  // 3rd vertex anticlockwise
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise

	//glColor3f(0.0f, 0.5f, 0.0f);  // bottom horizontal line of D
	glColor3f(0.0f, G_val, 0.0f);
	glVertex3f(-0.6f, -0.4f, 0.0f);  // 3rd vertex anticlockwise
	glVertex3f(-0.6f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glVertex3f(0.1f, -0.4f, 0.0f);  // 3rd vertex anticlockwise

	//glColor3f(1.0f, 0.5f, 0.0f);  // 2nd vertical line of D
	glColor3f(R_val, G_val, B_val);
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);  // 1st vertex anticlockwise
	//glColor3f(0.0f, 0.5f, 0.0f);
	glColor3f(0.0f, G_val, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glEnd();

	// draw I
	glLoadIdentity();
	//glTranslatef(0.4f, 0.0f, -3.0f);
	glTranslatef(0.4f, I_ypos, -3.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.1f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glVertex3f(0.0f, 0.5f, 0.0f);   // 1st vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 2nd vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glEnd();

	glLoadIdentity();           //  A
	glTranslatef(A_xpos, 0.0f, -3.0f);
	draw_finalA();


	// last airplane flag
	if (R_val == 1.0f && G_val == 0.5f && B_val == 0.0f)
	{
		glLineWidth(10.0f);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0f, 0.5f, 0.0f);
		// orange horizontal line
		glVertex2f(x_pos, -0.018);
		glVertex2f(x2_pos, -0.018);
		glEnd();

		glLineWidth(10.0f);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0f, 1.0f, 1.0f);
		// white horizontal line
		glVertex2f(x_pos, -0.039f);
		glVertex2f(x2_pos, -0.039f);
		glEnd();

		glLineWidth(10.0f);
		glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.5f, 0.0f);
		// Green horizontal line
		glVertex2f(x_pos, -0.059f);
		glVertex2f(x2_pos, -0.059f);
		glEnd();

		if (x_pos == 0.0f)
		{
			//co-ordinates of mid point of curve
			GLfloat midY = 0;
			GLfloat midX = 0.0;

			GLfloat y = 0;

			glBegin(GL_POINTS);
			//up-ward parabola
			for (GLfloat x = -30.0; x <= 30.5; x += 0.0005) // end co-ordinates of curves
			{
				if (x > 0.0f)
				{
					y = x * x;
					glColor3f(1.0f, 0.5f, 0.0f); // orange colour
					glVertex2f(midX + x, midY + y);
					glColor3f(0.0f, 0.5f, 0.0f); // green colour
					glVertex2f(midX + x, -(midY + y));
				}
				/*else
				{
					glVertex2f(midX + x, midY + y);
					glVertex2f(midX + x, -(midY + y));
				}*/
			}
			glEnd();
		}
	}
	
	/*
	// central white line
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(x_pos, 0.0f);
	glVertex2f(x2_pos, 0.0f);
	glEnd();
	*/
	SwapBuffers(PMS_ghdc);
	
	I_xpos = I_xpos + 0.01f;
	if (I_xpos >= -1.5f)
	{
		I_xpos = -1.5f;
	}

	N_ypos = N_ypos - 0.01f;
	if (N_ypos <= 0.0f)
	{
		N_ypos = 0.0f;
	}

	// D's orange colour
	R_val = R_val + 0.003f;
	if (R_val >= 1.0f)
	{
		R_val = 1.0f;
	}
	G_val = G_val + 0.001f;
	if (G_val >= 0.5f)
	{
		G_val = 0.5f;
	}
	B_val = B_val + 0.01f;
	if (B_val >= 0.0f)
	{
		B_val = 0.0f;
	}

	I_ypos = I_ypos + 0.01f;
	if (I_ypos >= 0.0f)
	{
		I_ypos = 0.0f;
	}

	A_xpos = A_xpos - 0.01f;
	if (A_xpos <= 0.8f)
	{
		A_xpos = 0.7f;
	}

	if (R_val == 1.0f && G_val == 0.5f && B_val == 0.0f) 
	{
		x2_pos = x2_pos + 0.001f;
		if (x2_pos >= 5.0f)
		{
			x2_pos = x2_pos;
		}
	}
	
}

void draw_finalA(void)
{
	//glLoadIdentity();           //  A
	//glTranslatef(0.6f, 0.0f, -3.0f);
	glBegin(GL_QUADS);
	/*
	// flag tri colour
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glVertex3f(0.2f, -0.05f, 0.0f);
	glVertex3f(0.6f, -0.05f, 0.0f);
	glVertex3f(0.6f, 0.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, -0.03f, 0.0f);
	glVertex3f(0.2f, -0.1f, 0.0f);
	glVertex3f(0.6f, -0.1f, 0.0f);
	glVertex3f(0.6f, -0.03f, 0.0f);

	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.2f, -0.06f, 0.0f);
	glVertex3f(0.2f, -0.1f, 0.0f);
	glVertex3f(0.6f, -0.1f, 0.0f);
	glVertex3f(0.6f, -0.06f, 0.0f);
	*/
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.4f, 0.3f, 0.0f);  // 3rd vertex anticlockwise
	glVertex3f(0.4f, 0.5f, 0.0f);  // 4th vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);  // 1st vertex anticlockwise
	glVertex3f(0.1f, -0.5f, 0.0f);  // 2nd vertex anticlockwise

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.4f, 0.3f, 0.0f);  // 1st vertex anticlockwise
	glVertex3f(0.4f, 0.5f, 0.0f);  // 2nd vertex anticlockwise
	glColor3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.8f, -0.5f, 0.0f);  // 3rd vertex anticlockwise
	glVertex3f(0.7f, -0.5f, 0.0f);  // 4th vertex anticlockwise
	glEnd();

}
void graph(void)
{
	// draw graph

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	for (float y = -1.5f; y <= 1.6f; y = y + 0.1f) //draw horizontal lines
	{
		glVertex2f(1.5f, y);
		glVertex2f(-1.5f, y);
	}

	for (float x = -1.5f; x <= 1.6f; x = x + 0.1f)   //draw vertical lines
	{
		glVertex2f(x, 1.5f);
		glVertex2f(x, -1.5f);
	}
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);   // x-axis coloured
	glVertex2f(1.5f, 0.0f);
	glVertex2f(-1.5f, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);  // y-axis coloured
	glVertex2f(0.0f, 1.5f);
	glVertex2f(0.0f, -1.5f);
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
