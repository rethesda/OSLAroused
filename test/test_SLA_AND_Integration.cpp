#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>

using Catch::Approx;

// Mock implementations for testing SLA AND integration
// These mirror the spectating logic in ArousalSystemSLA::HandleSpectatingNaked

namespace TestHelpers {
    float CalculateSpectatingExposureScale(float elapsedGameTime, float updateInterval = 0.1f) {
        return std::min(1.0f, elapsedGameTime / updateInterval);
    }

    // Calculate exposure gain with AND nudity scaling
    float CalculateSpectatingExposure(float baseExposure, float exposureScale, float nudityScale) {
        return baseExposure * exposureScale * nudityScale;
    }

    // Calculate nudity scale from AND score (0-50 range)
    float CalculateNudityScale(float andScore) {
        return andScore / 50.0f;
    }
}

TEST_CASE("SLA AND Integration - Nudity Score Scaling", "[SLA][AND][Spectating]") {

    SECTION("Full nudity (score 50) gives full exposure") {
        float andScore = 50.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == Approx(1.0f));

        // Preferred gender spectating
        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(4.0f));
    }

    SECTION("Topless (score 20) gives 40% exposure") {
        float andScore = 20.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == Approx(0.4f));

        // Preferred gender spectating
        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(1.6f));
    }

    SECTION("Bottomless (score 30) gives 60% exposure") {
        float andScore = 30.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == Approx(0.6f));

        // Non-preferred gender spectating
        float baseExposure = 2.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(1.2f));
    }

    SECTION("Showing chest (score 12) gives 24% exposure") {
        float andScore = 12.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == Approx(0.24f));

        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(0.96f));
    }

    SECTION("Showing underwear (score 8) gives 16% exposure") {
        float andScore = 8.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == Approx(0.16f));

        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(0.64f));
    }

    SECTION("Topless + Bottomless synergy (score 37) gives 74% exposure") {
        float andScore = 37.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == Approx(0.74f));

        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(2.96f));
    }
}

TEST_CASE("SLA AND Integration - Preferred Gender Scaling", "[SLA][AND][Spectating]") {

    SECTION("Preferred gender with full nudity") {
        float baseExposure = 4.0f;  // Preferred gender base
        float exposureScale = 1.0f;
        float nudityScale = 1.0f;   // Full nudity (50/50)

        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(4.0f));
    }

    SECTION("Preferred gender with topless") {
        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float nudityScale = 0.4f;   // Topless (20/50)

        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(1.6f));
    }

    SECTION("Non-preferred gender with full nudity") {
        float baseExposure = 2.0f;  // Non-preferred gender base
        float exposureScale = 1.0f;
        float nudityScale = 1.0f;   // Full nudity

        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(2.0f));
    }

    SECTION("Non-preferred gender with topless") {
        float baseExposure = 2.0f;
        float exposureScale = 1.0f;
        float nudityScale = 0.4f;   // Topless

        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(0.8f));
    }
}

TEST_CASE("SLA AND Integration - Exhibitionist Bonus Scaling", "[SLA][AND][Spectating]") {

    SECTION("Exhibitionist bonus with full nudity") {
        float baseBonus = 2.0f;
        float exposureScale = 1.0f;
        float nudityScale = 1.0f;  // Full nudity

        float bonusGain = TestHelpers::CalculateSpectatingExposure(baseBonus, exposureScale, nudityScale);
        REQUIRE(bonusGain == Approx(2.0f));
    }

    SECTION("Exhibitionist bonus with topless") {
        float baseBonus = 2.0f;
        float exposureScale = 1.0f;
        float nudityScale = 0.4f;  // Topless (20/50)

        float bonusGain = TestHelpers::CalculateSpectatingExposure(baseBonus, exposureScale, nudityScale);
        REQUIRE(bonusGain == Approx(0.8f));
    }

    SECTION("Exhibitionist bonus with underwear only") {
        float baseBonus = 2.0f;
        float exposureScale = 1.0f;
        float nudityScale = 0.16f;  // Showing underwear (8/50)

        float bonusGain = TestHelpers::CalculateSpectatingExposure(baseBonus, exposureScale, nudityScale);
        REQUIRE(bonusGain == Approx(0.32f));
    }
}

TEST_CASE("SLA AND Integration - Time Scaling Combined with Nudity", "[SLA][AND][Spectating]") {

    SECTION("Full update interval with full nudity") {
        float elapsedGameTime = 0.1f;
        float updateInterval = 0.1f;
        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);

        REQUIRE(exposureScale == 1.0f);

        float baseExposure = 4.0f;
        float nudityScale = 1.0f;  // Full nudity
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == Approx(4.0f));
    }

    SECTION("Half update interval with topless") {
        float elapsedGameTime = 0.05f;
        float updateInterval = 0.1f;
        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);

        REQUIRE(exposureScale == 0.5f);

        float baseExposure = 4.0f;
        float nudityScale = 0.4f;  // Topless (20/50)
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);

        // 4 * 0.5 * 0.4 = 0.8
        REQUIRE(exposureGain == Approx(0.8f));
    }

    SECTION("Quarter update interval with bottomless") {
        float elapsedGameTime = 0.025f;
        float updateInterval = 0.1f;
        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);

        REQUIRE(exposureScale == 0.25f);

        float baseExposure = 4.0f;
        float nudityScale = 0.6f;  // Bottomless (30/50)
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);

        // 4 * 0.25 * 0.6 = 0.6
        REQUIRE(exposureGain == Approx(0.6f));
    }

    SECTION("Rapid updates with minimal nudity") {
        float elapsedGameTime = 0.01f;  // Very rapid update
        float updateInterval = 0.1f;
        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);

        REQUIRE(exposureScale == 0.1f);

        float baseExposure = 4.0f;
        float nudityScale = 0.16f;  // Showing underwear (8/50)
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);

        // 4 * 0.1 * 0.16 = 0.064
        REQUIRE(exposureGain == Approx(0.064f));
    }
}

TEST_CASE("SLA AND Integration - Edge Cases", "[SLA][AND][EdgeCases]") {

    SECTION("Zero nudity score gives no exposure") {
        float andScore = 0.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == 0.0f);

        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == 0.0f);
    }

    SECTION("Maximum nudity score gives full exposure") {
        float andScore = 50.0f;
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        REQUIRE(nudityScale == 1.0f);

        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == 4.0f);
    }

    SECTION("Zero time elapsed gives no exposure") {
        float elapsedGameTime = 0.0f;
        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime);

        REQUIRE(exposureScale == 0.0f);

        float baseExposure = 4.0f;
        float nudityScale = 1.0f;
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == 0.0f);
    }

    SECTION("All factors at maximum") {
        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float nudityScale = 1.0f;

        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == 4.0f);
    }

    SECTION("All factors at minimum") {
        float baseExposure = 0.0f;
        float exposureScale = 0.0f;
        float nudityScale = 0.0f;

        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        REQUIRE(exposureGain == 0.0f);
    }
}

TEST_CASE("SLA AND Integration - Real-World Scenarios", "[SLA][AND][Scenarios]") {

    SECTION("Spectating fully nude preferred gender over full interval") {
        // Typical scenario: seeing a fully nude NPC of preferred gender
        float baseExposure = 4.0f;
        float elapsedGameTime = 0.1f;
        float updateInterval = 0.1f;
        float andScore = 50.0f;

        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);

        // Should get full 4.0 exposure
        REQUIRE(exposureGain == Approx(4.0f));
    }

    SECTION("Spectating topless NPC briefly") {
        // Scenario: quickly passing by a topless NPC
        float baseExposure = 2.0f;  // Non-preferred gender
        float elapsedGameTime = 0.025f;  // Brief encounter
        float updateInterval = 0.1f;
        float andScore = 20.0f;  // Topless

        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);
        float exposureGain = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);

        // 2 * 0.25 * 0.4 = 0.2
        REQUIRE(exposureGain == Approx(0.2f));
    }

    SECTION("Exhibitionist showing off in underwear") {
        // Scenario: exhibitionist wearing only underwear
        float baseBonus = 2.0f;
        float elapsedGameTime = 0.1f;
        float updateInterval = 0.1f;
        float andScore = 16.0f;  // Bra + underwear

        float exposureScale = TestHelpers::CalculateSpectatingExposureScale(elapsedGameTime, updateInterval);
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);
        float bonusGain = TestHelpers::CalculateSpectatingExposure(baseBonus, exposureScale, nudityScale);

        // 2 * 1.0 * 0.32 = 0.64
        REQUIRE(bonusGain == Approx(0.64f));
    }

    SECTION("Multiple spectators viewing topless + bottomless") {
        // Scenario: character with topless + bottomless synergy
        float andScore = 37.0f;  // Synergy score
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);

        // Spectator 1: Preferred gender, full interval
        float exposure1 = TestHelpers::CalculateSpectatingExposure(4.0f, 1.0f, nudityScale);
        REQUIRE(exposure1 == Approx(2.96f));

        // Spectator 2: Non-preferred gender, half interval
        float exposure2 = TestHelpers::CalculateSpectatingExposure(2.0f, 0.5f, nudityScale);
        REQUIRE(exposure2 == Approx(0.74f));

        // Exhibitionist bonus for the naked actor
        float exhibBonus = TestHelpers::CalculateSpectatingExposure(2.0f, 1.0f, nudityScale);
        REQUIRE(exhibBonus == Approx(1.48f));
    }

    SECTION("Rapid polling with showing chest") {
        // Scenario: Very frequent updates (high performance mode)
        float baseExposure = 4.0f;
        float andScore = 12.0f;  // Showing chest
        float nudityScale = TestHelpers::CalculateNudityScale(andScore);
        float totalExposure = 0.0f;

        // Simulate 10 rapid updates totaling 0.1 game hours
        for (int i = 0; i < 10; ++i) {
            float exposureScale = TestHelpers::CalculateSpectatingExposureScale(0.01f, 0.1f);
            totalExposure += TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, nudityScale);
        }

        // 10 * (4 * 0.1 * 0.24) = 10 * 0.096 = 0.96
        REQUIRE(totalExposure == Approx(0.96f));
    }
}

TEST_CASE("SLA AND Integration - Comparison with Legacy", "[SLA][AND][Comparison]") {

    SECTION("Legacy behavior equivalent to score 50") {
        // Without AND, all naked actors are treated as fully nude (score 50)
        float baseExposure = 4.0f;
        float exposureScale = 1.0f;
        float legacyNudityScale = 1.0f;

        float legacyExposure = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, legacyNudityScale);

        // AND with score 50 should match legacy
        float andNudityScale = TestHelpers::CalculateNudityScale(50.0f);
        float andExposure = TestHelpers::CalculateSpectatingExposure(baseExposure, exposureScale, andNudityScale);

        REQUIRE(legacyExposure == andExposure);
    }

    SECTION("AND provides granularity that legacy doesn't") {
        // Legacy: binary naked check
        float legacyExposure = 4.0f * 1.0f * 1.0f;  // Always 4.0 if naked

        // AND: gradual based on nudity level
        float toplessExposure = 4.0f * 1.0f * TestHelpers::CalculateNudityScale(20.0f);  // 1.6
        float underwearExposure = 4.0f * 1.0f * TestHelpers::CalculateNudityScale(8.0f);  // 0.32

        REQUIRE(toplessExposure < legacyExposure);
        REQUIRE(underwearExposure < legacyExposure);
        REQUIRE(toplessExposure > underwearExposure);
    }
}

TEST_CASE("SLA AND Integration - Nudity Scale Range Validation", "[SLA][AND][Validation]") {

    SECTION("All valid AND scores produce valid scales") {
        std::vector<float> testScores = {0.0f, 8.0f, 12.0f, 15.0f, 20.0f, 30.0f, 37.0f, 50.0f};

        for (float score : testScores) {
            float scale = TestHelpers::CalculateNudityScale(score);
            REQUIRE(scale >= 0.0f);
            REQUIRE(scale <= 1.0f);
        }
    }

    SECTION("Nudity scale is proportional to AND score") {
        float score1 = 10.0f;
        float score2 = 20.0f;
        float score3 = 40.0f;

        float scale1 = TestHelpers::CalculateNudityScale(score1);
        float scale2 = TestHelpers::CalculateNudityScale(score2);
        float scale3 = TestHelpers::CalculateNudityScale(score3);

        REQUIRE(scale2 == Approx(scale1 * 2.0f));
        REQUIRE(scale3 == Approx(scale1 * 4.0f));
    }
}
