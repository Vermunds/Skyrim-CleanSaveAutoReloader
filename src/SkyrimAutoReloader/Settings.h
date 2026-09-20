#pragma once

namespace SAR
{
	// General
	constexpr bool MOD_ACTIVE_DEFAULT_VALUE = true;
	constexpr bool SKIP_INTRO_MOVIE_DEFAULT_VALUE = true;
	constexpr bool SILENT_RELOAD_DEFAULT_VALUE = false;

	class Settings
	{
	public:
		static Settings* GetSingleton();

		// General
		bool modActive;
		bool skipIntroMovie;
		bool silentReload;

	private:
		Settings() {};
		~Settings() {};
		Settings(const Settings&) = delete;
		Settings& operator=(const Settings&) = delete;
	};

	void LoadSettings();
	void SaveSettings();
	void RestoreDefaults();
}
