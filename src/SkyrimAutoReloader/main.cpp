#include <string>
#include <Windows.h>

const std::map<DWORD, std::string_view> priorityLevels = {
	{ ABOVE_NORMAL_PRIORITY_CLASS, "above_normal" },
	{ BELOW_NORMAL_PRIORITY_CLASS, "below_normal" },
	{ HIGH_PRIORITY_CLASS, "high" },
	{ IDLE_PRIORITY_CLASS, "idle" },
	{ NORMAL_PRIORITY_CLASS, "normal" },
	{ REALTIME_PRIORITY_CLASS, "realtime" }
};

namespace SAR
{
	bool autoLoadMode = false;
	bool skipIntro = false;
	std::string autoLoadFileName{};
	uint32_t loadingCounter = 0;

	using LoadGame_t = decltype(&RE::BSWin32SaveDataSystemUtility::Unk_11);
	REL::Relocation<LoadGame_t> _LoadGame;

	const std::string getSKSECommandLine()
	{
		std::stringstream commandLine;
		commandLine << "skse64_loader.exe";

		// Figure out SKSE arguments from the log file of the loader
		// This should handle the most important stuff
		auto logPath = SKSE::log::log_directory().value() / std::filesystem::path("skse64_loader.log"s);
		std::ifstream logFile(logPath);
		if (logFile)
		{
			std::string line;
			while (std::getline(logFile, line))
			{
				if (line.starts_with("forcing steam loader"))
				{
					commandLine << " -forcesteamloader";
					break;
				}
				else if (line.starts_with("launching alternate exe"))
				{
					size_t openPos = line.find("(");
					size_t closePos = line.find(")");
					if (openPos != std::string::npos && closePos != std::string::npos && closePos > openPos)
					{
						std::string value = line.substr(openPos + 1, closePos - openPos - 1);
						commandLine << " -altexe " << value;
					}
				}
				else if (line.starts_with("launching alternate dll"))
				{
					size_t openPos = line.find("(");
					size_t closePos = line.find(")");
					if (openPos != std::string::npos && closePos != std::string::npos && closePos > openPos)
					{
						std::string value = line.substr(openPos + 1, closePos - openPos - 1);
						commandLine << " -altdll " << value;
					}
				}
			}
		}

		// Some priority classes are not inherited, so set it manually
		// Affinity is inherited
		HANDLE currentProcess = GetCurrentProcess();
		commandLine << " -priority " << priorityLevels.at(GetPriorityClass(currentProcess));

		return commandLine.str();
	}

	void RestartGame(const std::string& a_fileName)
	{	
		spdlog::info("Restart requested. Filename: {}", a_fileName);

		// Build command line arguments
		std::stringstream commandLine;
		commandLine << "Data\\SKSE\\Plugins\\SkyrimAutoReloaderHelper.exe"
					<< " --pid " << GetCurrentProcessId()
					<< " --commandline \"" << getSKSECommandLine() << "\"";

		if (!a_fileName.empty())
		{
			commandLine << " --filename \"" << a_fileName << "\"";
		}

		// Create startup information and process information structs
		STARTUPINFO startupInfo;
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&startupInfo, sizeof(startupInfo));
		ZeroMemory(&processInfo, sizeof(processInfo));
		startupInfo.cb = sizeof(startupInfo);

		// Create restarter process
		if (CreateProcessA(nullptr, const_cast<char*>(commandLine.str().c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInfo))
		{
			// Close process handles
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);

			// Set quitGame flag
			RE::Main::GetSingleton()->quitGame = true;

			spdlog::info("New process created: {}", commandLine.str());
		}
		else
		{
			spdlog::error("Failed to create process. Command line arguments: {}", commandLine.str());
		}
	}

	void LoadGame_Hook(RE::BSWin32SaveDataSystemUtility* a_this, const char* a_fileName, std::uint64_t a_unk1, void* a_unk2)
	{
		if (loadingCounter > 1) // This is incremented to 1 before main menu
		{
			// a_fileName contains the full path to the file
			std::filesystem::path path{ a_fileName };
			return RestartGame(path.stem().string());
		}
		return _LoadGame(a_this, a_fileName, a_unk1, a_unk2);
	}

	void FadeThenMainMenuCallback_Hook(void* a_this, char a_unk1)
	{
		return RestartGame(""s);
	}

	void InstallHook()
	{
		// Hook loading function
		// This is shared for all types of loading (journal, console, auto-load) as well as calls by other mods into the Load function.
		REL::Relocation<std::uintptr_t> vTable(REL::ID{ 306359 });
		_LoadGame = vTable.write_vfunc(0x11, &LoadGame_Hook);

		// Hook returning to main menu while in-game
		SKSE::GetTrampoline().write_branch<5>(REL::ID{ 17554 }.address(), FadeThenMainMenuCallback_Hook);

		if (autoLoadMode || skipIntro)
		{
			// Disable startup movie when auto-loading to speed things up a bit
			REL::safe_fill(REL::ID{ 35549 }.address() + 0xB4, 0x90, 5);
		}
	}

	class UIEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:

		// Menu event handler to count loading screens, in case the game was not started by loading a save (eg. new game was started or the user coc'd from the main menu).
		// Also hides the main menu in case we are auto-loading a save.
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_dispatcher) override
		{
			if (!a_event || !a_event->opening)
			{
				return RE::BSEventNotifyControl::kContinue;
			}

			RE::UI* ui = RE::UI::GetSingleton();

			// Prevent the main menu from showing up
			if (a_event->menuName == RE::MainMenu::MENU_NAME && autoLoadMode)
			{
				if (RE::MainMenu* mainMenu = static_cast<RE::MainMenu*>(ui->GetMenu(RE::MainMenu::MENU_NAME).get()))
				{
					mainMenu->uiMovie->SetVisible(false);
				}
			}

			if (a_event->menuName == RE::LoadingMenu::MENU_NAME)
			{
				++loadingCounter;
				if (loadingCounter > 1) // This is incremented to 1 before main menu
				{
					spdlog::info("Game loaded. Removing MenuOpenCloseEvent sink.");
					ui->RemoveEventSink<RE::MenuOpenCloseEvent>(this);
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		static UIEventHandler* GetSingleton()
		{
			static UIEventHandler instance{};
			return &instance;
		}

	private:
		UIEventHandler(){};
		~UIEventHandler(){};
		UIEventHandler(const UIEventHandler&) = delete;
		UIEventHandler& operator=(const UIEventHandler&) = delete;
	};
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type)
	{
	case SKSE::MessagingInterface::kInputLoaded:
	{
		RE::UI* ui = RE::UI::GetSingleton();
		assert(ui);

		ui->AddEventSink(SAR::UIEventHandler::GetSingleton());
	}
	break;
	case SKSE::MessagingInterface::kDataLoaded:
	{
		if (SAR::autoLoadMode)
		{
			RE::BGSSaveLoadManager* manager = RE::BGSSaveLoadManager::GetSingleton();
			assert(manager);
			if (!manager->Load(SAR::autoLoadFileName.c_str(), false))
			{
				spdlog::error("Loading save failed. Setting main menu to visible.");
				RE::UI* ui = RE::UI::GetSingleton();
				if (RE::MainMenu* mainMenu = static_cast<RE::MainMenu*>(ui->GetMenu(RE::MainMenu::MENU_NAME).get()))
				{
					mainMenu->uiMovie->SetVisible(true);
				}
				RE::DebugNotification("Error loading save.");
			}
		}
	}
	break;
	}
}

extern "C" {
	//DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	//	SKSE::PluginVersionData v;

	//	v.PluginVersion(Plugin::VERSION);
	//	v.PluginName(Plugin::NAME);

	//	v.UsesAddressLibrary(true);
	//	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	//	return v;
	//}();

	DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = Plugin::NAME.data();
		a_info->version = Plugin::VERSION[0];

		if (a_skse->RuntimeVersion() < SKSE::RUNTIME_1_5_39)
		{
			SKSE::log::critical("Unsupported runtime version {}", a_skse->RuntimeVersion().string());
			return false;
		}
		return true;
	}

	DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		// Create logger
		assert(SKSE::log::log_directory().has_value());
		auto path = SKSE::log::log_directory().value() / std::filesystem::path(Plugin::NAME.data() + ".log"s);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

		log->set_level(spdlog::level::trace);
		log->flush_on(spdlog::level::trace);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%g(%#): [%^%l%$] %v", spdlog::pattern_time_type::local);

		// Init mod
		SKSE::log::info("{} v{} - {}", Plugin::NAME.data(), Plugin::VERSION_STRING.data(), __TIMESTAMP__);

		if (a_skse->IsEditor())
		{
			SKSE::log::critical("Loaded in editor, marking as incompatible!");
			return false;
		}

		SKSE::AllocTrampoline((size_t)2 << 4);
		SKSE::Init(a_skse);

		// Register SKSE Messaging interface
		auto messaging = SKSE::GetMessagingInterface();
		if (messaging->RegisterListener("SKSE", MessageHandler))
		{
			SKSE::log::info("Messaging interface registration successful.");
		}
		else
		{
			SKSE::log::critical("Messaging interface registration failed.");
			return false;
		}

		// Check if we are auto-loading and set up the variables
		char fileName[1024];
		size_t len = GetEnvironmentVariableA("SKYRIM_AUTOLOAD_FILE_NAME", fileName, 0);
		GetEnvironmentVariableA("SKYRIM_AUTOLOAD_FILE_NAME", fileName, len);
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
		{
			SKSE::log::info("Environment variable SKYRIM_AUTOLOAD_FILE_NAME is not set. Proceeding normally.");
		}
		else if (len)
		{
			std::string fileNameStr{ fileName, len - 1 };
			if (fileNameStr == "$$$_MAIN_MENU_$$$")
			{
				SKSE::log::info("Environment variable SKYRIM_AUTOLOAD_FILE_NAME is set to $$$_MAIN_MENU_$$$. Skipping intro and proceeding normally.");
				SAR::skipIntro = true;

			}
			else
			{
				SKSE::log::info("Environment variable SKYRIM_AUTOLOAD_FILE_NAME is set. Auto-loading file: {}", fileNameStr);
				SAR::autoLoadMode = true;
				SAR::autoLoadFileName = fileNameStr;
			}
		}

		// Install hooks
		SAR::InstallHook();

		return true;
	}
}
