#pragma once
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <shobjidl.h> 
#include <sstream>
#include <imm.h>  // 输入法管理相关头文件
#include <vector>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Imm32.lib")

// 控件渲染循环
void Render();

// 【简单代码设计原则】

// 2024-1-2   7:00  完成拖拽按钮 简单控制台 增加选择渲染，不在视图不渲染
// 2024-1-1   15:20 完整优化了单选编辑框
// 2024-1-1	  10:10 整理IO数据结构优化代码
// 2024-1-1	  7:00  完成标签部件可格式化输出
// 2023-12-31 6:50  基本实现编辑框
// 2023-12-28 22:50 颜色结构重构
// 2023-12-28 20:00 自动布局插入空行空列


// DirectX资源
namespace ui{
	//-------------------------------------------------------------------------
	// [资源] D2D
	//-------------------------------------------------------------------------
	static ID2D1Factory*			g_pFactory = NULL;
	static ID2D1HwndRenderTarget*	g_pRenderTarget = NULL;

	// 初始化D2D设备
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

	// 释放D2D资源
	void ReleaseD2D(){
		if (g_pFactory) g_pFactory->Release();
		if (g_pRenderTarget) g_pRenderTarget->Release();
	}

	// 调节D2D渲染目标大小
	void Resize(int clientW, int clientH){
		g_pRenderTarget->Resize(D2D1::SizeU(clientW, clientH));
	}


	//-------------------------------------------------------------------------
	// [资源] 画刷
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
		BrushCount // 用于表示笔刷数量
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
		D2D1::ColorF(75, 90, 110, 1)		// EditBoxBackgroundActiveBrush 选择区域颜色
	};
	//86,103,126
	ID2D1SolidColorBrush* g_brushs[BrushCount];

	// 初始化颜色资源
	void InitColors() {
		for (int i = 0; i < BrushCount; ++i) {
			D2D1_COLOR_F rgb = colorMapping[i];
			g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rgb.r / 255.0f, rgb.g / 255.0f, rgb.b / 255.0f, rgb.a), &g_brushs[i]);
		}
	}

	// 释放颜色资源
	void ReleaseColors() {
		for (int i = 0; i < BrushCount; ++i) {
			if (g_brushs[i]) {
				g_brushs[i]->Release();
				g_brushs[i] = nullptr;
			}
		}
	}


	//-------------------------------------------------------------------------
	// [资源] 字体
	//-------------------------------------------------------------------------
	static IDWriteFactory*			g_pDWriteFactory = NULL;
	static IDWriteTextFormat*		g_pTextFormat = NULL;
	static IDWriteFactory*			g_pDWriteFactory2 = NULL;
	static IDWriteTextFormat*		g_pTextFormat2 = NULL;

	// 初始化字体资源
	void InitFont(){

		// 初始化字体
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
		g_pDWriteFactory->CreateTextFormat(
			L"微软雅黑",            // 字体名称
			NULL,                   // 字体集
			DWRITE_FONT_WEIGHT_THIN, //DWRITE_FONT_WEIGHT_NORMAL 
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			12.0f,                  // 字体大小
			L"",                    // 语言
			&g_pTextFormat
			);

		// 设置文本居中对齐
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		g_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		// 设置如何裁剪溢出布局框的文本
		DWRITE_TRIMMING trimmingOptions;
		trimmingOptions.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
		trimmingOptions.delimiter = 0;
		trimmingOptions.delimiterCount = 0;
		g_pTextFormat->SetTrimming(&trimmingOptions, nullptr);

		//================
		// 初始化字体
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory2));
		g_pDWriteFactory2->CreateTextFormat(
			L"微软雅黑",            // 字体名称
			NULL,                   // 字体集
			DWRITE_FONT_WEIGHT_THIN, //DWRITE_FONT_WEIGHT_NORMAL 
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			12.0f,                  // 字体大小
			L"",                    // 语言
			&g_pTextFormat2
			);

		// 设置文本居中对齐
		g_pTextFormat2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); // 左对齐
		g_pTextFormat2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); // 垂直居中对齐

		// 设置如何裁剪溢出布局框的文本
		DWRITE_TRIMMING trimmingOptions2;
		trimmingOptions2.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
		trimmingOptions2.delimiter = 0;
		trimmingOptions2.delimiterCount = 0;
		g_pTextFormat2->SetTrimming(&trimmingOptions2, nullptr);
	}

	// 释放字体资源
	void ReleaseFont(){
		if (g_pTextFormat) g_pTextFormat->Release();
		if (g_pDWriteFactory) g_pDWriteFactory->Release();

		if (g_pTextFormat2) g_pTextFormat2->Release();
		if (g_pDWriteFactory2) g_pDWriteFactory2->Release();
	}

}

// 获取设备交互
namespace io {
	// [全局数据] 
	HWND	Hwnd = nullptr;
	int		ClientWidth, ClientHeight;
	float	MouseWheel = 0.0f;
	bool	IsKeysDown[256] = { false };	// 键盘按键状态
	bool	IsMouseDown[5] = { false };		// 鼠标按键状态 0左键 1右键 2中键 3后退 4前进
	bool	IsMouseReleased[5] = { true };
	POINT	MousePos = { -1, -1 };			// 客户端鼠标位置
	std::wstring ch = L"";					// 输入的字符每次绘画结束记得清空
	POINT	CursorScreenPos = { 0, 0 };		// 编辑框光标位于屏幕坐标，放在编辑框函数里更新，给输入法预选词框位置作为依据

	// [更新IO数据] 
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			SetFocus(hwnd);	//设置下焦点不然启动窗口不会激活输入法
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
			// 如果未按下CTRL且wParam非VK_BACK，处理字符输入
			if (!(GetKeyState(VK_CONTROL) & 0x8000) && wParam != VK_BACK) {
				ch += (wParam == VK_RETURN) ? L'\r\n' : static_cast<wchar_t>(wParam); // Enter插入换行，否则添加字符
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
			// 清理资源
			ui::ReleaseD2D();
			ui::ReleaseColors();
			ui::ReleaseFont();
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	// [辅助函数]
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
		// 跟踪键盘按键按下一次的状态
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
		// 尝试打开剪贴板
		if (!OpenClipboard(NULL)) {
			OutputDebugString(L"无法打开剪贴板。\n");
			return L"";
		}

		std::wstring text;
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData != NULL) {
			// 锁定剪贴板数据并检查其有效性
			wchar_t* buffer = static_cast<wchar_t*>(GlobalLock(hData));
			if (buffer != NULL) {
				// 安全措施：限制读取的文本长度
				size_t maxLen = 1024; // 或者您可以选择一个合适的最大长度
				text.assign(buffer, wcsnlen(buffer, maxLen));

				GlobalUnlock(hData);
			}
			else {
				OutputDebugString(L"无法锁定剪贴板数据。\n");
			}
		}
		else {
			OutputDebugString(L"未找到剪贴板数据。\n");
		}

		CloseClipboard();
		return text;
	}
}

// 小部件（Widgets）
namespace ui{
	float	g_Round = 3.5f;		//矩形圆角
	float	g_WidgetX = 0.0f;	//控件自动布局
	float	g_WidgetY = 0.0f;
	float	g_WidgetW = 0.0f;
	float	g_WidgetH = 0.0f;
	bool	g_SameLine = false;	//下一个控件是否同一行
	float	g_PaddingX = 10.0f;
	float	g_PaddingY = 20.0f;
	float	g_MarginX = 5.0f;
	float	g_MarginY = 5.0f;
	float	g_LabelW = 50;
	float	g_LabelH = 20;
	float	g_ButtonW = 75.0f;
	float	g_ButtonH = 23.0f;

	// 调试函数
	void AsyncBeep(){
		CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
			Beep(55555, 55);
			return 0;
		}, NULL, 0, NULL);
	}

	void PrintFPS() {
		static DWORD lastTime = 0; // 上次调用的时间
		static int frameCount = 0; // 帧数计数器

		// 获取当前时间
		DWORD currentTime = GetTickCount64();

		// 计算自上次调用以来经过的时间（以毫秒为单位）
		DWORD timeDiff = currentTime - lastTime;

		// 累计帧数
		frameCount++;

		// 如果时间差大于1000毫秒（1秒），计算FPS
		if (timeDiff >= 1000) {
			float fps = frameCount * 1000.0f / timeDiff; // 计算FPS

			// 构造输出字符串
			wchar_t output[256];
			swprintf(output, 256, L"FPS: %.2f\n", fps);
			OutputDebugStringW(output);

			// 重置计数器和时间
			frameCount = 0;
			lastTime = currentTime;
		}
	}

	// 一切的开始
	void Begin(){
		ui::PrintFPS();
		Sleep(1);
		g_pRenderTarget->BeginDraw();
		//g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		g_pRenderTarget->Clear(D2D1::ColorF(48 / 255.0f, 63 / 255.0f, 79 / 255.0f, 1));

		// 控件自动布局位置重置
		g_WidgetX = 0.0f;
		g_WidgetY = 0.0f;
		g_WidgetW = 0.0f;
		g_WidgetH = 0.0f;


	}
	void End(){
		g_pRenderTarget->EndDraw();
		io::ch.clear();
	}

	// 动画实现
	void BeginAnimateScale(D2D1_ROUNDED_RECT roundedRect) {
		if (io::IsMouseInRect(roundedRect)) {
			static DWORD lastTick = 0;			// 上次更新的时间
			static float currentScale = 1.0f;	// 当前缩放比例

			// 计算缩放动画
			if (io::IsMouseInRect(roundedRect)) {
				DWORD currentTick = GetTickCount64();
				const float scaleSpeed = 0.002f;	// 缩放速度
				const float minScale = 0.95f;		// 最小缩放比例
				const float maxScale = 1.0f;		// 最大缩放比例

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

			// 计算圆角矩形的中心点
			D2D1_POINT_2F rectCenter = D2D1::Point2F(
				(roundedRect.rect.left + roundedRect.rect.right) / 2.0f,
				(roundedRect.rect.top + roundedRect.rect.bottom) / 2.0f
				);

			// 设置变换矩阵
			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(currentScale, currentScale, rectCenter));
		}
	}
	void EndAnimateScale(){
		// 重置变换矩阵
		g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	// 自动布局实现
	void AutoLayout(float& x, float& y, float& w, float& h) {
		// 检查是否需要换行：当前行宽度加上下一个控件宽度是否超过窗口宽度
		if (g_SameLine && (g_WidgetX + w + g_MarginX > io::ClientWidth)) {
			// 需要换行
			g_SameLine = false;
			g_WidgetX = 0;
			g_WidgetY += g_WidgetH;
			g_WidgetH = 0;
		}

		if (g_SameLine) {
			// 同一行中的下一个控件：在x坐标上添加水平间距
			x = g_WidgetX + g_MarginX;
			y = g_WidgetY;
		}
		else {
			// 新行的第一个控件：处理x坐标和y坐标
			x = g_PaddingX; // 在所有行的第一个控件左侧添加内边距
			y = (g_WidgetY == 0 && g_WidgetX == 0) ? g_PaddingY : g_WidgetY + g_WidgetH + g_MarginY;

			// 重置行高为新行的第一个控件高度
			g_WidgetH = h;
		}

		// 更新全局位置状态，为下一个控件准备
		g_WidgetX = x + w;
		g_WidgetY = y;

		// 重置NextSameLine标志
		g_SameLine = false;
	}

	// 判断控件是否至少部分在窗口的客户区内
	bool IsWidgetInClient(float x, float y, float w, float h) {
		// 检查控件右下角坐标是否在窗口左上角的左侧或上方
		if (x + w < 0 || y + h < 0) {
			return false;
		}

		// 检查控件左上角坐标是否在窗口右下角的右侧或下方
		if (x > io::ClientWidth || y > io::ClientHeight) {
			return false;
		}

		// 如果以上条件都不满足，表示控件至少部分在客户区内
		return true;
	}

	// 下个部件为同一行
	void SameLine(){
		g_SameLine = true;
	}
	// 插入空行
	void NewLine(float height = 0.0f) {
		// 如果当前行已有控件，则换行
		if (g_WidgetX != 0.0f) {
			g_WidgetX = 0.0f; // 重置行的水平起点

			// 如果传入参数为0，则使用全局变量g_widget_h；否则使用传入的参数
			if (height == 0.0f) {
				g_WidgetY += g_MarginY;
			}
			else {
				g_WidgetY += height + g_MarginY;
			}
		}
		// 如果当前行没有控件，不执行操作（已处于新行的起始位置）
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
	//[x] BeginComboBox|EndComboBox|ComboBoxItem +需要实现滚动条和页面
	//[x] ColorPicker
	//[x] Help
	//[x] BeginMenu|EndMenu|MenuItem
	//[x] BeginPopup|EndPopup 毫无思路
	//[x] BeginTable|EndTable 可以加增删页
	//DPI实现

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

	bool Button(std::wstring text = L"按钮") {
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

		// 判断鼠标是否点击了复选框
		bool clicked = io::IsMouseClickedInRect(allRoundedRect);
		if (clicked) {
			isChecked = !isChecked; // 切换复选框的选中状态
		}

		// 绘制复选框
		g_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

		if (isChecked) {
			// 定义水平镜像的"L"形状的两个矩形
			float rectWidth = 2.5f;  // 矩形宽度
			float shortRectWidth = h / 4.0f;  // 较短矩形的宽度
			float longRectHeight = h / 2.0f;  // 较长矩形的高度

			D2D1_RECT_F shortRect = D2D1::RectF(x + h - shortRectWidth - 5, y + h / 2, x + h - 5, y + h / 2 + rectWidth);
			D2D1_RECT_F longRect = D2D1::RectF(x + h / 2 - rectWidth, y + h / 2, x + h / 2, y + h + 3);

			// 创建变换层
			ID2D1Layer* pLayer = NULL;
			g_pRenderTarget->CreateLayer(NULL, &pLayer);
			g_pRenderTarget->PushLayer(D2D1::LayerParameters(), pLayer);

			// 应用旋转变换（90度）
			D2D1_POINT_2F center = D2D1::Point2F(x + h / 2 - 1, y + h / 2 + 3);
			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(225, center));

			// 绘制水平镜像的"L"形状
			g_pRenderTarget->FillRectangle(shortRect, g_brushs[TextBrush]);
			g_pRenderTarget->FillRectangle(longRect, g_brushs[TextBrush]);

			// 重置变换并移除层
			g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			g_pRenderTarget->PopLayer();

			// 清理
			if (pLayer) {
				pLayer->Release();
			}
		}

		// 绘制文本标签
		D2D1_RECT_F textRect = D2D1::RectF(x + h + g_MarginX, y, x + w, y + h);
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		g_pRenderTarget->DrawTextW(text.c_str(), text.length(), g_pTextFormat, textRect, g_brushs[TextBrush]);
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		return clicked;
	}

	void EditBox(std::wstring& editText) {
		static int selectionStart = -1;			// 选择开始位置，-1 表示没有选择
		static int selectionEnd = -1;			// 选择结束位置，-1 表示没有选择
		static int cursorPosition = 0;			// 光标位置，表示当前光标在文本中的索引位置
		static bool cursorVisible = true;		// 控制光标的可见性状态，true 表示光标可见
		static int activeEditBoxKey = -1;		// 初始值为-1，表示没有编辑框被激活

		// 自动布局
		float x = 0, y = 0, w = 300, h = 23;
		if (io::ClientWidth <= 300 + g_PaddingX * 2)w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return;

		// 创建矩形区域用于绘制文本
		D2D1_RECT_F layoutRect = D2D1::RectF(x, y, x + w, y + h);

		if (io::IsMouseInRect(layoutRect))
		{
			// 设置鼠标光标
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
		}

		// 生成密钥
		int editBoxKey = static_cast<int>(x) ^ static_cast<int>(y);

		// 检查鼠标是否点击编辑框
		if (io::IsMouseDown[0]) {
			if (io::IsMouseInRect(layoutRect)) {
				// 鼠标点击了这个编辑框，激活它
				activeEditBoxKey = editBoxKey;
			}
			else if (activeEditBoxKey == editBoxKey) {
				// 鼠标点击在这个编辑框外部，取消激活
				activeEditBoxKey = -1;
			}
		}

		// 检查鼠标是否在编辑框内
		bool isActiveEditBoxKey = activeEditBoxKey == editBoxKey;

		// 创建TextLayout对象用于整个函数
		IDWriteTextLayout* pTextLayout = nullptr;
		HRESULT hr = g_pDWriteFactory->CreateTextLayout(editText.c_str(), editText.length(), g_pTextFormat2, w, h, &pTextLayout);
		if (SUCCEEDED(hr)) {
			if (isActiveEditBoxKey)
			{
				// 检查鼠标是否在编辑框内并更新字符串
				if (!io::ch.empty()) {

					// 限制为单行输入
					if (io::ch.find(L'\r') != std::wstring::npos) {
						io::ch.clear();
					}

					// 检查是否有选择的文本
					bool hasSelection = (selectionStart != -1) && (selectionEnd != -1) && (selectionStart != selectionEnd);
					size_t charsToRemove = hasSelection ? abs(selectionEnd - selectionStart) : 0;
					size_t newLength = editText.length() + io::ch.length() - charsToRemove;

					// 确保新长度不超过字符串最大长度
					if (newLength <= editText.max_size()) {
						if (hasSelection) {
							int selStart = min(selectionStart, selectionEnd);
							int selEnd = max(selectionStart, selectionEnd);
							editText.replace(selStart, selEnd - selStart, io::ch);
							cursorPosition = selStart + io::ch.length();
						}
						else {
							// 确保光标位置有效
							cursorPosition = (cursorPosition > editText.length()) ? editText.length() : cursorPosition;
							editText.insert(cursorPosition, io::ch);
							cursorPosition += io::ch.length();
						}
						// 清空已处理的字符
						io::ch.clear();
						// 重置选择区域
						selectionStart = selectionEnd = -1;
					}
				}


				// ========== 鼠标点击编辑框和选择中 ==========
				if (io::IsMouseDown[0] || io::IsMouseClicked(0)) {
					BOOL isTrailingHit, isInside;
					DWRITE_HIT_TEST_METRICS metrics;

					// 调整鼠标坐标，使其相对于编辑框
					float adjustedX = static_cast<float>(io::MousePos.x) - x;
					float adjustedY = static_cast<float>(io::MousePos.y) - y;

					// 使用调整后的坐标获取光标位置
					pTextLayout->HitTestPoint(adjustedX, adjustedY, &isTrailingHit, &isInside, &metrics);

					if (isInside || isTrailingHit) {
						cursorPosition = metrics.textPosition + (isTrailingHit ? 1 : 0);
						cursorPosition = min(cursorPosition, static_cast<int>(editText.length()));
					}

					if (io::IsMouseClicked(0)) {
						// 初始化选择区域为光标位置
						selectionStart = selectionEnd = cursorPosition;
					}
					else if (io::IsMouseDown[0]) {
						// 更新选择区域
						selectionEnd = cursorPosition;
					}
				}

				// ========== 更新输入预选词窗口位置 ==========
				DWRITE_HIT_TEST_METRICS metrics;
				float cursorX, cursorY;
				if (SUCCEEDED(pTextLayout->HitTestTextPosition(cursorPosition, false, &cursorX, &cursorY, &metrics))) {
					POINT pt = { static_cast<LONG>(x + cursorX), static_cast<LONG>(y + cursorY) };
					io::CursorScreenPos = pt;

					// 更新输入法候选框位置
					HIMC hIMC = ImmGetContext(io::Hwnd);
					if (hIMC) {
						COMPOSITIONFORM cf = { CFS_POINT, io::CursorScreenPos };
						ImmSetCompositionWindow(hIMC, &cf);
						ImmReleaseContext(io::Hwnd, hIMC);
					}
				}


				// ========== 光标左右键移动 ==========
				static DWORD lastArrowPressTime = 0; // 上次按下箭头键的时间
				static DWORD lastCursorMoveTime = 0; // 上次移动光标的时间
				DWORD currentTime = GetTickCount64();
				bool isRapidMovement = (currentTime - lastArrowPressTime > 500); // 快速移动模式
				// 处理左右键光标移动
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

				// ========== 退格键删除字符 ==========
				static DWORD lastBackspacePressTime = 0; // 上次按下退格键的时间
				static DWORD lastCharDeleteTime = 0;     // 上次删除字符的时间

				if (io::IsKeysDown[VK_BACK] && cursorPosition > 0) {
					DWORD currentTime = GetTickCount64();
					bool isRapidDeletion = (currentTime - lastBackspacePressTime > 500); // 快速删除模式
					bool hasSelection = (selectionStart != -1) && (selectionEnd != -1) && (selectionStart != selectionEnd);

					// 删除选中文本或单个字符
					if (hasSelection) {
						int selStart = min(selectionStart, selectionEnd);
						int selEnd = max(selectionStart, selectionEnd);
						editText.erase(selStart, selEnd - selStart);
						cursorPosition = selStart;
					}
					else if (cursorPosition > 0 && (isRapidDeletion || currentTime - lastCharDeleteTime >= 40)) {
						editText.erase(--cursorPosition, 1);
					}

					// 更新时间记录
					lastBackspacePressTime = (lastBackspacePressTime == 0) ? currentTime : lastBackspacePressTime;
					lastCharDeleteTime = currentTime;

					// 重置选择区域
					selectionStart = selectionEnd = -1;
				}
				else {
					// 重置时间记录
					lastBackspacePressTime = lastCharDeleteTime = 0;
				}

				// ========== 按键相关处理 ==========
				if (GetAsyncKeyState(VK_CONTROL))
				{
					// ========== Ctrl + A 全选 ==========
					if (io::IsKeyPressed('A')) {
						selectionStart = 0;
						selectionEnd = editText.length();
					}

					// ========== Ctrl + V 粘贴 ==========
					if (io::IsKeyPressed('V'))
					{
						std::wstring clipboardText = io::GetClipboardText();
						bool hasSelection = (selectionStart != -1) && (selectionEnd != -1) && (selectionStart != selectionEnd);
						size_t charsToRemove = hasSelection ? abs(selectionEnd - selectionStart) : 0;
						size_t newLength = editText.length() + clipboardText.length() - charsToRemove;

						// 确保新长度不超过字符串最大长度
						if (newLength <= editText.max_size()) {
							if (hasSelection) {
								int selStart = min(selectionStart, selectionEnd);
								int selEnd = max(selectionStart, selectionEnd);
								editText.replace(selStart, selEnd - selStart, clipboardText);
								cursorPosition = selStart + clipboardText.length();
							}
							else {
								// 确保光标位置有效
								cursorPosition = (cursorPosition > editText.length()) ? editText.length() : cursorPosition;
								editText.insert(cursorPosition, clipboardText);
								cursorPosition += clipboardText.length();
							}
							// 重置选择区域
							selectionStart = selectionEnd = -1;
						}
					}

					// ========== Ctrl + C 复制==========
					// 选中文本的起始和结束位置
					int selStart = min(selectionStart, selectionEnd);
					int selEnd = max(selectionStart, selectionEnd);

					// 复制：CTRL+C
					if (io::IsKeyPressed('C') && selStart != selEnd) {
						std::wstring selectedText = editText.substr(selStart, selEnd - selStart);
						io::SetClipboard(selectedText);
					}

					// ========== Ctrl + V 剪贴 ==========
					if (io::IsKeyPressed('X'))
					{
						// 确保选择的起始和结束位置有效
						if (selStart != -1 && selEnd != -1 && selStart != selEnd) {
							// 确保选择范围在字符串长度内
							selStart = max(0, selStart);
							selEnd = min(static_cast<int>(editText.length()), selEnd);

							if (selEnd > selStart) {
								std::wstring selectedText = editText.substr(selStart, selEnd - selStart);
								io::SetClipboard(selectedText);
								editText.erase(selStart, selEnd - selStart);
								cursorPosition = selStart;

								// 重置选择区域
								selectionStart = selectionEnd = -1;
							}
						}
						else {
							OutputDebugString(L"无效的选择区域。\n");
						}
					}

				}
			}

			// ========== 绘制编辑框部分 ==========

			// 绘制编辑框底色
			ID2D1SolidColorBrush* pBrush = isActiveEditBoxKey ? g_brushs[EditBoxBackgroundActiveBrush] : (io::IsMouseInRect(layoutRect) ? g_brushs[EditBoxBackgroundHovereBrush] : g_brushs[EditBoxBackgroundBrush]);

			g_pRenderTarget->FillRoundedRectangle(D2D1::RoundedRect(layoutRect, g_Round, g_Round), pBrush);

			// 绘制未选中的文本（正常颜色）
			g_pRenderTarget->DrawTextW(editText.c_str(), editText.length(), g_pTextFormat2, layoutRect, g_brushs[TextBrush]);

			// 绘制选中文本的高亮背景和白色文本
			if (selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd && isActiveEditBoxKey) {
				int selStart = min(selectionStart, selectionEnd);
				int selEnd = max(selectionStart, selectionEnd);

				DWRITE_HIT_TEST_METRICS hitTestMetrics;
				float selStartX, selStartY;
				float selEndX, selEndY;

				// 获取选中文本的起始和结束坐标
				pTextLayout->HitTestTextPosition(selStart, false, &selStartX, &selStartY, &hitTestMetrics);
				pTextLayout->HitTestTextPosition(selEnd, false, &selEndX, &selEndY, &hitTestMetrics);

				D2D1_RECT_F highlightRect = D2D1::RectF(x + selStartX, y + hitTestMetrics.top, x + selEndX, y + hitTestMetrics.top + hitTestMetrics.height);
				D2D1_ROUNDED_RECT roundedRect = { highlightRect, 2, 2 };
				g_pRenderTarget->FillRoundedRectangle(roundedRect, g_brushs[EditBoxSelectBrush]);

				std::wstring selectedText = editText.substr(selStart, selEnd - selStart);
				g_pRenderTarget->DrawTextW(selectedText.c_str(), selectedText.length(), g_pTextFormat2, highlightRect, g_brushs[TextWhiteBrush]);
			}

			// 绘制光标
			if (cursorVisible && isActiveEditBoxKey && (selectionStart == -1 || selectionEnd == -1 || selectionStart == selectionEnd)) {
				DWRITE_HIT_TEST_METRICS metrics;
				float cursorX, cursorY;
				pTextLayout->HitTestTextPosition(cursorPosition, false, &cursorX, &cursorY, &metrics);

				cursorX += x; // 考虑编辑框的 x 偏移
				cursorY += y; // 考虑编辑框的 y 偏移

				// 绘制光标
				g_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
				g_pRenderTarget->DrawLine(D2D1::Point2F(cursorX, cursorY), D2D1::Point2F(cursorX, cursorY + metrics.height), g_brushs[EditBoxCursorBrush], 2.0f);
				g_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			}


			// 释放TextLayout对象
			if (pTextLayout)pTextLayout->Release();

		}
		else {
			OutputDebugString(L"TextLayout 创建失败。\n");
		}
	}

	// 控制台 创建多个可以用字符数组做形参 自己用一个就好
	static std::vector<std::wstring> g_logEntries; // 存储日志条目的容器
	void Log() {
		float x = 0, y = 0, w = 300, h = 100; // 日志区域的尺寸和位置
		if (io::ClientWidth <= 300 + g_PaddingX * 2)w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);
		if (!IsWidgetInClient(x, y, w, h))return;

		D2D1_RECT_F logRect = D2D1::RectF(x, y, x + w, y + h);
		D2D1_ROUNDED_RECT logRectBG = D2D1::RoundedRect(logRect, g_Round, g_Round);

		// 绘制背景
		g_pRenderTarget->FillRoundedRectangle(logRectBG, g_brushs[EditBoxBackgroundBrush]);

		// 设置文本格式为左对齐
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

		// 计算可以显示的行数
		int maxLines = static_cast<int>(h / 20); // 假设每行高度为20像素

		// 确定从哪一行开始绘制
		int startLine = max(0, static_cast<int>(g_logEntries.size()) - maxLines);

		// 绘制每条日志
		float lineSpacing = 20; // 行间距
		float currentY = y + h - maxLines * lineSpacing; // 从底部开始绘制
		for (int i = startLine; i < g_logEntries.size(); ++i) {
			D2D1_RECT_F textRect = D2D1::RectF(x + 5, currentY, x + w - 15, currentY + lineSpacing);
			textRect.top = max(textRect.top, y);

			g_pRenderTarget->DrawTextW(g_logEntries[i].c_str(), g_logEntries[i].length(), g_pTextFormat, textRect, g_brushs[TextBrush]);

			currentY += lineSpacing;

			// 如果超出绘制区域，终止循环
			if (currentY > y + h) {
				break;
			}
		}

		// 恢复文本格式的默认对齐方式
		g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	void LogAdd(const wchar_t* fmt, ...) {
		wchar_t buffer[1024];
		int offset = 0;

		// 如果启用时间戳
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

	// 拖拽部件
	void DragInt(int& value, const std::wstring& label = L"", int minValue = 0, int maxValue = 100) {
		static int* pActiveValue = nullptr; // 指向当前活动控件的指针
		static float initialMouseX = 0.0f;  // 记录左键按下时的鼠标X坐标
		const float labelW = 70;

		float x, y, w = 300, h = 15; // 控件的默认尺寸
		if (io::ClientWidth <= 300 + g_PaddingX * 2)w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);

		if (IsWidgetInClient(x, y, w, h)) {
			D2D1_RECT_F rc = D2D1::RectF(x, y, x + w - labelW, y + h);
			D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(rc, g_Round, g_Round);

			// 绘制背景
			ID2D1SolidColorBrush* pBrush = io::IsMouseInRect(rc) && io::IsMouseDown[0] ? g_brushs[ButtonActiveBrush] : (io::IsMouseInRect(rc) ? g_brushs[ButtonHoveredBrush] : g_brushs[ButtonBrush]);
			g_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

			// 检测鼠标操作
			if (io::IsMouseInRect(rc)) {
				if (io::IsMouseClicked(0)) {
					// 记录左键按下时的鼠标X坐标
					initialMouseX = io::MousePos.x;

					// 单击激活
					pActiveValue = &value;
				}

				// 检测键盘按键状态
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
				pActiveValue = nullptr; // 清除活动控件指针
			}
			else if (pActiveValue == &value && io::IsMouseDown[0]) {
				// 计算鼠标水平移动距离，并根据移动方向调整值
				float deltaX = io::MousePos.x - initialMouseX;
				int increment = static_cast<int>(deltaX);
				value += increment;

				// 确保值在指定范围内
				value = min(max(value, minValue), maxValue);

				// 更新初始鼠标X坐标，以便下一次计算
				initialMouseX = io::MousePos.x;
			}

			// 绘制当前值
			std::wstring displayText = std::to_wstring(value);
			g_pRenderTarget->DrawTextW(displayText.c_str(), displayText.length(), g_pTextFormat, rc, g_brushs[TextBrush]);

			// 绘制标题（如果有）
			if (!label.empty()) {
				g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				g_pRenderTarget->DrawTextW(label.c_str(), label.length(), g_pTextFormat, D2D1::RectF(x + w - labelW+5, y, x + w, y + h), g_brushs[TextBrush]);
				g_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			}
		}
	}
	void DragFloat(float& value, const std::wstring& label = L"", float minValue = 0.0f, float maxValue = 100.0f) {
		static float* pActiveValue = nullptr; // 指向当前活动控件的指针
		static float initialMouseX = 0.0f;    // 记录左键按下时的鼠标X坐标
		const float labelW = 70;

		float x, y, w = 300, h = 15; // 控件的默认尺寸
		if (io::ClientWidth <= 300 + g_PaddingX * 2) w = io::ClientWidth - g_PaddingX * 2;
		AutoLayout(x, y, w, h);

		if (IsWidgetInClient(x, y, w, h)) {
			D2D1_RECT_F rc = D2D1::RectF(x, y, x + w - labelW, y + h);
			D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(rc, g_Round, g_Round);

			// 绘制背景
			ID2D1SolidColorBrush* pBrush = io::IsMouseInRect(rc) && io::IsMouseDown[0] ? g_brushs[ButtonActiveBrush] : (io::IsMouseInRect(rc) ? g_brushs[ButtonHoveredBrush] : g_brushs[ButtonBrush]);
			g_pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);

			// 检测鼠标操作
			if (io::IsMouseInRect(rc)) {
				if (io::IsMouseClicked(0)) {
					// 记录左键按下时的鼠标X坐标
					initialMouseX = io::MousePos.x;

					// 单击激活
					pActiveValue = &value;
				}

				// 检测键盘按键状态
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
				pActiveValue = nullptr; // 清除活动控件指针
			}
			else if (pActiveValue == &value && io::IsMouseDown[0]) {
				// 计算鼠标水平移动距离，并根据移动方向调整值
				float deltaX = io::MousePos.x - initialMouseX;
				float increment = deltaX;
				value += increment;

				// 确保值在指定范围内
				value = max(min(value, maxValue), minValue);

				// 更新初始鼠标X坐标，以便下一次计算
				initialMouseX = io::MousePos.x;
			}

			// 绘制当前值
			std::wstring displayText = std::to_wstring(value);
			g_pRenderTarget->DrawTextW(displayText.c_str(), displayText.length(), g_pTextFormat, rc, g_brushs[TextBrush]);

			// 绘制标题（如果有）
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