#pragma once
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <shobjidl.h> 
#include <sstream>
#include <imm.h>  // ���뷨�������ͷ�ļ�
#include <vector>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Imm32.lib")

// �ؼ���Ⱦѭ��
void Render();

// ���򵥴������ԭ��

// 2024-1-2   7:00  �����ק��ť �򵥿���̨ ����ѡ����Ⱦ��������ͼ����Ⱦ
// 2024-1-1   15:20 �����Ż��˵�ѡ�༭��
// 2024-1-1	  10:10 ����IO���ݽṹ�Ż�����
// 2024-1-1	  7:00  ��ɱ�ǩ�����ɸ�ʽ�����
// 2023-12-31 6:50  ����ʵ�ֱ༭��
// 2023-12-28 22:50 ��ɫ�ṹ�ع�
// 2023-12-28 20:00 �Զ����ֲ�����п���


// DirectX��Դ
namespace ui{
	//-------------------------------------------------------------------------
	// [��Դ] D2D
	//-------------------------------------------------------------------------
	static ID2D1Factory*			g_pFactory = NULL;
	static ID2D1HwndRenderTarget*	g_pRenderTarget = NULL;

	// ��ʼ��D2D�豸
	void InitD2D(HWND hwnd){
		RECT rc;
		GetClientRect(hwnd, &rc);
		D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &g_pFactory);
		g_pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
			&g_pRenderTarget
			);
	}

	// �ͷ�D2D��Դ
	void ReleaseD2D(){
		if (g_pFactory) g_pFactory->Release();
		if (g_pRenderTarget) g_pRenderTarget->Release();
	}

	// ����D2D��ȾĿ���С
	void Resize(int clientW, int clientH){
		g_pRenderTarget->Resize(D2D1::SizeU(clientW, clientH));
	}


	//-------------------------------------------------------------------------
	// [��Դ] ��ˢ
	//-------------------------------------------------------------------------
	enum BrushType {
		BrushS,
		ButtonBrush,
		ButtonHoveredBrush,
		ButtonActiveBrush,
		TextBrush,
		TextWhiteBrush,
		EditBoxCursorBrush,
		EditBoxSelectBrush,
		EditBoxBackgroundBrush,
		EditBoxBackgroundHovereBrush,
		EditBoxBackgroundActiveBrush,
		BrushCount // ���ڱ�ʾ��ˢ����
	};
	const D2D1_COLOR_F colorMapping[BrushCount] = {
		D2D1::ColorF(255, 0, 255, 0.2f),	// BrushS
		D2D1::ColorF(100, 76, 95, 1),		// ButtonBrush
		D2D1::ColorF(120, 76, 95, 1),		// ButtonHoveredBrush
		D2D1::ColorF(140, 76, 95, 1),		// ButtonActiveBrush
		D2D1::ColorF(255, 255, 255, 1),		// TextBrush
		D2D1::ColorF(255, 255, 255, 1),		// TextWhiteBrush
		D2D1::ColorF(189, 152, 255, 0.8f),	// EditBoxCursorBrush 
		D2D1::ColorF(140, 76, 95, 1),		// EditBoxSelectBrush 
		D2D1::ColorF(60, 76, 95, 1),		// EditBoxBackgroundBrush
		D2D1::ColorF(60, 76, 95, 1),		// EditBoxBackgroundHovereBrush 
		D2D1::ColorF(75, 90, 110, 1)		// EditBoxBackgroundActiveBrush ѡ��������ɫ
	};
	//86,103,126
	ID2D1SolidColorBrush* g_brushs[BrushCount];

	// ��ʼ����ɫ��Դ
	void InitColors() {
		for (int i = 0; i < BrushCount; ++i) {
			D2D1_COLOR_F rgb = colorMapping[i];
			g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rgb.r / 255.0f, rgb.g / 255.0f, rgb.b / 255.0f, rgb.a), &g_brushs[i]);
		}
	}

	// �ͷ���ɫ��Դ
	void ReleaseColors() {
		for (int i = 0; i < BrushCount; ++i) {
			if (g_brushs[i]) {
				g_brushs[i]->Release();
				g_brushs[i] = nullptr;
			}
		}
	}


	//-------------------------------------------------------------------------
	// [��Դ] ����
	//-------------------------------------------------------------------------
	static IDWriteFactory*			g_pDWriteFactory = NULL;
	static IDWriteTextFormat*		g_pTextFormat = NULL;
	static IDWriteFactory*			g_pDWriteFactory2 = NULL;
	static IDWriteTextFormat*		g_pTextFormat2 = NULL;

	// ��ʼ��������Դ
	void InitFont(){

		// ��ʼ������
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
		g_pDWriteFactory->CreateTextFormat(
			L"΢���ź�",            // ��������
			NULL,                   // ���弯
			DWRITE_FONT_WEIGHT_THIN, //DWRITE_FONT_WEIGHT_NORMAL 
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			12.0f,                  // �����С
			L"",                    // ����
			&g_pTextFormat
			);

		// �����ı����ж���
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		g_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		// ������βü�������ֿ���ı�
		DWRITE_TRIMMING trimmingOptions;
		trimmingOptions.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
		trimmingOptions.delimiter = 0;
		trimmingOptions.delimiterCount = 0;
		g_pTextFormat->SetTrimming(&trimmingOptions, nullptr);

		//================
		// ��ʼ������
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory2));
		g_pDWriteFactory2->CreateTextFormat(
			L"΢���ź�",            // ��������
			NULL,                   // ���弯
			DWRITE_FONT_WEIGHT_THIN, //DWRITE_FONT_WEIGHT_NORMAL 
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			12.0f,                  // �����С
			L"",                    // ����
			&g_pTextFormat2
			);

		// �����ı����ж���
		g_pTextFormat2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); // �����
		g_pTextFormat2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); // ��ֱ���ж���

		// ������βü�������ֿ���ı�
		DWRITE_TRIMMING trimmingOptions2;
		trimmingOptions2.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
		trimmingOptions2.delimiter = 0;
		trimmingOptions2.delimiterCount = 0;
		g_pTextFormat2->SetTrimming(&trimmingOptions2, nullptr);
	}

	// �ͷ�������Դ
	void ReleaseFont(){
		if (g_pTextFormat) g_pTextFormat->Release();
		if (g_pDWriteFactory) g_pDWriteFactory->Release();

		if (g_pTextFormat2) g_pTextFormat2->Release();
		if (g_pDWriteFactory2) g_pDWriteFactory2->Release();
	}

}

// ��ȡ�豸����
namespace io {
	// [ȫ������] 
	HWND	Hwnd = nullptr;
	int		ClientWidth, ClientHeight;
	float	MouseWheel = 0.0f;
	bool	IsKeysDown[256] = { false };	// ���̰���״̬
	bool	IsMouseDown[5] = { false };		// ��갴��״̬ 0��� 1�Ҽ� 2�м� 3���� 4ǰ��
	bool	IsMouseReleased[5] = { true };
	POINT	MousePos = { -1, -1 };			// �ͻ������λ��
	std::wstring ch = L"";					// ������ַ�ÿ�λ滭�����ǵ����
	POINT	CursorScreenPos = { 0, 0 };		// �༭����λ����Ļ���꣬���ڱ༭��������£������뷨Ԥѡ�ʿ�λ����Ϊ����

	// [����IO����] 
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			SetFocus(hwnd);	//�����½��㲻Ȼ�������ڲ��ἤ�����뷨
			Hwnd = hwnd;
			ui::InitD2D(hwnd);
			ui::InitColors();
			ui::InitFont();
			break;

		case WM_MOUSEMOVE:MousePos = { LOWORD(lParam), HIWORD(lParam) }; break;
		case WM_MOUSEWHEEL:MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA; break;
		case WM_LBUTTONDOWN:IsMouseDown[0] = true; break;
		case WM_RBUTTONDOWN:IsMouseDown[1] = true; break;
		case WM_MBUTTONDOWN:IsMouseDown[2] = true; break;
		case WM_XBUTTONDOWN:IsMouseDown[(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4] = true; break;
		case WM_LBUTTONUP:IsMouseDown[0] = false; IsMouseReleased[0] = true; break;
		case WM_RBUTTONUP:IsMouseDown[1] = false; IsMouseReleased[1] = true; break;
		case WM_MBUTTONUP:IsMouseDown[2] = false; IsMouseReleased[2] = true; break;
		case WM_XBUTTONUP:{ int index = GET_XBUTTON_WPARAM(wParam) == XBUTTON2 ? 4 : 3; IsMouseDown[index] = false; IsMouseReleased[index] = true; } break;

		case WM_CHAR:
			// ���δ����CTRL��wParam��VK_BACK�������ַ�����
			if (!(GetKeyState(VK_CONTROL) & 0x8000) && wParam != VK_BACK) {
				ch += (wParam == VK_RETURN) ? L'\r\n' : static_cast<wchar_t>(wParam); // Enter���뻻�У���������ַ�
			}
			break;

		case WM_PAINT:Render(); break;
		case WM_KEYDOWN:IsKeysDown[wParam] = true; break;
		case WM_KEYUP:IsKeysDown[wParam] = false; break;

		case WM_SIZE:
			RECT rc;
			GetClientRect(hwnd, &rc);
			ClientWidth = rc.right - rc.left;
			ClientHeight = rc.bottom - rc.top;
			ui::Resize(ClientWidth, ClientHeight);
			break;

		case WM_CONTEXTMENU:
			break;

		case WM_CLOSE:
			// ������Դ
			ui::ReleaseD2D();
			ui::ReleaseColors();
			ui::ReleaseFont();
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	// [��������]
	bool IsMouseInRect(const D2D1_RECT_F& rect) {
		return MousePos.x >= rect.left && MousePos.x <= rect.right &&
			MousePos.y >= rect.top && MousePos.y <= rect.bottom;
	}
	bool IsMouseInRect(const D2D1_ROUNDED_RECT& rect) {
		return MousePos.x >= rect.rect.left && MousePos.x <= rect.rect.right &&
			MousePos.y >= rect.rect.top && MousePos.y <= rect.rect.bottom;
	}
	bool IsMouseClicked(int buttonIndex) {
		if (IsMouseDown[buttonIndex] && IsMouseReleased[buttonIndex])
		{
			IsMouseReleased[buttonIndex] = false;
			return true;
		}
		return false;
	}
	bool IsMouseClickedInRect(const D2D1_ROUNDED_RECT& rect) {
		return MousePos.x >= rect.rect.left && MousePos.x <= rect.rect.right && MousePos.y >= rect.rect.top && MousePos.y <= rect.rect.bottom && IsMouseClicked(0);
	}
	bool IsKeyPressed(int keyCode) {
		// ���ټ��̰�������һ�ε�״̬
		static bool IsKeysPress[256] = { false };
		if (IsKeysDown[keyCode] && !IsKeysPress[keyCode]) {
			IsKeysPress[keyCode] = true;
			return true;
		}
		else if (!IsKeysDown[keyCode]) {
			IsKeysPress[keyCode] = false;
		}
		return false;
	}
	void SetClipboard(const std::wstring& text) {
		if (!OpenClipboard(NULL)) return;
		EmptyClipboard();

		HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, (text.length() + 1) * sizeof(wchar_t));
		wchar_t* buffer = (wchar_t*)GlobalLock(clipbuffer);
		wcscpy_s(buffer, text.length() + 1, text.c_str());

		GlobalUnlock(clipbuffer);

		SetClipboardData(CF_UNICODETEXT, clipbuffer);
		CloseClipboard();
	}
	std::wstring GetClipboardText() {
		// ���Դ򿪼�����
		if (!OpenClipboard(NULL)) {
			OutputDebugString(L"�޷��򿪼����塣\n");
			return L"";
		}

		std::wstring text;
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData != NULL) {
			// �������������ݲ��������Ч��
			wchar_t* buffer = static_cast<wchar_t*>(GlobalLock(hData));
			if (buffer != NULL) {
				// ��ȫ��ʩ�����ƶ�ȡ���ı�����
				size_t maxLen = 1024; // ����������ѡ��һ�����ʵ���󳤶�
				text.assign(buffer, wcsnlen(buffer, maxLen));

				GlobalUnlock(hData);
			}
			else {
				OutputDebugString(L"�޷��������������ݡ�\n");
			}
		}
		else {
			OutputDebugString(L"δ�ҵ����������ݡ�\n");
		}

		CloseClipboard();
		return text;
	}
}

// С������Widgets��
namespace ui{
	float	g_Round = 3.5f;		//����Բ��
	float	g_WidgetX = 0.0f;	//�ؼ��Զ�����
	float	g_WidgetY = 0.0f;
	float	g_WidgetW = 0.0f;
	float	g_WidgetH = 0.0f;
	bool	g_SameLine = false;	//��һ���ؼ��Ƿ�ͬһ��
	float	g_PaddingX = 10.0f;
	float	g_PaddingY = 20.0f;
	float	g_MarginX = 5.0f;
	float	g_MarginY = 5.0f;
	float	g_LabelW = 50;
	float	g_LabelH = 20;
	float	g_ButtonW = 75.0f;
	float	g_ButtonH = 23.0f;

	// ���Ժ���
	void AsyncBeep(){
		CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
			Beep(55555, 55);
			return 0;
		}, NULL, 0, NULL);
	}

	void PrintFPS() {
		static DWORD lastTime = 0; // �ϴε��õ�ʱ��
		static int frameCount = 0; // ֡��������

		// ��ȡ��ǰʱ��
		DWORD currentTime = GetTickCount64();

		// �������ϴε�������������ʱ�䣨�Ժ���Ϊ��λ��
		DWORD timeDiff = currentTime - lastTime;

		// �ۼ�֡��
		frameCount++;

		// ���ʱ������1000���루1�룩������FPS
		if (timeDiff >= 1000) {
			float fps = frameCount * 1000.0f / timeDiff; // ����FPS

			// ��������ַ���
			wchar_t output[256];
			swprintf(output, 256, L"FPS: %.2f\n", fps);
			OutputDebugStringW(output);

			// ���ü�������ʱ��
			frameCount = 0;
			lastTime = currentTime;
		}
	}

	// һ�еĿ�ʼ
	void Begin(){
		ui::PrintFPS();
		Sleep(1);
		g_pRenderTarget->BeginDraw();
		//g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		g_pRenderTarget->Clear(D2D1::ColorF(48 / 255.0f, 63 / 255.0f, 79 / 255.0f, 1));

		// �ؼ��Զ�����λ������
		g_WidgetX = 0.0f;
		g_WidgetY = 0.0f;
		g_WidgetW = 0.0f;
		g_WidgetH = 0.0f;


	}
	void End(){
		g_pRenderTarget->EndDraw();
		io::ch.clear();
	}

	// ����ʵ��
	void BeginAnimateScale(D2D1_ROUNDED_RECT roundedRect) {
		if (io::IsMouseInRect(roundedRect)) {
			static DWORD lastTick = 0;			// �ϴθ��µ�ʱ��
			static float currentScale = 1.0f;	// ��ǰ���ű���

			// �������Ŷ���
			if (io::IsMouseInRect(roundedRect)) {
				DWORD currentTick = GetTickCount64();
				const float scaleSpeed = 0.002f;	// �����ٶ�
				const float minScale = 0.95f;		// ��С���ű���
				const float maxScale = 1.0f;		// ������ű���

				if (io::IsMouseDown[0]) {
					currentScale -= scaleSpeed * (currentTick - lastTick);
					currentScale = max(minScale, currentScale);
				}
				else {
					currentScale += scaleSpeed * (currentTick - lastTick);
					currentScale = min(maxScale, currentScale);
				}

				lastTick = currentTick;
			}

			// ����Բ�Ǿ��ε����ĵ�
			D2D1_POINT_2F rectCenter = D2D1::Point2F(
				(roundedRect.rect.left + roundedRect.rect.right) / 2.0f,
				(roundedRect.rect.top + roundedRect.rect.bottom) / 2.0f
				);

			// ���ñ任����
			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(currentScale, currentScale, rectCenter));
		}
	}
	void EndAnimateScale(){
		// ���ñ任����
		g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	// �Զ�����ʵ��
	void AutoLayout(float& x, float& y, float& w, float& h) {
		// ����Ƿ���Ҫ���У���ǰ�п�ȼ�����һ���ؼ�����Ƿ񳬹����ڿ��
		if (g_SameLine && (g_WidgetX + w + g_MarginX > io::ClientWidth)) {
			// ��Ҫ����
			g_SameLine = false;
			g_WidgetX = 0;
			g_WidgetY += g_WidgetH;
			g_WidgetH = 0;
		}

		if (g_SameLine) {
			// ͬһ���е���һ���ؼ�����x���������ˮƽ���
			x = g_WidgetX + g_MarginX;
			y = g_WidgetY;
		}
		else {
			// ���еĵ�һ���ؼ�������x�����y����
			x = g_PaddingX; // �������еĵ�һ���ؼ��������ڱ߾�
			y = (g_WidgetY == 0 && g_WidgetX == 0) ? g_PaddingY : g_WidgetY + g_WidgetH + g_MarginY;

			// �����и�Ϊ���еĵ�һ���ؼ��߶�
			g_WidgetH = h;
		}

		// ����ȫ��λ��״̬��Ϊ��һ���ؼ�׼��
		g_WidgetX = x + w;
		g_WidgetY = y;

		// ����NextSameLine��־
		g_SameLine = false;
	}

	// �жϿؼ��Ƿ����ٲ����ڴ��ڵĿͻ�����
	bool IsWidgetInClient(float x, float y, float w, float h) {
		// ���ؼ����½������Ƿ��ڴ������Ͻǵ������Ϸ�
		if (x + w < 0 || y + h < 0) {
			return false;
		}

		// ���ؼ����Ͻ������Ƿ��ڴ������½ǵ��Ҳ���·�
		if (x > io::ClientWidth || y > io::ClientHeight) {
			return false;
		}

		// ������������������㣬��ʾ�ؼ����ٲ����ڿͻ�����
		return true;
	}

	// �¸�����Ϊͬһ��
	void SameLine(){
		g_SameLine = true;
	}
	// �������
	void NewLine(float height = 0.0f) {
		// �����ǰ�����пؼ�������
		if (g_WidgetX != 0.0f) {
			g_WidgetX = 0.0f; // �����е�ˮƽ���

			// ����������Ϊ0����ʹ��ȫ�ֱ���g_widget_h������ʹ�ô���Ĳ���
			if (height == 0.0f) {
				g_WidgetY += g_MarginY;
			}
			else {
				g_WidgetY += height + g_MarginY;
			}
		}
		// �����ǰ��û�пؼ�����ִ�в������Ѵ������е���ʼλ�ã�
	}


	// DrawRoundedRectangle
	// DrawEllipse
	// DrawLine
	// DrawText
	// DrawBitmap
	// Clipping|Opacity

	//[v] Label
	//[x] SingleBox
	//[x] SliderBar
	//[v] EditBox 
	//[x] Image
	//[x] ImageButton
	//[x] ProgressBar
	//[x] BeginComboBox|EndComboBox|ComboBoxItem +��Ҫʵ�ֹ�������ҳ��
	//[x] ColorPicker
	//[x] Help
	//[x] BeginMenu|EndMenu|MenuItem
	//[x] BeginPopup|EndPopup ����˼·
	//[x] BeginTable|EndTable ���Լ���ɾҳ
	//DPIʵ��

	bool Label(const wchar_t* format, ...) {
		float x, y, w = g_LabelW, h = g_LabelH;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return false;

		va_list args;
		va_start(args, format);

		wchar_t buffer[1024];
		vswprintf(buffer, 1024, format, args);

		va_end(args);

		std::wstring text = buffer;



		D2D1_RECT_F textRect = D2D1::RectF(x, y, x + w, y + h);
		D2D1_ROUNDED_RECT textRectBG = D2D1::RoundedRect(textRect, g_Round, g_Round);

		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		g_pRenderTarget->DrawTextW(text.c_str(), text.length(), g_pTextFormat, textRect, g_brushs[TextBrush]);
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		return io::IsMouseClickedInRect(textRectBG);
	}

	bool Button(std::wstring text = L"��ť") {
		float x, y, w = g_ButtonW, h = g_ButtonH;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return false;

		D2D1_ROUNDED_RECT rc = D2D1::RoundedRect(D2D1::RectF(x, y, x + w, y + h), g_Round, g_Round);
		ID2D1SolidColorBrush* pBrush = io::IsMouseInRect(rc) && io::IsMouseDown[0] ? g_brushs[ButtonActiveBrush] : (io::IsMouseInRect(rc) ? g_brushs[ButtonHoveredBrush] : g_brushs[ButtonBrush]);

		BeginAnimateScale(rc);
		g_pRenderTarget->FillRoundedRectangle(rc, pBrush);
		g_pRenderTarget->DrawTextW(text.c_str(), text.length(), g_pTextFormat, D2D1::RectF(x, y, x + w, y + h), g_brushs[TextBrush]);
		EndAnimateScale();

		return io::IsMouseClickedInRect(rc);
	}

	bool CheckBox(const std::wstring& text, bool& isChecked) {
		float x, y, w = 120, h = 18;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return false;

		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(x, y, x + 18, y + h), g_Round, g_Round);
		D2D1_ROUNDED_RECT allRoundedRect = D2D1::RoundedRect(D2D1::RectF(x, y, x + w, y + h), g_Round, g_Round);
		ID2D1SolidColorBrush* pBrush = io::IsMouseInRect(allRoundedRect) && io::IsMouseDown[0] ? g_brushs[ButtonActiveBrush] :
			(io::IsMouseInRect(allRoundedRect) ? g_brushs[ButtonHoveredBrush] : g_brushs[ButtonBrush]);

		// �ж�����Ƿ����˸�ѡ��
		bool clicked = io::IsMouseClickedInRect(allRoundedRect);
		if (clicked) {
			isChecked = !isChecked; // �л���ѡ���ѡ��״̬
		}

		// ���Ƹ�ѡ��
		g_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

		if (isChecked) {
			// ����ˮƽ�����"L"��״����������
			float rectWidth = 2.5f;  // ���ο��
			float shortRectWidth = h / 4.0f;  // �϶̾��εĿ��
			float longRectHeight = h / 2.0f;  // �ϳ����εĸ߶�

			D2D1_RECT_F shortRect = D2D1::RectF(x + h - shortRectWidth - 5, y + h / 2, x + h - 5, y + h / 2 + rectWidth);
			D2D1_RECT_F longRect = D2D1::RectF(x + h / 2 - rectWidth, y + h / 2, x + h / 2, y + h + 3);

			// �����任��
			ID2D1Layer* pLayer = NULL;
			g_pRenderTarget->CreateLayer(NULL, &pLayer);
			g_pRenderTarget->PushLayer(D2D1::LayerParameters(), pLayer);

			// Ӧ����ת�任��90�ȣ�
			D2D1_POINT_2F center = D2D1::Point2F(x + h / 2 - 1, y + h / 2 + 3);
			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(225, center));

			// ����ˮƽ�����"L"��״
			g_pRenderTarget->FillRectangle(shortRect, g_brushs[TextBrush]);
			g_pRenderTarget->FillRectangle(longRect, g_brushs[TextBrush]);

			// ���ñ任���Ƴ���
			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			g_pRenderTarget->PopLayer();

			// ����
			if (pLayer) {
				pLayer->Release();
			}
		}

		// �����ı���ǩ
		D2D1_RECT_F textRect = D2D1::RectF(x + h + g_MarginX, y, x + w, y + h);
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		g_pRenderTarget->DrawTextW(text.c_str(), text.length(), g_pTextFormat, textRect, g_brushs[TextBrush]);
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		return clicked;
	}

	void EditBox(std::wstring& editText) {
		static int selectionStart = -1;			// ѡ��ʼλ�ã�-1 ��ʾû��ѡ��
		static int selectionEnd = -1;			// ѡ�����λ�ã�-1 ��ʾû��ѡ��
		static int cursorPosition = 0;			// ���λ�ã���ʾ��ǰ������ı��е�����λ��
		static bool cursorVisible = true;		// ���ƹ��Ŀɼ���״̬��true ��ʾ���ɼ�
		static int activeEditBoxKey = -1;		// ��ʼֵΪ-1����ʾû�б༭�򱻼���

		// �Զ�����
		float x = 0, y = 0, w = 300, h = 23;
		if (io::ClientWidth <= 300 + g_PaddingX * 2)w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return;

		// ���������������ڻ����ı�
		D2D1_RECT_F layoutRect = D2D1::RectF(x, y, x + w, y + h);

		if (io::IsMouseInRect(layoutRect))
		{
			// ���������
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
		}

		// ������Կ
		int editBoxKey = static_cast<int>(x) ^ static_cast<int>(y);

		// �������Ƿ����༭��
		if (io::IsMouseDown[0]) {
			if (io::IsMouseInRect(layoutRect)) {
				// �����������༭�򣬼�����
				activeEditBoxKey = editBoxKey;
			}
			else if (activeEditBoxKey == editBoxKey) {
				// �����������༭���ⲿ��ȡ������
				activeEditBoxKey = -1;
			}
		}

		// �������Ƿ��ڱ༭����
		bool isActiveEditBoxKey = activeEditBoxKey == editBoxKey;

		// ����TextLayout����������������
		IDWriteTextLayout* pTextLayout = nullptr;
		HRESULT hr = g_pDWriteFactory->CreateTextLayout(editText.c_str(), editText.length(), g_pTextFormat2, w, h, &pTextLayout);
		if (SUCCEEDED(hr)) {
			if (isActiveEditBoxKey)
			{
				// �������Ƿ��ڱ༭���ڲ������ַ���
				if (!io::ch.empty()) {

					// ����Ϊ��������
					if (io::ch.find(L'\r') != std::wstring::npos) {
						io::ch.clear();
					}

					// ����Ƿ���ѡ����ı�
					bool hasSelection = (selectionStart != -1) && (selectionEnd != -1) && (selectionStart != selectionEnd);
					size_t charsToRemove = hasSelection ? abs(selectionEnd - selectionStart) : 0;
					size_t newLength = editText.length() + io::ch.length() - charsToRemove;

					// ȷ���³��Ȳ������ַ�����󳤶�
					if (newLength <= editText.max_size()) {
						if (hasSelection) {
							int selStart = min(selectionStart, selectionEnd);
							int selEnd = max(selectionStart, selectionEnd);
							editText.replace(selStart, selEnd - selStart, io::ch);
							cursorPosition = selStart + io::ch.length();
						}
						else {
							// ȷ�����λ����Ч
							cursorPosition = (cursorPosition > editText.length()) ? editText.length() : cursorPosition;
							editText.insert(cursorPosition, io::ch);
							cursorPosition += io::ch.length();
						}
						// ����Ѵ�����ַ�
						io::ch.clear();
						// ����ѡ������
						selectionStart = selectionEnd = -1;
					}
				}


				// ========== ������༭���ѡ���� ==========
				if (io::IsMouseDown[0] || io::IsMouseClicked(0)) {
					BOOL isTrailingHit, isInside;
					DWRITE_HIT_TEST_METRICS metrics;

					// ����������꣬ʹ������ڱ༭��
					float adjustedX = static_cast<float>(io::MousePos.x) - x;
					float adjustedY = static_cast<float>(io::MousePos.y) - y;

					// ʹ�õ�����������ȡ���λ��
					pTextLayout->HitTestPoint(adjustedX, adjustedY, &isTrailingHit, &isInside, &metrics);

					if (isInside || isTrailingHit) {
						cursorPosition = metrics.textPosition + (isTrailingHit ? 1 : 0);
						cursorPosition = min(cursorPosition, static_cast<int>(editText.length()));
					}

					if (io::IsMouseClicked(0)) {
						// ��ʼ��ѡ������Ϊ���λ��
						selectionStart = selectionEnd = cursorPosition;
					}
					else if (io::IsMouseDown[0]) {
						// ����ѡ������
						selectionEnd = cursorPosition;
					}
				}

				// ========== ��������Ԥѡ�ʴ���λ�� ==========
				DWRITE_HIT_TEST_METRICS metrics;
				float cursorX, cursorY;
				if (SUCCEEDED(pTextLayout->HitTestTextPosition(cursorPosition, false, &cursorX, &cursorY, &metrics))) {
					POINT pt = { static_cast<LONG>(x + cursorX), static_cast<LONG>(y + cursorY) };
					io::CursorScreenPos = pt;

					// �������뷨��ѡ��λ��
					HIMC hIMC = ImmGetContext(io::Hwnd);
					if (hIMC) {
						COMPOSITIONFORM cf = { CFS_POINT, io::CursorScreenPos };
						ImmSetCompositionWindow(hIMC, &cf);
						ImmReleaseContext(io::Hwnd, hIMC);
					}
				}


				// ========== ������Ҽ��ƶ� ==========
				static DWORD lastArrowPressTime = 0; // �ϴΰ��¼�ͷ����ʱ��
				static DWORD lastCursorMoveTime = 0; // �ϴ��ƶ�����ʱ��
				DWORD currentTime = GetTickCount64();
				bool isRapidMovement = (currentTime - lastArrowPressTime > 500); // �����ƶ�ģʽ
				// �������Ҽ�����ƶ�
				auto HandleCursorMove = [&](bool keyDown, int cursorChange, bool boundaryCondition) {
					if (keyDown && boundaryCondition) {
						if (isRapidMovement && (currentTime - lastCursorMoveTime >= 50)) {
							cursorPosition += cursorChange;
							lastCursorMoveTime = currentTime;
						}
						if (lastArrowPressTime == 0) lastArrowPressTime = currentTime;
						selectionStart = selectionEnd = -1;
					}
				};
				HandleCursorMove(io::IsKeysDown[VK_LEFT], -1, cursorPosition > 0);
				HandleCursorMove(io::IsKeysDown[VK_RIGHT], 1, cursorPosition < editText.length());
				if (!io::IsKeysDown[VK_LEFT] && !io::IsKeysDown[VK_RIGHT]) {
					lastArrowPressTime = lastCursorMoveTime = 0;
				}

				// ========== �˸��ɾ���ַ� ==========
				static DWORD lastBackspacePressTime = 0; // �ϴΰ����˸����ʱ��
				static DWORD lastCharDeleteTime = 0;     // �ϴ�ɾ���ַ���ʱ��

				if (io::IsKeysDown[VK_BACK] && cursorPosition > 0) {
					DWORD currentTime = GetTickCount64();
					bool isRapidDeletion = (currentTime - lastBackspacePressTime > 500); // ����ɾ��ģʽ
					bool hasSelection = (selectionStart != -1) && (selectionEnd != -1) && (selectionStart != selectionEnd);

					// ɾ��ѡ���ı��򵥸��ַ�
					if (hasSelection) {
						int selStart = min(selectionStart, selectionEnd);
						int selEnd = max(selectionStart, selectionEnd);
						editText.erase(selStart, selEnd - selStart);
						cursorPosition = selStart;
					}
					else if (cursorPosition > 0 && (isRapidDeletion || currentTime - lastCharDeleteTime >= 40)) {
						editText.erase(--cursorPosition, 1);
					}

					// ����ʱ���¼
					lastBackspacePressTime = (lastBackspacePressTime == 0) ? currentTime : lastBackspacePressTime;
					lastCharDeleteTime = currentTime;

					// ����ѡ������
					selectionStart = selectionEnd = -1;
				}
				else {
					// ����ʱ���¼
					lastBackspacePressTime = lastCharDeleteTime = 0;
				}

				// ========== ������ش��� ==========
				if (GetAsyncKeyState(VK_CONTROL))
				{
					// ========== Ctrl + A ȫѡ ==========
					if (io::IsKeyPressed('A')) {
						selectionStart = 0;
						selectionEnd = editText.length();
					}

					// ========== Ctrl + V ճ�� ==========
					if (io::IsKeyPressed('V'))
					{
						std::wstring clipboardText = io::GetClipboardText();
						bool hasSelection = (selectionStart != -1) && (selectionEnd != -1) && (selectionStart != selectionEnd);
						size_t charsToRemove = hasSelection ? abs(selectionEnd - selectionStart) : 0;
						size_t newLength = editText.length() + clipboardText.length() - charsToRemove;

						// ȷ���³��Ȳ������ַ�����󳤶�
						if (newLength <= editText.max_size()) {
							if (hasSelection) {
								int selStart = min(selectionStart, selectionEnd);
								int selEnd = max(selectionStart, selectionEnd);
								editText.replace(selStart, selEnd - selStart, clipboardText);
								cursorPosition = selStart + clipboardText.length();
							}
							else {
								// ȷ�����λ����Ч
								cursorPosition = (cursorPosition > editText.length()) ? editText.length() : cursorPosition;
								editText.insert(cursorPosition, clipboardText);
								cursorPosition += clipboardText.length();
							}
							// ����ѡ������
							selectionStart = selectionEnd = -1;
						}
					}

					// ========== Ctrl + C ����==========
					// ѡ���ı�����ʼ�ͽ���λ��
					int selStart = min(selectionStart, selectionEnd);
					int selEnd = max(selectionStart, selectionEnd);

					// ���ƣ�CTRL+C
					if (io::IsKeyPressed('C') && selStart != selEnd) {
						std::wstring selectedText = editText.substr(selStart, selEnd - selStart);
						io::SetClipboard(selectedText);
					}

					// ========== Ctrl + V ���� ==========
					if (io::IsKeyPressed('X'))
					{
						// ȷ��ѡ�����ʼ�ͽ���λ����Ч
						if (selStart != -1 && selEnd != -1 && selStart != selEnd) {
							// ȷ��ѡ��Χ���ַ���������
							selStart = max(0, selStart);
							selEnd = min(static_cast<int>(editText.length()), selEnd);

							if (selEnd > selStart) {
								std::wstring selectedText = editText.substr(selStart, selEnd - selStart);
								io::SetClipboard(selectedText);
								editText.erase(selStart, selEnd - selStart);
								cursorPosition = selStart;

								// ����ѡ������
								selectionStart = selectionEnd = -1;
							}
						}
						else {
							OutputDebugString(L"��Ч��ѡ������\n");
						}
					}

				}
			}

			// ========== ���Ʊ༭�򲿷� ==========

			// ���Ʊ༭���ɫ
			ID2D1SolidColorBrush* pBrush = isActiveEditBoxKey ? g_brushs[EditBoxBackgroundActiveBrush] : (io::IsMouseInRect(layoutRect) ? g_brushs[EditBoxBackgroundHovereBrush] : g_brushs[EditBoxBackgroundBrush]);

			g_pRenderTarget->FillRoundedRectangle(D2D1::RoundedRect(layoutRect, g_Round, g_Round), pBrush);

			// ����δѡ�е��ı���������ɫ��
			g_pRenderTarget->DrawTextW(editText.c_str(), editText.length(), g_pTextFormat2, layoutRect, g_brushs[TextBrush]);

			// ����ѡ���ı��ĸ��������Ͱ�ɫ�ı�
			if (selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd && isActiveEditBoxKey) {
				int selStart = min(selectionStart, selectionEnd);
				int selEnd = max(selectionStart, selectionEnd);

				DWRITE_HIT_TEST_METRICS hitTestMetrics;
				float selStartX, selStartY;
				float selEndX, selEndY;

				// ��ȡѡ���ı�����ʼ�ͽ�������
				pTextLayout->HitTestTextPosition(selStart, false, &selStartX, &selStartY, &hitTestMetrics);
				pTextLayout->HitTestTextPosition(selEnd, false, &selEndX, &selEndY, &hitTestMetrics);

				D2D1_RECT_F highlightRect = D2D1::RectF(x + selStartX, y + hitTestMetrics.top, x + selEndX, y + hitTestMetrics.top + hitTestMetrics.height);
				D2D1_ROUNDED_RECT roundedRect = { highlightRect, 2, 2 };
				g_pRenderTarget->FillRoundedRectangle(roundedRect, g_brushs[EditBoxSelectBrush]);

				std::wstring selectedText = editText.substr(selStart, selEnd - selStart);
				g_pRenderTarget->DrawTextW(selectedText.c_str(), selectedText.length(), g_pTextFormat2, highlightRect, g_brushs[TextWhiteBrush]);
			}

			// ���ƹ��
			if (cursorVisible && isActiveEditBoxKey && (selectionStart == -1 || selectionEnd == -1 || selectionStart == selectionEnd)) {
				DWRITE_HIT_TEST_METRICS metrics;
				float cursorX, cursorY;
				pTextLayout->HitTestTextPosition(cursorPosition, false, &cursorX, &cursorY, &metrics);

				cursorX += x; // ���Ǳ༭��� x ƫ��
				cursorY += y; // ���Ǳ༭��� y ƫ��

				// ���ƹ��
				g_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
				g_pRenderTarget->DrawLine(D2D1::Point2F(cursorX, cursorY), D2D1::Point2F(cursorX, cursorY + metrics.height), g_brushs[EditBoxCursorBrush], 2.0f);
				g_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			}


			// �ͷ�TextLayout����
			if (pTextLayout)pTextLayout->Release();

		}
		else {
			OutputDebugString(L"TextLayout ����ʧ�ܡ�\n");
		}
	}

	// ����̨ ��������������ַ��������β� �Լ���һ���ͺ�
	static std::vector<std::wstring> g_logEntries; // �洢��־��Ŀ������
	void Log() {
		float x = 0, y = 0, w = 300, h = 100; // ��־����ĳߴ��λ��
		if (io::ClientWidth <= 300 + g_PaddingX * 2)w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return;

		D2D1_RECT_F logRect = D2D1::RectF(x, y, x + w, y + h);
		D2D1_ROUNDED_RECT logRectBG = D2D1::RoundedRect(logRect, g_Round, g_Round);

		// ���Ʊ���
		g_pRenderTarget->FillRoundedRectangle(logRectBG, g_brushs[EditBoxBackgroundBrush]);

		// �����ı���ʽΪ�����
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

		// ���������ʾ������
		int maxLines = static_cast<int>(h / 20); // ����ÿ�и߶�Ϊ20����

		// ȷ������һ�п�ʼ����
		int startLine = max(0, static_cast<int>(g_logEntries.size()) - maxLines);

		// ����ÿ����־
		float lineSpacing = 20; // �м��
		float currentY = y + h - maxLines * lineSpacing; // �ӵײ���ʼ����
		for (int i = startLine; i < g_logEntries.size(); ++i) {
			D2D1_RECT_F textRect = D2D1::RectF(x + 5, currentY, x + w - 15, currentY + lineSpacing);
			textRect.top = max(textRect.top, y);

			g_pRenderTarget->DrawTextW(g_logEntries[i].c_str(), g_logEntries[i].length(), g_pTextFormat, textRect, g_brushs[TextBrush]);

			currentY += lineSpacing;

			// �����������������ֹѭ��
			if (currentY > y + h) {
				break;
			}
		}

		// �ָ��ı���ʽ��Ĭ�϶��뷽ʽ
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	void LogAdd(const wchar_t* fmt, ...) {
		wchar_t buffer[1024];
		int offset = 0;

		// �������ʱ���
		if (true) {
			FILETIME ft;
			ULARGE_INTEGER li;
			SYSTEMTIME st;

			GetSystemTimeAsFileTime(&ft);
			li.LowPart = ft.dwLowDateTime;
			li.HighPart = ft.dwHighDateTime;
			FileTimeToSystemTime(&ft, &st);

			offset = swprintf_s(buffer, 1024, L"[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
		}

		va_list args;
		va_start(args, fmt);
		vswprintf_s(buffer + offset, 1024 - offset, fmt, args);
		va_end(args);

		g_logEntries.emplace_back(buffer);

		if (g_logEntries.size() > 100) {
			g_logEntries.erase(g_logEntries.begin(), g_logEntries.begin() + 50);
		}
	}

	// ��ק����
	void DragInt(int& value, const std::wstring& label = L"", int minValue = 0, int maxValue = 100) {
		static int* pActiveValue = nullptr; // ָ��ǰ��ؼ���ָ��
		static float initialMouseX = 0.0f;  // ��¼�������ʱ�����X����
		const float labelW = 70;

		float x, y, w = 300, h = 15; // �ؼ���Ĭ�ϳߴ�
		if (io::ClientWidth <= 300 + g_PaddingX * 2)w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);

		if (IsWidgetInClient(x, y, w, h)) {
			D2D1_RECT_F rc = D2D1::RectF(x, y, x + w - labelW, y + h);
			D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(rc, g_Round, g_Round);

			// ���Ʊ���
			ID2D1SolidColorBrush* pBrush = io::IsMouseInRect(rc) && io::IsMouseDown[0] ? g_brushs[ButtonActiveBrush] : (io::IsMouseInRect(rc) ? g_brushs[ButtonHoveredBrush] : g_brushs[ButtonBrush]);
			g_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

			// ���������
			if (io::IsMouseInRect(rc)) {
				if (io::IsMouseClicked(0)) {
					// ��¼�������ʱ�����X����
					initialMouseX = io::MousePos.x;

					// ��������
					pActiveValue = &value;
				}

				// �����̰���״̬
				if (io::IsKeyPressed(VK_LEFT)) {
					value -= 1;
					value = max(value, minValue);
				}
				if (io::IsKeyPressed(VK_RIGHT)) {
					value += 1;
					value = min(value, maxValue);
				}

			}

			if (io::IsMouseReleased[0] && pActiveValue == &value) {
				pActiveValue = nullptr; // �����ؼ�ָ��
			}
			else if (pActiveValue == &value && io::IsMouseDown[0]) {
				// �������ˮƽ�ƶ����룬�������ƶ��������ֵ
				float deltaX = io::MousePos.x - initialMouseX;
				int increment = static_cast<int>(deltaX);
				value += increment;

				// ȷ��ֵ��ָ����Χ��
				value = min(max(value, minValue), maxValue);

				// ���³�ʼ���X���꣬�Ա���һ�μ���
				initialMouseX = io::MousePos.x;
			}

			// ���Ƶ�ǰֵ
			std::wstring displayText = std::to_wstring(value);
			g_pRenderTarget->DrawTextW(displayText.c_str(), displayText.length(), g_pTextFormat, rc, g_brushs[TextBrush]);

			// ���Ʊ��⣨����У�
			if (!label.empty()) {
				g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				g_pRenderTarget->DrawTextW(label.c_str(), label.length(), g_pTextFormat, D2D1::RectF(x + w - labelW+5, y, x + w, y + h), g_brushs[TextBrush]);
				g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			}
		}
	}
	void DragFloat(float& value, const std::wstring& label = L"", float minValue = 0.0f, float maxValue = 100.0f) {
		static float* pActiveValue = nullptr; // ָ��ǰ��ؼ���ָ��
		static float initialMouseX = 0.0f;    // ��¼�������ʱ�����X����
		const float labelW = 70;

		float x, y, w = 300, h = 15; // �ؼ���Ĭ�ϳߴ�
		if (io::ClientWidth <= 300 + g_PaddingX * 2) w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);

		if (IsWidgetInClient(x, y, w, h)) {
			D2D1_RECT_F rc = D2D1::RectF(x, y, x + w - labelW, y + h);
			D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(rc, g_Round, g_Round);

			// ���Ʊ���
			ID2D1SolidColorBrush* pBrush = io::IsMouseInRect(rc) && io::IsMouseDown[0] ? g_brushs[ButtonActiveBrush] : (io::IsMouseInRect(rc) ? g_brushs[ButtonHoveredBrush] : g_brushs[ButtonBrush]);
			g_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

			// ���������
			if (io::IsMouseInRect(rc)) {
				if (io::IsMouseClicked(0)) {
					// ��¼�������ʱ�����X����
					initialMouseX = io::MousePos.x;

					// ��������
					pActiveValue = &value;
				}

				// �����̰���״̬
				if (io::IsKeyPressed(VK_LEFT)) {
					value -= 1.0f;
					value = max(value, minValue);
				}
				if (io::IsKeyPressed(VK_RIGHT)) {
					value += 1.0f;
					value = min(value, maxValue);
				}
			}

			if (io::IsMouseReleased[0] && pActiveValue == &value) {
				pActiveValue = nullptr; // �����ؼ�ָ��
			}
			else if (pActiveValue == &value && io::IsMouseDown[0]) {
				// �������ˮƽ�ƶ����룬�������ƶ��������ֵ
				float deltaX = io::MousePos.x - initialMouseX;
				float increment = deltaX;
				value += increment;

				// ȷ��ֵ��ָ����Χ��
				value = max(min(value, maxValue), minValue);

				// ���³�ʼ���X���꣬�Ա���һ�μ���
				initialMouseX = io::MousePos.x;
			}

			// ���Ƶ�ǰֵ
			std::wstring displayText = std::to_wstring(value);
			g_pRenderTarget->DrawTextW(displayText.c_str(), displayText.length(), g_pTextFormat, rc, g_brushs[TextBrush]);

			// ���Ʊ��⣨����У�
			if (!label.empty()) {
				g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				g_pRenderTarget->DrawTextW(label.c_str(), label.length(), g_pTextFormat, D2D1::RectF(x + w - labelW + 5, y, x + w, y + h), g_brushs[TextBrush]);
				g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			}
		}
	}





}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, io::WndProc, 0, 0, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, L"Example", NULL };
	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME, L"Example", L"D2D", WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE, 600, 400, 500, 400, NULL, NULL, hInstance, NULL);



	//SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	//SetLayeredWindowAttributes(hwnd, 0, 233, LWA_ALPHA);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}