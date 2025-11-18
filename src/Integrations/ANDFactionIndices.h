#pragma once

namespace Integrations
{
    /**
     * @brief Faction indices for A.N.D. (Advanced Nudity Detection) integration
     *
     * These constants define the array indices used when working with
     * A.N.D. faction data in functions like GetANDFactionContributions()
     * and SetANDFactionBaseline().
     */
    namespace ANDFactionIndex
    {
        constexpr int NUDE = 0;           // Full nudity state
        constexpr int TOPLESS = 1;        // Upper body exposed
        constexpr int BOTTOMLESS = 2;     // Lower body exposed
        constexpr int SHOWING_CHEST = 3;  // Partially exposed chest
        constexpr int SHOWING_ASS = 4;    // Partially exposed rear
        constexpr int SHOWING_GENITALS = 5; // Partially exposed front
        constexpr int SHOWING_BRA = 6;    // Wearing only bra
        constexpr int SHOWING_UNDERWEAR = 7; // Wearing only underwear

        constexpr int COUNT = 8;          // Total number of factions

        /**
         * @brief Validates a faction index
         * @param index The index to validate
         * @return true if index is valid (0-7), false otherwise
         */
        inline bool IsValidIndex(int index)
        {
            return index >= 0 && index < COUNT;
        }

        /**
         * @brief Gets a descriptive name for a faction index
         * @param index The faction index
         * @return String name of the faction, or "Unknown" for invalid indices
         */
        inline const char* GetFactionName(int index)
        {
            switch (index)
            {
            case NUDE: return "Nude";
            case TOPLESS: return "Topless";
            case BOTTOMLESS: return "Bottomless";
            case SHOWING_CHEST: return "ShowingChest";
            case SHOWING_ASS: return "ShowingAss";
            case SHOWING_GENITALS: return "ShowingGenitals";
            case SHOWING_BRA: return "ShowingBra";
            case SHOWING_UNDERWEAR: return "ShowingUnderwear";
            default: return "Unknown";
            }
        }
    }
}