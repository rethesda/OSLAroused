#pragma once

#include "../PCH.h"
#include "../Utilities/Ticker.h"

namespace Integrations
{
    /**
     * Integration with Advanced Nudity Detection (A.N.D.) mod
     *
     * This class provides integration with the A.N.D. mod, which uses factions
     * to track detailed nudity states. It calculates a nudity score (0-50) based
     * on which A.N.D. factions an actor belongs to, and converts that to an
     * arousal baseline modifier.
     *
     * The integration gracefully falls back to simple nudity detection if A.N.D.
     * is not present in the load order.
     */
    class ANDIntegration
    {
    public:
        struct ANDNudityState
        {
            bool isNude = false;
            bool isTopless = false;
            bool isBottomless = false;
            bool isShowingChest = false;
            bool isShowingAss = false;
            bool isShowingGenitals = false;
            bool isShowingBra = false;
            bool isShowingUnderwear = false;
            float calculatedScore = 0.0f;
        };

        [[nodiscard]] static ANDIntegration *GetSingleton()
        {
            static ANDIntegration singleton;
            return &singleton;
        }

        bool Initialize();

        /**
         * @brief Checks if A.N.D. integration is available and enabled
         */
        [[nodiscard]] bool IsAvailable() const { return m_IsAvailable; }

        /**
         * @brief Calculates nudity score (0-50) based on A.N.D. faction membership
         *
         * Score calculation follows these rules:
         * - Nude faction: Returns 50 immediately (hard override)
         * - Chest region (priority): Topless(20) > ShowingChest(12) > ShowingBra(8)
         * - Front bottom (priority): Bottomless(30) > ShowingGenitals(15) > ShowingUnderwear(8)
         * - Back bottom: ShowingAss(8) only if not Bottomless
         * - Special synergy: Topless + Bottomless (without Nude) = 37
         *
         * @param actor The actor to check
         * @return Nudity score between 0 and 50
         */
        [[nodiscard]] float GetANDNudityScore(RE::Actor *actor);

        /**
         * @brief Gets the arousal baseline modifier based on nudity state
         *
         * Uses A.N.D. integration if available and enabled, otherwise falls back
         * to legacy nudity/erotic armor detection.
         *
         * @param actor The actor to check
         * @return Arousal baseline modifier (typically 0-30)
         */
        [[nodiscard]] float GetNudityBaselineModifier(RE::Actor *actor);

        /**
         * @brief Gets the A.N.D. faction contribution values for an actor
         *
         * Returns an array of 8 floats representing the arousal contribution from each faction:
         * [0] = Nude contribution (0 or 50)
         * [1] = Topless contribution (0 or 20)
         * [2] = Bottomless contribution (0 or 30)
         * [3] = ShowingChest contribution (0 or 12)
         * [4] = ShowingAss contribution (0 or 8)
         * [5] = ShowingGenitals contribution (0 or 15)
         * [6] = ShowingBra contribution (0 or 8)
         * [7] = ShowingUnderwear contribution (0 or 8)
         *
         * Note: The actual contributions follow priority rules and may be 0 even if
         * the actor is in the faction (e.g., ShowingChest is 0 if Topless is active)
         *
         * @param actor The actor to check
         * @return Vector of 8 floats representing contribution values, or empty if not available
         */
        [[nodiscard]] std::vector<float> GetANDFactionContributions(RE::Actor *actor);

    private:
        ANDIntegration() {};
        ~ANDIntegration() = default;
        ANDIntegration(const ANDIntegration &) = delete;
        ANDIntegration &operator=(const ANDIntegration &) = delete;
        ANDIntegration(ANDIntegration &&) = delete;
        ANDIntegration &operator=(ANDIntegration &&) = delete;

        /**
         * @brief Fetches the current A.N.D. faction state for an actor
         * @param actor The actor to check
         * @return The nudity state based on faction membership
         */
        [[nodiscard]] ANDNudityState FetchActorNudityState(RE::Actor *actor);

        /**
         * @brief Checks if the actor is wearing erotic armor (legacy)
         * @param actor The actor to check
         * @return true if wearing armor with erotic keywords
         */
        [[nodiscard]] bool IsActorWearingEroticArmorLegacy(RE::Actor *actor) const;

        /**
         * @brief Checks if the actor is nude (legacy)
         * @param actor The actor to check
         * @return true if not wearing body armor
         */
        [[nodiscard]] bool IsActorNudeLegacy(RE::Actor *actor) const;

        // A.N.D. Faction FormIDs (resolved at initialization)
        RE::TESFaction *m_ANDNudeFaction = nullptr;
        RE::TESFaction *m_ANDToplessFaction = nullptr;
        RE::TESFaction *m_ANDBottomlessFaction = nullptr;
        RE::TESFaction *m_ANDShowingChestFaction = nullptr;
        RE::TESFaction *m_ANDShowingAssFaction = nullptr;
        RE::TESFaction *m_ANDShowingGenitalsFaction = nullptr;
        RE::TESFaction *m_ANDShowingBraFaction = nullptr;
        RE::TESFaction *m_ANDShowingUnderwearFaction = nullptr;

        // Base FormIDs for A.N.D. factions (before resolution)
        // These will be configured based on the actual A.N.D. mod structure
        static constexpr uint32_t AND_SHOWING_ASS_FACTION_ID = 0x82E;
        static constexpr uint32_t AND_SHOWING_CHEST_FACTION_ID = 0x82F;
        static constexpr uint32_t AND_SHOWING_GENITALS_FACTION_ID = 0x830;
        static constexpr uint32_t AND_NUDE_FACTION_ID = 0x831;
        static constexpr uint32_t AND_TOPLESS_FACTION_ID = 0x832;
        static constexpr uint32_t AND_BOTTOMLESS_FACTION_ID = 0x833;
        static constexpr uint32_t AND_SHOWING_BRA_FACTION_ID = 0x834;
        static constexpr uint32_t AND_SHOWING_UNDERWEAR_FACTION_ID = 0x835;

        // Baseline contribution values for each nudity state
        static constexpr float NUDE_BASELINE = 50.0f;
        static constexpr float TOPLESS_BASELINE = 20.0f;
        static constexpr float BOTTOMLESS_BASELINE = 30.0f;
        static constexpr float SHOWING_CHEST_BASELINE = 12.0f;
        static constexpr float SHOWING_GENITALS_BASELINE = 15.0f;
        static constexpr float SHOWING_ASS_BASELINE = 8.0f;
        static constexpr float SHOWING_BRA_BASELINE = 8.0f;
        static constexpr float SHOWING_UNDERWEAR_BASELINE = 8.0f;

        // Special synergy score for Topless + Bottomless without Nude
        static constexpr float TOPLESS_BOTTOMLESS_SYNERGY = 37.0f;

        // Integration availability flag
        bool m_IsAvailable = false;

        // Mod index for A.N.D. (resolved at initialization)
        uint16_t m_ANDModIndex = 0xFFFF;

        // Thread safety
        mutable std::recursive_mutex m_Lock;
        using Locker = std::lock_guard<std::recursive_mutex>;
    };
}