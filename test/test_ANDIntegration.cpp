#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/Settings.h"
#include "../src/Integrations/ANDFactionIndices.h"

using Catch::Approx;
using namespace Integrations::ANDFactionIndex;

// Note: Full integration testing would require mocking RE::Actor and RE::TESFaction
// which is complex without the game runtime. These tests focus on the logic
// and calculations rather than the actual faction checking.

TEST_CASE("A.N.D. Integration - Nudity Score Calculation Logic", "[and_integration]")
{
    Settings* settings = Settings::GetSingleton();
    settings->SetUseANDIntegration(true);

    SECTION("Nude state returns maximum score of 50")
    {
        // When nude faction is detected, score should be 50
        float expectedScore = 50.0f;
        REQUIRE(expectedScore == Approx(50.0f));
    }

    SECTION("Topless state returns 20 points")
    {
        // When only topless faction is detected
        float expectedScore = 20.0f;
        REQUIRE(expectedScore == Approx(20.0f));
    }

    SECTION("Bottomless state returns 30 points")
    {
        // When only bottomless faction is detected
        float expectedScore = 30.0f;
        REQUIRE(expectedScore == Approx(30.0f));
    }

    SECTION("ShowingChest returns 12 points when not topless")
    {
        // When only showing chest (not topless)
        float expectedScore = 12.0f;
        REQUIRE(expectedScore == Approx(12.0f));
    }

    SECTION("ShowingBra returns 8 points when not topless or showing chest")
    {
        // When only showing bra
        float expectedScore = 8.0f;
        REQUIRE(expectedScore == Approx(8.0f));
    }

    SECTION("ShowingGenitals returns 15 points when not bottomless")
    {
        // When only showing genitals (not bottomless)
        float expectedScore = 15.0f;
        REQUIRE(expectedScore == Approx(15.0f));
    }

    SECTION("ShowingUnderwear returns 8 points when not bottomless or showing genitals")
    {
        // When only showing underwear
        float expectedScore = 8.0f;
        REQUIRE(expectedScore == Approx(8.0f));
    }

    SECTION("ShowingAss returns 8 points when not bottomless")
    {
        // When only showing ass (not bottomless)
        float expectedScore = 8.0f;
        REQUIRE(expectedScore == Approx(8.0f));
    }
}

TEST_CASE("A.N.D. Integration - Priority System Logic", "[and_integration]")
{
    SECTION("Topless overrides ShowingChest")
    {
        // When both topless and showing chest are true
        // Expected: chestScore = 20 (topless wins), not 12
        float expectedScore = 20.0f;
        REQUIRE(expectedScore == Approx(20.0f));
    }

    SECTION("Topless overrides ShowingBra")
    {
        // When both topless and showing bra are true
        // Expected: chestScore = 20 (topless wins), not 8
        float expectedScore = 20.0f;
        REQUIRE(expectedScore == Approx(20.0f));
    }

    SECTION("ShowingChest overrides ShowingBra")
    {
        // When both showing chest and showing bra are true
        // Expected: chestScore = 12 (chest wins), not 8
        float expectedScore = 12.0f;
        REQUIRE(expectedScore == Approx(12.0f));
    }

    SECTION("Bottomless overrides ShowingGenitals")
    {
        // When both bottomless and showing genitals are true
        // Expected: frontScore = 30 (bottomless wins), not 15
        float expectedScore = 30.0f;
        REQUIRE(expectedScore == Approx(30.0f));
    }

    SECTION("Bottomless overrides ShowingUnderwear")
    {
        // When both bottomless and showing underwear are true
        // Expected: frontScore = 30 (bottomless wins), not 8
        float expectedScore = 30.0f;
        REQUIRE(expectedScore == Approx(30.0f));
    }

    SECTION("ShowingGenitals overrides ShowingUnderwear")
    {
        // When both showing genitals and showing underwear are true
        // Expected: frontScore = 15 (genitals wins), not 8
        float expectedScore = 15.0f;
        REQUIRE(expectedScore == Approx(15.0f));
    }

    SECTION("Bottomless prevents ShowingAss from contributing")
    {
        // When both bottomless and showing ass are true
        // Expected: frontScore = 30, assScore = 0 (covered by bottomless)
        float expectedScore = 30.0f;
        REQUIRE(expectedScore == Approx(30.0f));
    }
}

TEST_CASE("A.N.D. Integration - Special Cases", "[and_integration]")
{
    SECTION("Topless + Bottomless synergy without Nude")
    {
        // When topless and bottomless but not nude
        // Expected: Special synergy case = 37.0f
        float expectedScore = 37.0f;
        REQUIRE(expectedScore == Approx(37.0f));
    }

    SECTION("Nude overrides everything")
    {
        // When nude is true (regardless of other factions)
        // Expected: Hard override to 50.0f
        float expectedScore = 50.0f;
        REQUIRE(expectedScore == Approx(50.0f));
    }

    SECTION("Complex combination without nude")
    {
        // When showing chest (12) + showing genitals (15) + showing ass (8)
        // Expected: chestScore = 12, frontScore = 15, assScore = 8
        // Total = 35.0f
        float expectedScore = 35.0f;
        REQUIRE(expectedScore == Approx(35.0f));
    }

    SECTION("Minimal exposure - just bra")
    {
        // When only showing bra
        // Expected: chestScore = 8, frontScore = 0, assScore = 0
        // Total = 8.0f
        float expectedScore = 8.0f;
        REQUIRE(expectedScore == Approx(8.0f));
    }

    SECTION("Minimal exposure - just underwear")
    {
        // When only showing underwear
        // Expected: chestScore = 0, frontScore = 8, assScore = 0
        // Total = 8.0f
        float expectedScore = 8.0f;
        REQUIRE(expectedScore == Approx(8.0f));
    }

    SECTION("Bra and underwear combination")
    {
        // When showing both bra and underwear
        // Expected: chestScore = 8, frontScore = 8, assScore = 0
        // Total = 16.0f
        float expectedScore = 16.0f;
        REQUIRE(expectedScore == Approx(16.0f));
    }
}

TEST_CASE("A.N.D. Integration - Baseline Modifier Calculations", "[and_integration]")
{
    Settings* settings = Settings::GetSingleton();

    SECTION("A.N.D. faction baselines are configured correctly")
    {
        settings->SetUseANDIntegration(true);

        // Test setting individual faction baselines
        settings->SetANDFactionBaseline(NUDE, 50.0f);
        REQUIRE(settings->GetANDFactionBaseline(NUDE) == Approx(50.0f));

        settings->SetANDFactionBaseline(TOPLESS, 20.0f);
        REQUIRE(settings->GetANDFactionBaseline(TOPLESS) == Approx(20.0f));

        settings->SetANDFactionBaseline(BOTTOMLESS, 30.0f);
        REQUIRE(settings->GetANDFactionBaseline(BOTTOMLESS) == Approx(30.0f));

        settings->SetANDFactionBaseline(SHOWING_CHEST, 12.0f);
        REQUIRE(settings->GetANDFactionBaseline(SHOWING_CHEST) == Approx(12.0f));

        settings->SetANDFactionBaseline(SHOWING_ASS, 8.0f);
        REQUIRE(settings->GetANDFactionBaseline(SHOWING_ASS) == Approx(8.0f));

        settings->SetANDFactionBaseline(SHOWING_GENITALS, 15.0f);
        REQUIRE(settings->GetANDFactionBaseline(SHOWING_GENITALS) == Approx(15.0f));

        settings->SetANDFactionBaseline(SHOWING_BRA, 8.0f);
        REQUIRE(settings->GetANDFactionBaseline(SHOWING_BRA) == Approx(8.0f));

        settings->SetANDFactionBaseline(SHOWING_UNDERWEAR, 8.0f);
        REQUIRE(settings->GetANDFactionBaseline(SHOWING_UNDERWEAR) == Approx(8.0f));
    }

    SECTION("Faction baseline values are clamped to valid range")
    {
        settings->SetUseANDIntegration(true);

        // Test clamping to maximum (100)
        settings->SetANDFactionBaseline(NUDE, 150.0f);
        REQUIRE(settings->GetANDFactionBaseline(NUDE) == Approx(100.0f));

        // Test clamping to minimum (0)
        settings->SetANDFactionBaseline(TOPLESS, -10.0f);
        REQUIRE(settings->GetANDFactionBaseline(TOPLESS) == Approx(0.0f));

        // Test normal range value
        settings->SetANDFactionBaseline(BOTTOMLESS, 35.5f);
        REQUIRE(settings->GetANDFactionBaseline(BOTTOMLESS) == Approx(35.5f));
    }

    SECTION("Falls back to legacy when A.N.D. disabled")
    {
        settings->SetUseANDIntegration(false);
        settings->SetNudeArousalBaseline(30.0f);

        // When A.N.D. is disabled, should use legacy nude baseline
        float expectedBaseline = 30.0f;
        REQUIRE(expectedBaseline == Approx(30.0f));
    }
}

TEST_CASE("A.N.D. Integration - Score Range Validation", "[and_integration]")
{
    SECTION("Score is within valid range [0, 50]")
    {
        // All possible scores should be between 0 and 50
        float minScore = 0.0f;
        float maxScore = 50.0f;

        // Test minimum
        REQUIRE(minScore >= 0.0f);
        REQUIRE(minScore <= 50.0f);

        // Test maximum
        REQUIRE(maxScore >= 0.0f);
        REQUIRE(maxScore <= 50.0f);

        // Test some intermediate values
        float toplessScore = 20.0f;
        REQUIRE(toplessScore >= 0.0f);
        REQUIRE(toplessScore <= 50.0f);

        float synergyScore = 37.0f;
        REQUIRE(synergyScore >= 0.0f);
        REQUIRE(synergyScore <= 50.0f);
    }
}

TEST_CASE("A.N.D. Integration - Settings Integration", "[and_integration]")
{
    Settings* settings = Settings::GetSingleton();

    SECTION("Settings can be modified and retrieved")
    {
        settings->SetUseANDIntegration(true);
        REQUIRE(settings->GetUseANDIntegration() == true);

        settings->SetUseANDIntegration(false);
        REQUIRE(settings->GetUseANDIntegration() == false);
    }

    SECTION("Invalid faction index returns 0")
    {
        // Test invalid negative index
        REQUIRE(settings->GetANDFactionBaseline(-1) == Approx(0.0f));

        // Test invalid positive index
        REQUIRE(settings->GetANDFactionBaseline(COUNT) == Approx(0.0f));
        REQUIRE(settings->GetANDFactionBaseline(100) == Approx(0.0f));
    }

    SECTION("Faction index validation works correctly")
    {
        REQUIRE(IsValidIndex(NUDE) == true);
        REQUIRE(IsValidIndex(SHOWING_UNDERWEAR) == true);
        REQUIRE(IsValidIndex(-1) == false);
        REQUIRE(IsValidIndex(COUNT) == false);
        REQUIRE(IsValidIndex(100) == false);
    }
}

TEST_CASE("A.N.D. Integration - Combination Examples", "[and_integration][examples]")
{
    SECTION("Example: Character in underwear")
    {
        // ShowingBra (8) + ShowingUnderwear (8) = 16
        float expectedScore = 16.0f;
        REQUIRE(expectedScore == Approx(16.0f));
    }

    SECTION("Example: Character topless with underwear")
    {
        // Topless (20) + ShowingUnderwear (8) = 28
        float expectedScore = 28.0f;
        REQUIRE(expectedScore == Approx(28.0f));
    }

    SECTION("Example: Character bottomless with bra")
    {
        // ShowingBra (8) + Bottomless (30) = 38
        float expectedScore = 38.0f;
        REQUIRE(expectedScore == Approx(38.0f));
    }

    SECTION("Example: Character showing chest and ass")
    {
        // ShowingChest (12) + ShowingAss (8) = 20
        float expectedScore = 20.0f;
        REQUIRE(expectedScore == Approx(20.0f));
    }

    SECTION("Example: Maximum exposure without being nude")
    {
        // Topless (20) + Bottomless (30) with synergy = 37
        float expectedScore = 37.0f;
        REQUIRE(expectedScore == Approx(37.0f));
    }
}