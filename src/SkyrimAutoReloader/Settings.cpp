#include "Settings.h"

#include <SimpleIni.h>

namespace
{
	constexpr const char* INI_PATH = R"(.\Data\SKSE\Plugins\SkyrimAutoReloader.ini)";

	void IniSection(CSimpleIniA& a_ini, const char* a_section, const char* a_comment = nullptr)
	{
		a_ini.SetValue(a_section, nullptr, nullptr, a_comment);
		logger::info("[{}]", a_section);
	}

	bool IniGetBool(CSimpleIniA& a_ini, const char* a_section, const char* a_key, bool a_default, const char* a_comment = nullptr)
	{
		bool val = a_ini.GetBoolValue(a_section, a_key, a_default);
		a_ini.SetBoolValue(a_section, a_key, val, a_comment, true);
		logger::info("  {}: {}", a_key, val);
		return val;
	}
}

namespace SAR
{
	Settings* Settings::GetSingleton()
	{
		static Settings singleton;
		return &singleton;
	}

	void LoadSettings()
	{
		Settings* settings = Settings::GetSingleton();

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(INI_PATH);

		logger::info("Loading settings from: {}", std::filesystem::absolute(INI_PATH).string());

		IniSection(ini, "GENERAL");
		settings->modActive = IniGetBool(ini, "GENERAL", "bModActive", MOD_ACTIVE_DEFAULT_VALUE, "# Turns the mod on and off. While it is off, loading a save and returning to the main menu work the way they do without the mod.");
		settings->skipIntroMovie = IniGetBool(ini, "GENERAL", "bSkipIntroMovie", SKIP_INTRO_MOVIE_DEFAULT_VALUE, "# Skips the intro movie when the game is relaunched by this mod. The first launch of the session is not affected.");
		settings->silentReload = IniGetBool(ini, "GENERAL", "bSilentReload", SILENT_RELOAD_DEFAULT_VALUE, "# Hides the \"Skyrim is reloading\" window shown while the game restarts.");

		logger::info("Settings loaded.");

		ini.SaveFile(INI_PATH);
	}

	void SaveSettings()
	{
		Settings* settings = Settings::GetSingleton();

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(INI_PATH);

		ini.SetBoolValue("GENERAL", "bModActive", settings->modActive, nullptr, true);
		ini.SetBoolValue("GENERAL", "bSkipIntroMovie", settings->skipIntroMovie, nullptr, true);
		ini.SetBoolValue("GENERAL", "bSilentReload", settings->silentReload, nullptr, true);

		ini.SaveFile(INI_PATH);

		logger::info("Settings saved.");
	}

	void RestoreDefaults()
	{
		Settings* settings = Settings::GetSingleton();

		settings->modActive = MOD_ACTIVE_DEFAULT_VALUE;
		settings->skipIntroMovie = SKIP_INTRO_MOVIE_DEFAULT_VALUE;
		settings->silentReload = SILENT_RELOAD_DEFAULT_VALUE;

		SaveSettings();
	}
}
