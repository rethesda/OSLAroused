#include "PCH.h"
#include "Config.h"
#include "Libraries/SimpleIni/SimpleIni.h"
#include "Settings.h"

#include <RE/T/TESForm.h>

void Config::LoadINIs()
{
    if (LoadINI("Data/SKSE/Plugins/OSLAroused.ini", true))
    {
        m_ConfigLoaded = true;
    }
    else
    {
        SKSE::log::error("Failed to load INI file.");
        m_ConfigLoaded = false;
        return;
    }

    // Load Custom INI - only override values that are explicitly present
    LoadINI("Data/SKSE/Plugins/OSLAroused_Custom.ini", false);
}

bool Config::LoadINI(std::string fileName, bool useDefaults)
{
    CSimpleIniA ini(false, true, false);
    SI_Error rc = ini.LoadFile(fileName.c_str());
    if (rc < 0)
    {
        return false;
    }

    // Helper lambda to load float values from INI
    auto loadFloat = [&](const char* section, const char* key, float defaultVal, float& target) {
        const char* str = ini.GetValue(section, key, nullptr);
        if (str != nullptr) {
            target = static_cast<float>(std::stod(str));
        } else if (useDefaults) {
            target = defaultVal;
        }
    };

    // Helper lambda to load bool values from INI
    auto loadBool = [&](const char* section, const char* key, bool defaultVal, auto setter) {
        const char* str = ini.GetValue(section, key, nullptr);
        if (str != nullptr) {
            setter(std::stoi(str) != 0);
        } else if (useDefaults) {
            setter(defaultVal);
        }
    };

    // Get a list of keywords in Keyword Section
    CSimpleIniA::TNamesDepend keywords;
    ini.GetAllValues("RegisteredKeywords", "KeywordEditorId", keywords);

    // Get A.N.D. Integration settings
    loadBool("ANDIntegration", "UseANDIntegration", true,
        [](bool val) { Settings::GetSingleton()->SetUseANDIntegration(val); });

    // Load A.N.D. faction baseline values
    Settings::ANDFactionBaselines baselines = Settings::GetSingleton()->GetANDFactionBaselines();
    loadFloat("ANDIntegration", "NudeBaseline", 50.0f, baselines.Nude);
    loadFloat("ANDIntegration", "ToplessBaseline", 20.0f, baselines.Topless);
    loadFloat("ANDIntegration", "BottomlessBaseline", 30.0f, baselines.Bottomless);
    loadFloat("ANDIntegration", "ShowingChestBaseline", 12.0f, baselines.ShowingChest);
    loadFloat("ANDIntegration", "ShowingAssBaseline", 8.0f, baselines.ShowingAss);
    loadFloat("ANDIntegration", "ShowingGenitalsBaseline", 15.0f, baselines.ShowingGenitals);
    loadFloat("ANDIntegration", "ShowingBraBaseline", 8.0f, baselines.ShowingBra);
    loadFloat("ANDIntegration", "ShowingUnderwearBaseline", 8.0f, baselines.ShowingUnderwear);
    Settings::GetSingleton()->SetANDFactionBaselines(baselines);

    // Get the log level from the System section
    const char *logLevelStr = ini.GetValue("System", "LogLevel", useDefaults ? "1" : nullptr);
    if (logLevelStr != nullptr) {
        m_LogLevel = std::stoi(logLevelStr);

        // Set the log level BEFORE logging the level name to ensure consistent output
        auto logLevel = static_cast<spdlog::level::level_enum>(m_LogLevel);
        spdlog::set_level(logLevel);  // Global level
        spdlog::default_logger()->set_level(logLevel);  // Default logger
        spdlog::default_logger()->flush_on(logLevel);  // Flush at or above this level

        // Log loglevel name
        switch (m_LogLevel)
        {
        case spdlog::level::trace:
            SKSE::log::info("Log Level: Trace");
            break;
        case spdlog::level::debug:
            SKSE::log::info("Log Level: Debug");
            break;
        case spdlog::level::info:
            SKSE::log::info("Log Level: Info");
            break;
        case spdlog::level::warn:
            SKSE::log::info("Log Level: Warn");
            break;
        default:
            SKSE::log::info("Log Level: Error");
            break;
        }
    }

    SKSE::log::info("Trying to Register {} Keywords", keywords.size());
    // Iterate and log each section name
    for (auto &keyword : keywords)
    {

        auto keywordForm = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(keyword.pItem);
        if (keywordForm != nullptr)
        {
            m_RegisteredKeywordEditorIds.emplace_back(keywordForm->formID, keyword.pItem);
        }
        else
        {
            SKSE::log::warn("Keyword: {} failed to register. Failed to find Keyword Form.", keyword.pItem);
        }
    }

    return true;
}

bool Config::RegisterKeyword(std::string keywordEditorId)
{
    auto keywordForm = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(keywordEditorId);
    if (!keywordForm)
    {
        SKSE::log::error("RegisterKeyword: Failed to find keyword form.");
        return false;
    }
    m_RegisteredKeywordEditorIds.emplace_back(keywordForm->formID, keywordEditorId);

    CSimpleIniA ini(false, true, false);
    SI_Error rc = ini.LoadFile("Data/SKSE/Plugins/OSLAroused_Custom.ini");
    if (rc < 0)
    {
        // This is fine, just means the file doesnt exist yet
    }

    rc = ini.SetValue("RegisteredKeywords", "KeywordEditorId", keywordEditorId.c_str());
    if (rc < 0)
    {
        SKSE::log::error("RegisterKeyword: Failed to set value in INI file. Error: {}", rc);
        return false;
    }

    rc = ini.SaveFile("Data/SKSE/Plugins/OSLAroused_Custom.ini");
    if (rc < 0)
    {
        SKSE::log::error("RegisterKeyword: Failed to save INI file. Error: {}", rc);
        return false;
    }

    return true;
}

bool Config::SaveANDFactionBaseline(int index, float value)
{
    // Map index to INI key name
    const char* keyName = nullptr;
    switch (index) {
    case 0: keyName = "NudeBaseline"; break;
    case 1: keyName = "ToplessBaseline"; break;
    case 2: keyName = "BottomlessBaseline"; break;
    case 3: keyName = "ShowingChestBaseline"; break;
    case 4: keyName = "ShowingAssBaseline"; break;
    case 5: keyName = "ShowingGenitalsBaseline"; break;
    case 6: keyName = "ShowingBraBaseline"; break;
    case 7: keyName = "ShowingUnderwearBaseline"; break;
    default:
        SKSE::log::error("SaveANDFactionBaseline: Invalid index {}", index);
        return false;
    }

    CSimpleIniA ini(false, true, false);
    // Try to load the custom INI file first to preserve other settings
    ini.LoadFile("Data/SKSE/Plugins/OSLAroused_Custom.ini");

    // Convert float to string with proper precision
    char valueStr[32];
    snprintf(valueStr, sizeof(valueStr), "%.1f", value);

    SI_Error rc = ini.SetValue("ANDIntegration", keyName, valueStr);
    if (rc < 0) {
        SKSE::log::error("SaveANDFactionBaseline: Failed to set value in INI. Error: {}", rc);
        return false;
    }

    rc = ini.SaveFile("Data/SKSE/Plugins/OSLAroused_Custom.ini");
    if (rc < 0) {
        SKSE::log::error("SaveANDFactionBaseline: Failed to save INI file. Error: {}", rc);
        return false;
    }

    SKSE::log::debug("SaveANDFactionBaseline: Saved {}={} to INI", keyName, value);
    return true;
}
