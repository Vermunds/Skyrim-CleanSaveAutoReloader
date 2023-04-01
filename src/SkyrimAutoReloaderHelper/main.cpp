#include <Windows.h>
#include <Shlobj.h>
#include <filesystem>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HANDLE newProcessHandle = 0;

void RestartGame(std::string & a_commandLine, std::string a_fileName)
{
	// Create startup information and process information structs
	STARTUPINFO startupInfo{};
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	ZeroMemory(&processInfo, sizeof(processInfo));
	startupInfo.cb = sizeof(startupInfo);

	const TCHAR* cmdline = a_commandLine.c_str();
	const TCHAR* envVarValue = a_fileName.empty() ? "$$$_MAIN_MENU_$$$" : a_fileName.c_str();

	// Environment values are inherited by child processes
	if (!SetEnvironmentVariableA("SKYRIM_AUTOLOAD_FILE_NAME", envVarValue))
	{
		spdlog::error("SetEnvironmentVariable failed ({})", GetLastError());
		return;
	}

	spdlog::info("Launching: {}", cmdline);
	if (!CreateProcessA(nullptr, const_cast<TCHAR*>(cmdline), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo))
	{
		spdlog::error("Failed to create process.({})", GetLastError());
		return;
	}

	CloseHandle(processInfo.hThread);
	newProcessHandle = processInfo.hProcess;
}

void WaitForGame(int a_pid)
{
	// Open parent Skyrim process
	HANDLE oldProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, a_pid);

	if (oldProcessHandle == nullptr)
	{
		spdlog::warn("Failed to open process. Assuming game process already terminated.");
	}
	else
	{
		// Wait until process terminates.
		DWORD waitResult = WaitForSingleObject(oldProcessHandle, INFINITE);

		if (waitResult != WAIT_OBJECT_0)
		{
			spdlog::error("WaitForSingleObject failed. Attempting to continue regardless.");
		}
		CloseHandle(oldProcessHandle);
	}
}

std::optional<std::filesystem::path> log_directory()
{
	wchar_t* buffer{ nullptr };
	const auto result = SHGetKnownFolderPath(::FOLDERID_Documents, KNOWN_FOLDER_FLAG::KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
	std::unique_ptr<wchar_t[], decltype(&CoTaskMemFree)> knownPath(buffer, CoTaskMemFree);
	if (!knownPath || result != S_OK)
	{
		return std::nullopt;
	}

	std::filesystem::path path = knownPath.get();
	path /= "My Games/Skyrim Special Edition/SKSE"sv;
	return path;
}

// Process command line function with support for quoted values
std::vector<std::string> ProcessCommandLine(std::string& a_input)
{
	std::vector<std::string> result;
	bool inQuotes = false;
	std::string currentString;
	for (char c : a_input)
	{
		if (c == '\"')
		{
			inQuotes = !inQuotes;
		}
		else if (c == ' ' && !inQuotes)
		{
			if (!currentString.empty())
			{
				result.push_back(currentString);
				currentString.clear();
			}
		}
		else
		{
			currentString += c;
		}
	}
	if (!currentString.empty())
	{
		result.push_back(currentString);
	}
	return result;
}

// WndProc of the "Skyrim is reloading" status window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hLabel, hProgress;
	static const char* targetClassName = "Skyrim Special Edition";
	static const char* targetWindowName = "Skyrim Special Edition";
	static const int checkInterval = 100;  // Check for target window every 100 millisecond
	static const int timeout = 30;         // Timeout after 30 seconds
	static UINT_PTR timerId = 0;
	static int elapsedTime = 0;
	static HANDLE processHandle = 0;

	switch (message)
	{
	case WM_CREATE:
		{
			// Create the label and progress bar controls
			const char* labelText = "Skyrim is reloading. Please wait...";
			hLabel = CreateWindowExA(0, "STATIC", labelText, WS_VISIBLE | WS_CHILD, 10, 10, 200, 20, hWnd, nullptr, nullptr, nullptr);
			hProgress = CreateWindowExA(0, PROGRESS_CLASS, nullptr, WS_VISIBLE | WS_CHILD | PBS_MARQUEE, 10, 50, 360, 20, hWnd, nullptr, nullptr, nullptr);
			SendMessage(hProgress, PBM_SETMARQUEE, TRUE, 0);

			// Calculate the width of the label text
			HDC hdc = GetDC(hLabel);
			SIZE size;
			GetTextExtentPoint32(hdc, labelText, strlen(labelText), &size);
			ReleaseDC(hLabel, hdc);

			// Calculate the center position of the label
			RECT rect;
			GetClientRect(hWnd, &rect);
			int centerX = (rect.right - rect.left) / 2;
			int labelWidth = size.cx;
			int labelX = centerX - labelWidth / 2;

			// Position the label in the center
			SetWindowPos(hLabel, nullptr, labelX, 10, labelWidth, 20, SWP_NOZORDER);

			// Start the timer
			timerId = SetTimer(hWnd, 1, checkInterval, nullptr);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		{
			// Check for the target window
			HWND hTargetWnd = FindWindow(targetClassName, targetWindowName);
			if (hTargetWnd)
			{
				// Target window found, close the application
				PostQuitMessage(0);
			}
			else
			{
				// Increment the elapsed time and check for timeout
				elapsedTime += checkInterval;
				if (elapsedTime >= timeout * 1000)
				{
					// Timeout reached, close the application
					PostQuitMessage(0);
				}
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Logger setup
	assert(log_directory().has_value());
	auto path = log_directory().value() / std::filesystem::path("SkyrimAutoReload_Helper.log"s);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::trace);
	log->flush_on(spdlog::level::trace);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s, spdlog::pattern_time_type::local);

    // Parse command line arguments
	std::string cmdLine(lpCmdLine);
	std::vector<std::string> args = ProcessCommandLine(cmdLine);

	// Check if all arguments are present
	if (args.size() <= 0)
	{
		spdlog::error("Do not run this application manually.");
		MessageBoxA(nullptr, "Do not run this application manually.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Parse arguments
	int pid = 0;
	std::string commandLine;
	std::string fileName;

	for (size_t i = 0; i < args.size(); i += 2)
	{
		if (i + 1 >= args.size())
		{
			spdlog::error("Missing argument value for {}.", args[i]);
			return 1;
		}

		if (args[i] == "--pid")
		{
			try
			{
				pid = std::stoi(args[i + 1]);
			}
			catch (std::invalid_argument&)
			{
				spdlog::error("Invalid argument for pid.");
				return 1;
			}
		}
		else if (args[i] == "--commandline")
		{
			commandLine = args[i + 1];
		}
		else if (args[i] == "--filename")
		{
			fileName = args[i + 1];
		}
		else
		{
			spdlog::error("Invalid argument {}.", args[i]);
			return 1;
		}
	}

	// Check if all arguments are valid
	if (pid <= 0)
	{
		spdlog::error("Missing or invalid pid argument.");
		return 1;
	}
	if (commandLine.empty())
	{
		spdlog::error("Missing commandline argument.");
		return 1;
	}
	if (fileName.empty())
	{
		spdlog::info("Missing filename argument. Starting game normally.");
	}

	spdlog::info("pid = {}, commandLine = {}", pid, commandLine);

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icc);

	// Register the window class
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = "SkyrimAutoReloader";
	RegisterClassA(&wc);

	// Get the screen dimensions
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Calculate the center coordinates of the window
	int windowWidth = 400;
	int windowHeight = 120;
	int windowX = (screenWidth - windowWidth) / 2;
	int windowY = (screenHeight - windowHeight) / 2;

	// Create the window
	HWND wnd = CreateWindowExA(0, "SkyrimAutoReloader", "Skyrim Special Edition", WS_POPUPWINDOW | WS_CAPTION | WS_SYSMENU, windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
	if (!wnd)
	{
		return 1;
	}

	WaitForGame(pid);
	RestartGame(commandLine, fileName);
	if (!newProcessHandle)
	{
		spdlog::error("Restarting game failed!");
		return 1;
	}

	// Show the window and enter the message loop
	ShowWindow(wnd, nCmdShow);
	UpdateWindow(wnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseHandle(newProcessHandle);

	return (int)msg.wParam;
}
