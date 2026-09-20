#include "ModConfigUI.h"

#include "Settings.h"
#include "Version.h"

#include <ModConfigUI/Localization.h>

namespace SAR
{
	const char* Translate(const char* a_key)
	{
		return ModConfigUI::Localization::Get(a_key);
	}

	// Pages
	void DrawSettingsPage(ModConfigUI::Renderer& a_renderer)
	{
		Settings* settings = Settings::GetSingleton();

		a_renderer.SeparatorText(Translate("$CSAR_Section_General"));

		if (a_renderer.Checkbox(Translate("$CSAR_ModActive"), &settings->modActive, MOD_ACTIVE_DEFAULT_VALUE, Translate("$CSAR_ModActive_Tooltip")))
		{
			SaveSettings();
		}

		// Both only ever apply to a restart this mod performs.
		a_renderer.BeginDisabled(!settings->modActive);
		if (a_renderer.Checkbox(Translate("$CSAR_SkipIntroMovie"), &settings->skipIntroMovie, SKIP_INTRO_MOVIE_DEFAULT_VALUE, Translate("$CSAR_SkipIntroMovie_Tooltip")))
		{
			SaveSettings();
		}

		if (a_renderer.Checkbox(Translate("$CSAR_SilentReload"), &settings->silentReload, SILENT_RELOAD_DEFAULT_VALUE, Translate("$CSAR_SilentReload_Tooltip")))
		{
			SaveSettings();
		}
		a_renderer.EndDisabled();
	}

	void InstallModConfigUI()
	{
		static constexpr ModConfigUI::ModInfo MOD_INFO{
			.pluginName = Version::NAME.data(),
			.displayName = Version::FORMATTED_NAME.data(),
			.version = Version::STRING.data(),
			.author = Version::AUTHOR.data(),
			.description = "$CSAR_Description",
			.nexusUrl = "https://www.nexusmods.com/skyrimspecialedition/mods/88219",
			.sourceUrl = "https://github.com/Vermunds/Skyrim-CleanSaveAutoReloader"
		};

		static constexpr ModConfigUI::Page PAGES[] = {
			{ "$CSAR_Page_Settings", &DrawSettingsPage }
		};

		ModConfigUI::Install(MOD_INFO, PAGES, &RestoreDefaults);
	}
}
