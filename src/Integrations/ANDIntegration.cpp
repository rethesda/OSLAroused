#include "ANDIntegration.h"
#include "../Settings.h"
#include "../Utilities/Utils.h"
#include "../Managers/ActorStateManager.h"

namespace Integrations
{
    bool ANDIntegration::Initialize()
    {
        Locker locker(m_Lock);

        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            logger::error("Failed to get TESDataHandler");
            return false;
        }

        // Look for A.N.D. mod in load order
        const char* andModName = "Advanced Nudity Detection.esp";
        const auto andMod = dataHandler->LookupModByName(andModName);
        if (!andMod) {
            logger::info("{} mod not found in load order, integration disabled", andModName);
            m_IsAvailable = false;
            return false;
        }

        m_ANDModIndex = andMod->GetPartialIndex();

        // Resolve all A.N.D. factions
        auto ResolveAndCastFaction = [&](uint32_t baseId, const char* name) -> RE::TESFaction* {
            RE::FormID formId = Utilities::Forms::ResolveFormId(m_ANDModIndex, baseId);
            if (!formId) {
                logger::warn("Failed to resolve A.N.D. faction: {} (0x{:08X})", name, baseId);
                return nullptr;
            }
            
            RE::TESFaction* faction = RE::TESForm::LookupByID<RE::TESFaction>(formId); 
            if(!faction) {
                logger::warn("Resolved A.N.D. faction is not a TESFaction: {} (0x{:08X})", name, formId);
                return nullptr;
            }
            return faction;
        };

        m_ANDNudeFaction = ResolveAndCastFaction(AND_NUDE_FACTION_ID, "Nude");
        m_ANDToplessFaction = ResolveAndCastFaction(AND_TOPLESS_FACTION_ID, "Topless");
        m_ANDBottomlessFaction = ResolveAndCastFaction(AND_BOTTOMLESS_FACTION_ID, "Bottomless");
        m_ANDShowingChestFaction = ResolveAndCastFaction(AND_SHOWING_CHEST_FACTION_ID, "ShowingChest");
        m_ANDShowingAssFaction = ResolveAndCastFaction(AND_SHOWING_ASS_FACTION_ID, "ShowingAss");
        m_ANDShowingGenitalsFaction = ResolveAndCastFaction(AND_SHOWING_GENITALS_FACTION_ID, "ShowingGenitals");
        m_ANDShowingBraFaction = ResolveAndCastFaction(AND_SHOWING_BRA_FACTION_ID, "ShowingBra");
        m_ANDShowingUnderwearFaction = ResolveAndCastFaction(AND_SHOWING_UNDERWEAR_FACTION_ID, "ShowingUnderwear");

        // Check if we resolved at least the core factions
        if (!m_ANDNudeFaction || !m_ANDToplessFaction || !m_ANDBottomlessFaction) {
            logger::warn("Failed to resolve core A.N.D. factions, integration disabled");
            m_IsAvailable = false;
            return false;
        }

        m_IsAvailable = true;
        logger::info("A.N.D. Integration initialized successfully");
        return true;
    }

    float ANDIntegration::GetANDNudityScore(RE::Actor* actor)
    {
        if (!actor) {
            return 0.0f;
        }

        Locker locker(m_Lock);

        if (!m_IsAvailable) {
            return 0.0f;
        }

        return FetchActorNudityState(actor).calculatedScore;
    }

    ANDIntegration::ANDNudityState ANDIntegration::FetchActorNudityState(RE::Actor* actor)
    {
        ANDNudityState state{};

        if (!actor || !m_IsAvailable) {
            return state;
        }

        // Check faction membership for each A.N.D. faction
        auto CheckFaction = [&](RE::TESFaction* faction) -> bool {
            return faction && actor->GetFactionRank(faction, actor->IsPlayer()) > 0;
        };

        state.isNude = CheckFaction(m_ANDNudeFaction);

        // Step 1: Hard override for Nude
        if (state.isNude) {
            state.calculatedScore = NUDE_BASELINE;  // 50.0f
            logger::debug("Actor {:08X} is nude, score: {}", actor->formID, state.calculatedScore);
            return state;
        }

        state.isTopless = CheckFaction(m_ANDToplessFaction);
        state.isBottomless = CheckFaction(m_ANDBottomlessFaction);
        state.isShowingChest = CheckFaction(m_ANDShowingChestFaction);
        state.isShowingAss = CheckFaction(m_ANDShowingAssFaction);
        state.isShowingGenitals = CheckFaction(m_ANDShowingGenitalsFaction);
        state.isShowingBra = CheckFaction(m_ANDShowingBraFaction);
        state.isShowingUnderwear = CheckFaction(m_ANDShowingUnderwearFaction);

        
        logger::debug("Actor {:08X} A.N.D. Nudity State - Nude: {}, Topless: {}, Bottomless: {}, ShowingChest: {}, ShowingAss: {}, ShowingGenitals: {}, ShowingBra: {}, ShowingUnderwear: {}",
                     actor->formID,
                     state.isNude,
                     state.isTopless,
                     state.isBottomless,
                     state.isShowingChest,
                     state.isShowingAss,
                     state.isShowingGenitals,
                     state.isShowingBra,
                     state.isShowingUnderwear);

        // Step 2: Chest region (priority: Topless > ShowingChest > ShowingBra)
        float chestScore = 0.0f;
        if (state.isTopless) {
            chestScore = TOPLESS_BASELINE;  // 20.0f
        } else if (state.isShowingChest) {
            chestScore = SHOWING_CHEST_BASELINE;  // 12.0f
        } else if (state.isShowingBra) {
            chestScore = SHOWING_BRA_BASELINE;  // 8.0f
        }

        // Step 3: Front bottom region (priority: Bottomless > ShowingGenitals > ShowingUnderwear)
        float frontScore = 0.0f;
        if (state.isBottomless) {
            frontScore = BOTTOMLESS_BASELINE;  // 30.0f
        } else if (state.isShowingGenitals) {
            frontScore = SHOWING_GENITALS_BASELINE;  // 15.0f
        } else if (state.isShowingUnderwear) {
            frontScore = SHOWING_UNDERWEAR_BASELINE;  // 8.0f
        }

        // Step 4: Back bottom / ass region
        float assScore = 0.0f;
        if (state.isBottomless) {
            assScore = 0.0f;  // Already covered by bottomless
        } else if (state.isShowingAss) {
            assScore = SHOWING_ASS_BASELINE;  // 8.0f
        }

        // Step 5: Calculate base score
        float baseScore = chestScore + frontScore + assScore;

        // Step 6: Special synergy case
        // If Topless and Bottomless are both true but Nude is false
        if (state.isTopless && state.isBottomless && !state.isNude) {
            baseScore = TOPLESS_BOTTOMLESS_SYNERGY;  // 37.0f
            logger::debug("Actor {:08X} has topless+bottomless synergy, score: {}",
                         actor->formID, baseScore);
        }

        // Step 7: Clamp to valid range [0, 50]
        state.calculatedScore = std::clamp(baseScore, 0.0f, 50.0f);

        logger::debug("Actor {:08X} nudity score: {} (chest:{}, front:{}, ass:{})",
                     actor->formID, state.calculatedScore, chestScore, frontScore, assScore);

        return state;
    }

    float ANDIntegration::GetNudityBaselineModifier(RE::Actor* actor)
    {
        if (!actor) {
            return 0.0f;
        }

        const auto settings = Settings::GetSingleton();

        // Check if A.N.D. integration is enabled in settings
        bool useANDIntegration = settings->GetUseANDIntegration();

        // Get A.N.D. score if integration is available and enabled
        float andScore = 0.0f;
        if (useANDIntegration && m_IsAvailable) {
            andScore = GetANDNudityScore(actor);
        }

        // If we have an A.N.D. score, apply the multiplier and return
        if (andScore > 0.0f) {
            float multiplier = settings->GetANDNudityMultiplier();
            float modifier = andScore * multiplier;

            logger::debug("Actor {:08X} using A.N.D. nudity modifier: {} (score:{} * mult:{})",
                         actor->formID, modifier, andScore, multiplier);
            return modifier;
        }

        // Fall back to legacy nudity detection
        if (IsActorNudeLegacy(actor)) {
            float nudeBaseline = settings->GetNudeArousalBaseline();
            logger::debug("Actor {:08X} using legacy nude baseline: {}", actor->formID, nudeBaseline);
            return nudeBaseline;
        }

        // Check for erotic armor as secondary fallback
        if (IsActorWearingEroticArmorLegacy(actor)) {
            // Get erotic armor baseline - Settings already has this functionality
            float eroticBaseline = settings->GetEroticArmorBaseline();
            if (eroticBaseline > 0.0f) {
                logger::debug("Actor {:08X} using erotic armor baseline: {}", actor->formID, eroticBaseline);
                return eroticBaseline;
            }
        }

        return 0.0f;
    }

    std::vector<float> ANDIntegration::GetANDFactionContributions(RE::Actor* actor)
    {
        std::vector<float> contributions;

        if (!actor) {
            return contributions;
        }

        Locker locker(m_Lock);

        if (!m_IsAvailable) {
            return contributions;
        }

        // Get the nudity state
        ANDNudityState nudityState = FetchActorNudityState(actor);

        // Initialize all contributions to 0
        contributions.reserve(8);
        float nudeContrib = 0.0f;
        float toplessContrib = 0.0f;
        float bottomlessContrib = 0.0f;
        float chestContrib = 0.0f;
        float assContrib = 0.0f;
        float genitalsContrib = 0.0f;
        float braContrib = 0.0f;
        float underwearContrib = 0.0f;

        // Calculate actual contributions based on priority rules
        // Step 1: Hard override for Nude
        if (nudityState.isNude) {
            nudeContrib = NUDE_BASELINE;  // 50.0f
            // When nude, all other contributions are 0
        } else {
            // Step 2: Chest region (priority: Topless > ShowingChest > ShowingBra)
            if (nudityState.isTopless) {
                toplessContrib = TOPLESS_BASELINE;  // 20.0f
                // ShowingChest and ShowingBra are overridden by Topless
            } else if (nudityState.isShowingChest) {
                chestContrib = SHOWING_CHEST_BASELINE;  // 12.0f
                // ShowingBra is overridden by ShowingChest
            } else if (nudityState.isShowingBra) {
                braContrib = SHOWING_BRA_BASELINE;  // 8.0f
            }

            // Step 3: Front bottom region (priority: Bottomless > ShowingGenitals > ShowingUnderwear)
            if (nudityState.isBottomless) {
                bottomlessContrib = BOTTOMLESS_BASELINE;  // 30.0f
                // ShowingGenitals and ShowingUnderwear are overridden by Bottomless
            } else if (nudityState.isShowingGenitals) {
                genitalsContrib = SHOWING_GENITALS_BASELINE;  // 15.0f
                // ShowingUnderwear is overridden by ShowingGenitals
            } else if (nudityState.isShowingUnderwear) {
                underwearContrib = SHOWING_UNDERWEAR_BASELINE;  // 8.0f
            }

            // Step 4: Back bottom / ass region
            if (!nudityState.isBottomless && nudityState.isShowingAss) {
                // ShowingAss only contributes if not Bottomless
                assContrib = SHOWING_ASS_BASELINE;  // 8.0f
            }

            // Step 5: Special synergy case
            // If Topless and Bottomless are both true but Nude is false, override individual contributions
            if (nudityState.isTopless && nudityState.isBottomless) {
                // Clear individual contributions and set the synergy value
                // We'll distribute this across topless and bottomless proportionally
                toplessContrib = 15.0f;  // Part of the 37 synergy
                bottomlessContrib = 22.0f;  // Part of the 37 synergy
                // Clear other contributions that would be redundant
                chestContrib = 0.0f;
                assContrib = 0.0f;
                genitalsContrib = 0.0f;
                braContrib = 0.0f;
                underwearContrib = 0.0f;
            }
        }

        // Build the result vector in the documented order
        contributions.push_back(nudeContrib);        // [0] Nude
        contributions.push_back(toplessContrib);     // [1] Topless
        contributions.push_back(bottomlessContrib);  // [2] Bottomless
        contributions.push_back(chestContrib);       // [3] ShowingChest
        contributions.push_back(assContrib);         // [4] ShowingAss
        contributions.push_back(genitalsContrib);    // [5] ShowingGenitals
        contributions.push_back(braContrib);         // [6] ShowingBra
        contributions.push_back(underwearContrib);   // [7] ShowingUnderwear

        logger::debug("Actor {:08X} A.N.D. faction contributions: Nude:{:.1f}, Topless:{:.1f}, Bottomless:{:.1f}, Chest:{:.1f}, Ass:{:.1f}, Genitals:{:.1f}, Bra:{:.1f}, Underwear:{:.1f}",
                     actor->formID,
                     contributions[0], contributions[1], contributions[2], contributions[3],
                     contributions[4], contributions[5], contributions[6], contributions[7]);

        return contributions;
    }

    bool ANDIntegration::IsActorNudeLegacy(RE::Actor* actor) const
    {
        if (!actor) {
            return false;
        }

        // Use existing utility function for legacy nude check
        return Utilities::Actor::IsNakedCached(actor);
    }

    bool ANDIntegration::IsActorWearingEroticArmorLegacy(RE::Actor* actor) const
    {
        if (!actor) {
            return false;
        }

        // Check if actor is wearing any armor with erotic keywords
        const auto settings = Settings::GetSingleton();

        const auto eroticKeyword = settings->GetEroticArmorKeyword();
        if(!eroticKeyword) {
            return false;  // No erotic armor keyword configured
        }
        const auto wornArmorKeywords = Utilities::Actor::GetWornArmorKeywords(actor);
        if(wornArmorKeywords.contains(eroticKeyword->formID)) {
            return true;  // Direct match found
        }

        return false;
    }
}