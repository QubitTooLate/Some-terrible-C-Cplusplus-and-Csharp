
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#include <stdio.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include <dwrite.h>
#pragma comment(lib, "dwrite")

#define SAFE_RELEASE(obj) if (obj) obj->Release()

const RECT CLIENT_RECT =
{
	0L,
	0L,
	1280L,
	720L
};

const int LifeWidth = 8000;
const int LifeHeight = 8000;

HWND WindowHandle = NULL;
HDC WindowDeviceContextHandle = NULL;

ID2D1Factory* Factory = NULL;
ID2D1DCRenderTarget* DCRenderTarget = NULL;
ID2D1SolidColorBrush* SolidBrush = NULL;

IDWriteFactory* WriteFactory = NULL;
IDWriteTextFormat* TextFormat = NULL;

UINT* GameBoard = NULL;
UINT* Buffer = NULL;

ID2D1Bitmap* Map = NULL;

D2D1_PIXEL_FORMAT PixelFormat =
{
	DXGI_FORMAT_B8G8R8A8_UNORM,
	D2D1_ALPHA_MODE_PREMULTIPLIED
};

float Zoom = 1.0f;

D2D1_POINT_2F MapPosition = { };
D2D1_POINT_2F PreviousMapPosition = { };
D2D1_POINT_2F PreviousCursorPosition = { };
bool CursorDown = false;

void Initialize()
{
	D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&Factory
	);

	D2D1_RENDER_TARGET_PROPERTIES props = 
	{
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		PixelFormat,
		0,
		0,
		D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
		D2D1_FEATURE_LEVEL_DEFAULT
	};

	Factory->CreateDCRenderTarget(&props, &DCRenderTarget);

	WindowDeviceContextHandle = GetDC(WindowHandle);

	DCRenderTarget->BindDC(WindowDeviceContextHandle, &CLIENT_RECT);

	DCRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Red),
		&SolidBrush
	);
	
	DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_ISOLATED, 
		__uuidof(IDWriteFactory), 
		(IUnknown**)&WriteFactory
	);

	WriteFactory->CreateTextFormat(
		L"Segoe UI Emoji",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16.0f,
		L"en-us",
		&TextFormat
	);

	GameBoard = new UINT[LifeWidth * LifeHeight];
	Buffer = new UINT[LifeWidth * LifeHeight];

	for (size_t i = 0; i < LifeWidth * LifeHeight; i++)
	{
		GameBoard[i] = rand() % 15 ? 0xFF << 24 : 0xFFFFFFFF;
		Buffer[i] = 0;
	}

	D2D1_BITMAP_PROPERTIES dbp =
	{
		PixelFormat,
		0,
		0
	};

	DCRenderTarget->CreateBitmap(D2D1::SizeU(LifeWidth, LifeHeight), GameBoard, LifeWidth * 4, &dbp, &Map);

	SetTimer(WindowHandle, 0, 1, NULL);
}

void Release()
{
	SAFE_RELEASE(Factory);
	SAFE_RELEASE(DCRenderTarget);
	SAFE_RELEASE(SolidBrush);

	SAFE_RELEASE(WriteFactory);
	SAFE_RELEASE(TextFormat);

	delete[] GameBoard;
	delete[] Buffer;
	
	ReleaseDC(WindowHandle, WindowDeviceContextHandle);

	SAFE_RELEASE(Map);

	KillTimer(WindowHandle, 0);
}

HANDLE Mutex1 = NULL;
DWORD WINAPI thread1(LPVOID lpThreadParameter)
{
	Mutex1 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex1, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = 1; y < LifeHeight / 8; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex1);
	return 0;
}

HANDLE Mutex2 = NULL;
DWORD WINAPI thread2(LPVOID lpThreadParameter)
{
	Mutex2 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex2, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = (LifeHeight / 8) + 1; y < (LifeHeight / 8) * 2; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex2);
	return 0;
}

HANDLE Mutex3 = NULL;
DWORD WINAPI thread3(LPVOID lpThreadParameter)
{
	Mutex3 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex3, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = ((LifeHeight / 8) * 2) + 1; y < (LifeHeight / 8) * 3; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex3);
	return 0;
}

HANDLE Mutex4 = NULL;
DWORD WINAPI thread4(LPVOID lpThreadParameter)
{
	Mutex4 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex4, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = ((LifeHeight / 8) * 3) + 1; y < (LifeHeight / 8) * 4; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex4);
	return 0;
}

HANDLE Mutex5 = NULL;
DWORD WINAPI thread5(LPVOID lpThreadParameter)
{
	Mutex5 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex5, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = ((LifeHeight / 8) * 4) + 1; y < (LifeHeight / 8) * 5; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex5);
	return 0;
}

HANDLE Mutex6 = NULL;
DWORD WINAPI thread6(LPVOID lpThreadParameter)
{
	Mutex6 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex6, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = ((LifeHeight / 8) * 5) + 1; y < (LifeHeight / 8) * 6; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex6);
	return 0;
}

HANDLE Mutex7 = NULL;
DWORD WINAPI thread7(LPVOID lpThreadParameter)
{
	Mutex7 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex7, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = ((LifeHeight / 8) * 6) + 1; y < (LifeHeight / 8) * 7; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex7);
	return 0;
}

HANDLE Mutex8 = NULL;
DWORD WINAPI thread8(LPVOID lpThreadParameter)
{
	Mutex8 = CreateMutexA(NULL, FALSE, NULL);
	WaitForSingleObject(Mutex8, INFINITE);

	for (int x = 1; x < LifeWidth - 1; x++)
	{
		for (int y = ((LifeHeight / 8) * 7) + 1; y < LifeHeight - 1; y++)
		{
			int neighborgs = 0;

			if (GameBoard[(x - 1) + ((y - 1) * LifeWidth)] & 1) // left upper
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y - 1) * LifeWidth)] & 1) //  upper
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y - 1) * LifeWidth)] & 1) // right upper
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + (y * LifeWidth)] & 1) // left
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + (y * LifeWidth)] & 1) // right
			{
				++neighborgs;
			}

			if (GameBoard[(x - 1) + ((y + 1) * LifeWidth)] & 1) // left under
			{
				++neighborgs;
			}

			if (GameBoard[x + ((y + 1) * LifeWidth)] & 1) // under
			{
				++neighborgs;
			}

			if (GameBoard[(x + 1) + ((y + 1) * LifeWidth)] & 1) // right under
			{
				++neighborgs;
			}

			switch (neighborgs)
			{
			case 2:
				if (GameBoard[x + (y * LifeWidth)] & 1)
				{
					Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				}
				break;
			case 3:
				Buffer[x + (y * LifeWidth)] = 0xFFFFFFFF;
				break;
			default:
				Buffer[x + (y * LifeWidth)] = 0xFF << 24;
				break;
			}
		}
	}

	ReleaseMutex(Mutex8);
	return 0;
}

void GameTick()
{
	HANDLE t1 = CreateThread(NULL, 0, thread1, NULL, NULL, NULL);

	HANDLE t2 = CreateThread(NULL, 0, thread2, NULL, NULL, NULL);

	HANDLE t3 = CreateThread(NULL, 0, thread3, NULL, NULL, NULL);

	HANDLE t4 = CreateThread(NULL, 0, thread4, NULL, NULL, NULL);

	HANDLE t5 = CreateThread(NULL, 0, thread5, NULL, NULL, NULL);

	HANDLE t6 = CreateThread(NULL, 0, thread6, NULL, NULL, NULL);

	HANDLE t7 = CreateThread(NULL, 0, thread7, NULL, NULL, NULL);

	HANDLE t8 = CreateThread(NULL, 0, thread8, NULL, NULL, NULL);

	while (Mutex1 == NULL)
	{
		Sleep(1);
	}

	WaitForSingleObject(Mutex1, INFINITE);
	CloseHandle(Mutex1);
	CloseHandle(t1);
	Mutex1 = NULL;

	WaitForSingleObject(Mutex2, INFINITE);
	CloseHandle(Mutex2);
	CloseHandle(t2);
	Mutex2 = NULL;

	WaitForSingleObject(Mutex3, INFINITE);
	CloseHandle(Mutex3);
	CloseHandle(t3);
	Mutex3 = NULL;

	WaitForSingleObject(Mutex4, INFINITE);
	CloseHandle(Mutex4);
	CloseHandle(t4);
	Mutex4 = NULL;

	WaitForSingleObject(Mutex5, INFINITE);
	CloseHandle(Mutex5);
	CloseHandle(t5);
	Mutex5 = NULL;

	WaitForSingleObject(Mutex6, INFINITE);
	CloseHandle(Mutex6);
	CloseHandle(t6);
	Mutex6 = NULL;

	WaitForSingleObject(Mutex7, INFINITE);
	CloseHandle(Mutex7);
	CloseHandle(t7);
	Mutex7 = NULL;

	WaitForSingleObject(Mutex8, INFINITE);
	CloseHandle(Mutex8);
	CloseHandle(t8);
	Mutex8 = NULL;

	memcpy(GameBoard, Buffer, LifeWidth * LifeHeight * 4);

	D2D1_BITMAP_PROPERTIES dbp =
	{
		PixelFormat,
		0,
		0
	};

	Map->CopyFromMemory(&D2D1::RectU(0, 0, LifeWidth, LifeHeight), GameBoard, LifeWidth * 4);

	DCRenderTarget->BeginDraw();
	DCRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	DCRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation((CLIENT_RECT.right / 2) + (-LifeWidth / 2) + MapPosition.x, (CLIENT_RECT.bottom / 2) + (-LifeHeight / 2) + MapPosition.y) * D2D1::Matrix3x2F::Scale(Zoom, Zoom));
	DCRenderTarget->DrawBitmap(
		Map,
		D2D1::RectF(0.0f, 0.0f, LifeWidth, LifeHeight),
		1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
		D2D1::RectF(0.0f, 0.0f, LifeWidth, LifeHeight)
	);
	DCRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0, 0) * D2D1::Matrix3x2F::Scale(1.0f, 1.0f));
	DCRenderTarget->DrawTextA(
		L"Bad Coded Game Of Life 8000 x 8000",
		34,
		TextFormat,
		D2D1::RectF(5.5f, 5.5f, 205.0f, 105.0f),
		SolidBrush,
		D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
		DWRITE_MEASURING_MODE_NATURAL
	);
	DCRenderTarget->EndDraw();
}

LRESULT CALLBACK Procedure(HWND window_handle, UINT message, WPARAM w, LPARAM l)
{
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_MOUSEMOVE:
		if (CursorDown)
		{
			MapPosition = { PreviousMapPosition.x - PreviousCursorPosition.x + (float)LOWORD(l), PreviousMapPosition.y - PreviousCursorPosition.y + (float)HIWORD(l) };
		}
		break;
	case WM_LBUTTONDOWN:
		PreviousCursorPosition = { (float)LOWORD(l), (float)HIWORD(l) };
		CursorDown = true;
		break;
	case WM_LBUTTONUP:
		PreviousMapPosition = MapPosition = { PreviousMapPosition.x - PreviousCursorPosition.x + (float)LOWORD(l), PreviousMapPosition.y - PreviousCursorPosition.y + (float)HIWORD(l) };
		CursorDown = false;
		break;
	case WM_KEYDOWN:
		if (w == VK_UP)
		{
			Zoom *= 1.1f;
		}
		else if (w == VK_DOWN)
		{
			Zoom *= 0.9f;
		}
		break;
	case WM_TIMER:
		GameTick();
		break;
	case WM_MOVE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcA(window_handle, message, w, l);
	}

	return 0;
}

int main()
{
	WNDCLASSEXA windowClass = { };
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.lpszClassName = "MoveMeClass";
	windowClass.lpfnWndProc = Procedure;
	windowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);

	if (RegisterClassExA(&windowClass))
	{
		RECT windowRect = CLIENT_RECT;
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		WindowHandle = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"Move me around!",
			WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right,
			windowRect.bottom,
			NULL,
			NULL,
			NULL,
			NULL
		);

		if (WindowHandle)
		{
			Initialize();

			ShowWindow(WindowHandle, SW_NORMAL);

			MSG msg = { };
			while (GetMessageA(&msg, NULL, 0, 0))
			{
				DispatchMessageA(&msg);
			}

			Release();
		}
	}
}
