// Entry point for program

#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// TODO: This is a global for now
global_variable bool running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int xOffset, int yOffset)
{
	int Width = BitmapWidth;
	int Height = BitmapHeight;

	int Pitch = Width * BytesPerPixel;
	uint8 *Row = (uint8 *)BitmapMemory;
	for (int y = 0; y < BitmapHeight; ++y)
	{
		uint32 *pixel = (uint32 *)Row;
		for (int x = 0; x < BitmapWidth; ++x)
		{
			uint8 Blue = (x + xOffset);
			uint8 Green = (y + yOffset);
			uint8 Red = (y + yOffset);

			*pixel++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}

internal void Win32ResizeDIBSection(int Width, int Height)
{
	// TODO: Bulletproof this
	// Maybe don't free first, free after, then free first if that fails.

	if (BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = BytesPerPixel * (Width * Height);
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;
	StretchDIBits(DeviceContext, 0, 0, BitmapWidth, BitmapHeight, 0, 0, WindowWidth, WindowHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message) {
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			Win32ResizeDIBSection(Width, Height);
			OutputDebugStringA("WM_SIZE\n");
		} break;
		
		case WM_DESTROY:
		{
			// TODO: handle this as an error - recreate window?
			running = false;
		} break;
		
		case WM_CLOSE:
		{
			// TODO: handle this with a message to the user?
			running = false;
		} break;
		
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);

			Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
			EndPaint(Window, &Paint);
		} break;
		
		default:
		{
			// OutputDebugStringA("default\n");
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return Result;
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
	WNDCLASS WindowClass = {};

	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	// WindowClass.hIcon = ;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass)) {
		
		HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
		
		if (Window != NULL) {
			int xOffset = 0;
			int yOffset = 0;

			running = true;
			while (running)
			{
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						running = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				RenderWeirdGradient(xOffset, yOffset);

				HDC DeviceContext = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(Window, DeviceContext);

				++xOffset;
			}
		}
		else {
			// Logging
		}
	}
	else {
		// Logging
	}


	return 0;
}