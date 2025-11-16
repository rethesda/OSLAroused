#include "PCH.h"
#include "Settings.h"
#include "Config.h"

void Settings::SetANDFactionBaseline(int index, float value)
{
    {
        Locker locker(m_Lock);
        value = std::clamp(value, 0.0f, 100.0f);
        switch (index) {
        case 0: m_ANDFactionBaselines.Nude = value; break;
        case 1: m_ANDFactionBaselines.Topless = value; break;
        case 2: m_ANDFactionBaselines.Bottomless = value; break;
        case 3: m_ANDFactionBaselines.ShowingChest = value; break;
        case 4: m_ANDFactionBaselines.ShowingAss = value; break;
        case 5: m_ANDFactionBaselines.ShowingGenitals = value; break;
        case 6: m_ANDFactionBaselines.ShowingBra = value; break;
        case 7: m_ANDFactionBaselines.ShowingUnderwear = value; break;
        default: return; // Invalid index
        }
    }

    // Save to INI file
    Config::GetSingleton()->SaveANDFactionBaseline(index, value);
}