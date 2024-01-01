#include "ui.h"

void LOG(std::wstring text){
	OutputDebugStringW(text.c_str());
}


void Render()
{
	ui::Begin();



	ui::Label(L"💜💙💛 Aries");
	ui::NewLine(10);

	ui::Label(L"鼠标位置：%d,%d", io::MousePos.x, io::MousePos.y);
	ui::DragFloat(ui::g_Round, L"圆角", 0, 10);
	ui::DragFloat(ui::g_PaddingX, L"间距X", 0, 50);
	ui::DragFloat(ui::g_PaddingY, L"间距Y", 0, 50);

	static std::wstring str = L"你好世界";
	static std::wstring str2 = L"";


	ui::Label(L"编辑框:");
	ui::SameLine();
	ui::EditBox(str);
	ui::Label(L"编辑框:");
	ui::SameLine();
	ui::EditBox(str2);

	ui::Button(L"白雪");
	ui::SameLine();
	if (ui::Button(L"叮铃"))ui::AsyncBeep();
	ui::SameLine();
	if (ui::Button(L"满天星"))
	{
		LOG(L"满天星");
	}

	ui::Label(L"2023年12月31日");
	ui::NewLine(10);

	static bool a, b, c = true;
	if (ui::CheckBox(L"再来一局", a))ui::AsyncBeep();

	ui::SameLine();
	if (ui::CheckBox(L"Transparent", b))
	{
		if (b)
		{
			SetWindowLong(io::Hwnd, GWL_EXSTYLE, GetWindowLong(io::Hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(io::Hwnd, 0, 233, LWA_ALPHA);
		}
		else
		{
			SetWindowLong(io::Hwnd, GWL_EXSTYLE, GetWindowLong(io::Hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(io::Hwnd, 0, 255, LWA_ALPHA);
		}

	}

	ui::SameLine();
	if (ui::CheckBox(L"QWQ", c))
	{
		ui::AsyncBeep();
		if (c)a = true;
	}

	ui::Log();
	if (io::IsKeyPressed(VK_F1))
	{
		ui::LogAdd(L"233");
	}

	//static std::wstring str2 = L"";
	ui::EditBox(str);

	ui::SameLine();
	ui::Button(L"BUTTON6");
	ui::SameLine();
	ui::Button(L"BUTTON7");
	ui::SameLine();
	ui::Button(L"BUTTON8");

	ui::NewLine(10);




	//--------------------------------------------------------------

	ui::End();
}
