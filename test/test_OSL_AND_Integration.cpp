#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>

using Catch::Approx;

// Mock implementations for testing OSL AND integration
// These mirror the baseline calculation logic in ArousalSystemOSL

namespace TestHelpers {
    // Calculate baseline arousal with AND nudity contribution
    float CalculateBaselineWithAND(float baseLibido, float andNudityScore,
                                    bool viewingNude = false, bool inScene = false,
                                    bool viewingScene = false, float deviceContribution = 0.0f) {
        float baseline = baseLibido;

        // AND nudity score contribution (0-50 range)
        baseline += andNudityScore;

        // Other modifiers
        if (viewingNude) baseline += 20.0f;
        if (inScene) baseline += 50.0f;
        if (viewingScene) baseline += 30.0f;
        baseline += deviceContribution;

        return std::min(100.0f, baseline);
    }

    // OSL arousal convergence
    float CalculateOSLConvergence(float currentArousal, float targetBaseline,
                                   float epsilon, float gameHoursPassed) {
        float t = 1.0f - std::pow(epsilon, gameHoursPassed);
        return std::lerp(currentArousal, targetBaseline, t);
    }

    float UpdateArousalToBaseline(float currentArousal, float baseline, float gameHoursPassed) {
        constexpr float kDefaultArousalChangeRate = 0.2f;  // 80% convergence per hour
        return CalculateOSLConvergence(currentArousal, baseline, kDefaultArousalChangeRate, gameHoursPassed);
    }
}

TEST_CASE("OSL AND Integration - Baseline Nudity Contributions", "[OSL][AND][Baseline]") {

    SECTION("Full nudity (score 50) adds 50 to baseline") {
        float baseLibido = 10.0f;
        float andScore = 50.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(60.0f));  // 10 + 50
    }

    SECTION("Topless (score 20) adds 20 to baseline") {
        float baseLibido = 10.0f;
        float andScore = 20.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(30.0f));  // 10 + 20
    }

    SECTION("Bottomless (score 30) adds 30 to baseline") {
        float baseLibido = 15.0f;
        float andScore = 30.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(45.0f));  // 15 + 30
    }

    SECTION("Showing chest (score 12) adds 12 to baseline") {
        float baseLibido = 10.0f;
        float andScore = 12.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(22.0f));  // 10 + 12
    }

    SECTION("Minimal nudity (score 8) adds 8 to baseline") {
        float baseLibido = 10.0f;
        float andScore = 8.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(18.0f));  // 10 + 8
    }

    SECTION("Topless + Bottomless synergy (score 37) adds 37 to baseline") {
        float baseLibido = 10.0f;
        float andScore = 37.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(47.0f));  // 10 + 37
    }

    SECTION("No nudity (score 0) adds nothing to baseline") {
        float baseLibido = 10.0f;
        float andScore = 0.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(10.0f));  // 10 + 0
    }
}

TEST_CASE("OSL AND Integration - Baseline with Multiple Modifiers", "[OSL][AND][Baseline]") {

    SECTION("Nudity + viewing nude") {
        float baseLibido = 10.0f;
        float andScore = 20.0f;  // Topless
        bool viewingNude = true;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, viewingNude);
        REQUIRE(baseline == Approx(50.0f));  // 10 + 20 + 20
    }

    SECTION("Nudity + in scene") {
        float baseLibido = 10.0f;
        float andScore = 30.0f;  // Bottomless
        bool viewingNude = false;
        bool inScene = true;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, viewingNude, inScene);
        REQUIRE(baseline == Approx(90.0f));  // 10 + 30 + 50
    }

    SECTION("Nudity + viewing scene") {
        float baseLibido = 10.0f;
        float andScore = 12.0f;  // Showing chest
        bool viewingNude = false;
        bool inScene = false;
        bool viewingScene = true;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, viewingNude, inScene, viewingScene);
        REQUIRE(baseline == Approx(52.0f));  // 10 + 12 + 30
    }

    SECTION("Full nudity + devices") {
        float baseLibido = 10.0f;
        float andScore = 50.0f;  // Fully nude
        float deviceContribution = 30.0f;  // 3 devices * 10 each

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, false, false, false, deviceContribution);
        REQUIRE(baseline == Approx(90.0f));  // 10 + 50 + 30
    }

    SECTION("Maximum stacking - all modifiers") {
        float baseLibido = 20.0f;
        float andScore = 50.0f;  // Fully nude
        bool viewingNude = true;   // +20
        bool inScene = true;       // +50
        bool viewingScene = true;  // +30
        float deviceContribution = 40.0f;  // +40

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, viewingNude, inScene, viewingScene, deviceContribution);
        // Total would be 20 + 50 + 20 + 50 + 30 + 40 = 210, but caps at 100
        REQUIRE(baseline == Approx(100.0f));
    }

    SECTION("Baseline capping with nudity") {
        float baseLibido = 30.0f;
        float andScore = 50.0f;  // Fully nude
        bool inScene = true;     // +50

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, false, inScene);
        // Total would be 30 + 50 + 50 = 130, but caps at 100
        REQUIRE(baseline == Approx(100.0f));
    }
}

TEST_CASE("OSL AND Integration - Arousal Convergence with AND Baseline", "[OSL][AND][Convergence]") {

    SECTION("Converging to topless baseline") {
        float currentArousal = 10.0f;
        float baseLibido = 10.0f;
        float andScore = 20.0f;  // Topless
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);

        // After 1 hour, should be 80% converged
        float newArousal = TestHelpers::UpdateArousalToBaseline(currentArousal, baseline, 1.0f);
        REQUIRE(newArousal == Approx(26.0f).epsilon(0.01));  // 10 + (30-10)*0.8
    }

    SECTION("Converging to fully nude baseline") {
        float currentArousal = 20.0f;
        float baseLibido = 10.0f;
        float andScore = 50.0f;  // Fully nude
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);

        // After 1 hour
        float newArousal = TestHelpers::UpdateArousalToBaseline(currentArousal, baseline, 1.0f);
        REQUIRE(newArousal == Approx(52.0f).epsilon(0.01));  // 20 + (60-20)*0.8
    }

    SECTION("Decreasing from high arousal to minimal nudity baseline") {
        float currentArousal = 80.0f;
        float baseLibido = 10.0f;
        float andScore = 8.0f;  // Minimal nudity
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);

        // After 1 hour
        float newArousal = TestHelpers::UpdateArousalToBaseline(currentArousal, baseline, 1.0f);
        REQUIRE(newArousal == Approx(30.4f).epsilon(0.01));  // 80 - (80-18)*0.8
    }

    SECTION("Multiple updates as nudity level changes") {
        float arousal = 30.0f;
        float baseLibido = 10.0f;

        // Start wearing underwear (score 8)
        float baseline1 = TestHelpers::CalculateBaselineWithAND(baseLibido, 8.0f);
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline1, 0.5f);
        REQUIRE(arousal < 30.0f);  // Decreasing

        // Remove top (now topless, score 20)
        float baseline2 = TestHelpers::CalculateBaselineWithAND(baseLibido, 20.0f);
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline2, 0.5f);
        // Should be increasing now
        REQUIRE(arousal > baseline1);

        // Remove bottom (now nude, score 50)
        float baseline3 = TestHelpers::CalculateBaselineWithAND(baseLibido, 50.0f);
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline3, 0.5f);
        // Should be significantly higher
        REQUIRE(arousal > 40.0f);
    }
}

TEST_CASE("OSL AND Integration - Real-World Scenarios", "[OSL][AND][Scenarios]") {

    SECTION("Character gradually undressing") {
        float arousal = 10.0f;
        float baseLibido = 10.0f;
        float timeStep = 0.25f;  // 15 minute intervals

        // Fully clothed (score 0)
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, 0.0f);
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, timeStep);
        float arousedClothed = arousal;

        // Remove top (topless, score 20)
        baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, 20.0f);
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, timeStep);
        float arousedTopless = arousal;

        // Remove bottom (fully nude, score 50)
        baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, 50.0f);
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, timeStep);
        float arousedNude = arousal;

        // Arousal should increase at each step
        REQUIRE(arousedTopless > arousedClothed);
        REQUIRE(arousedNude > arousedTopless);
    }

    SECTION("Wearing revealing armor in combat") {
        float arousal = 30.0f;
        float baseLibido = 15.0f;
        float andScore = 12.0f;  // Showing chest
        float deviceContribution = 20.0f;  // Erotic armor

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, false, false, false, deviceContribution);
        REQUIRE(baseline == Approx(47.0f));  // 15 + 12 + 20

        // After 1 hour of combat
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, 1.0f);
        REQUIRE(arousal > 40.0f);
    }

    SECTION("Exhibitionist in public while topless") {
        float arousal = 20.0f;
        float baseLibido = 20.0f;  // High libido exhibitionist
        float andScore = 20.0f;    // Topless
        bool viewingNude = true;   // Others can see them

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, viewingNude);
        REQUIRE(baseline == Approx(60.0f));  // 20 + 20 + 20

        // After 0.5 hours
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, 0.5f);
        REQUIRE(arousal > 35.0f);
    }

    SECTION("Participant in scene wearing lingerie") {
        float arousal = 50.0f;
        float baseLibido = 15.0f;
        float andScore = 16.0f;  // Bra + underwear
        bool inScene = true;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore, false, inScene);
        REQUIRE(baseline == Approx(81.0f));  // 15 + 16 + 50

        // After 0.25 hours (15 minutes)
        arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, 0.25f);
        REQUIRE(arousal > 60.0f);
    }

    SECTION("Long-term baseline maintenance") {
        float arousal = 30.0f;
        float baseLibido = 10.0f;
        float andScore = 20.0f;  // Topless
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);

        // Simulate staying topless for several game hours
        for (int i = 0; i < 5; ++i) {
            arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, 1.0f);
        }

        // Should have converged very close to baseline
        REQUIRE(arousal == Approx(baseline).epsilon(0.01));
    }
}

TEST_CASE("OSL AND Integration - Comparison with Legacy Nudity", "[OSL][AND][Comparison]") {

    SECTION("Legacy nude baseline vs AND full nude") {
        float baseLibido = 10.0f;

        // Legacy: Fixed 30 point nude baseline
        float legacyBaseline = baseLibido + 30.0f;
        REQUIRE(legacyBaseline == 40.0f);

        // AND: Full nude gives 50 points
        float andBaseline = TestHelpers::CalculateBaselineWithAND(baseLibido, 50.0f);
        REQUIRE(andBaseline == 60.0f);

        // AND provides higher baseline for full nudity
        REQUIRE(andBaseline > legacyBaseline);
    }

    SECTION("AND provides granular intermediate states") {
        float baseLibido = 10.0f;

        // Legacy: Binary (either 30 or 0)
        float legacyClothed = baseLibido + 0.0f;   // 10
        float legacyNaked = baseLibido + 30.0f;    // 40

        // AND: Multiple intermediate states
        float andUnderwear = TestHelpers::CalculateBaselineWithAND(baseLibido, 8.0f);    // 18
        float andTopless = TestHelpers::CalculateBaselineWithAND(baseLibido, 20.0f);    // 30
        float andBottomless = TestHelpers::CalculateBaselineWithAND(baseLibido, 30.0f); // 40
        float andNude = TestHelpers::CalculateBaselineWithAND(baseLibido, 50.0f);       // 60

        // AND provides smooth gradation
        REQUIRE(andUnderwear > legacyClothed);
        REQUIRE(andUnderwear < legacyNaked);
        REQUIRE(andTopless < legacyNaked);
        REQUIRE(andBottomless == legacyNaked);
        REQUIRE(andNude > legacyNaked);
    }

    SECTION("Arousal convergence differs with granular baselines") {
        float currentArousal = 50.0f;
        float baseLibido = 10.0f;

        // Legacy: Jump from 10 to 40 when getting naked
        float legacyTarget = 40.0f;
        float legacyResult = TestHelpers::UpdateArousalToBaseline(currentArousal, legacyTarget, 1.0f);

        // AND: Gradual progression through intermediate states
        // First topless (30)
        float andToplessResult = TestHelpers::UpdateArousalToBaseline(currentArousal, 30.0f, 0.5f);
        // Then nude (60)
        float andNudeResult = TestHelpers::UpdateArousalToBaseline(andToplessResult, 60.0f, 0.5f);

        // AND path differs from legacy
        REQUIRE(andToplessResult < currentArousal);
        REQUIRE(andNudeResult != legacyResult);
    }
}

TEST_CASE("OSL AND Integration - Edge Cases", "[OSL][AND][EdgeCases]") {

    SECTION("Zero libido with nudity") {
        float baseLibido = 0.0f;
        float andScore = 50.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        REQUIRE(baseline == Approx(50.0f));
    }

    SECTION("Maximum libido with maximum nudity") {
        float baseLibido = 100.0f;
        float andScore = 50.0f;

        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);
        // Would be 150, but caps at 100
        REQUIRE(baseline == Approx(100.0f));
    }

    SECTION("Minimal arousal converging to high nudity baseline") {
        float currentArousal = 0.0f;
        float baseLibido = 10.0f;
        float andScore = 50.0f;
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);

        float newArousal = TestHelpers::UpdateArousalToBaseline(currentArousal, baseline, 1.0f);
        // Should make significant progress toward baseline
        REQUIRE(newArousal > 40.0f);
    }

    SECTION("Maximum arousal converging to minimal baseline") {
        float currentArousal = 100.0f;
        float baseLibido = 10.0f;
        float andScore = 0.0f;
        float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, andScore);

        float newArousal = TestHelpers::UpdateArousalToBaseline(currentArousal, baseline, 1.0f);
        // Should drop significantly
        REQUIRE(newArousal < 30.0f);
    }

    SECTION("Rapid nudity state changes") {
        float arousal = 30.0f;
        float baseLibido = 10.0f;

        // Rapidly cycle through states
        std::vector<float> nudityScores = {0.0f, 20.0f, 8.0f, 50.0f, 12.0f};
        for (float score : nudityScores) {
            float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, score);
            arousal = TestHelpers::UpdateArousalToBaseline(arousal, baseline, 0.1f);
        }

        // Arousal should be partially converged to last baseline (22)
        REQUIRE(arousal > 20.0f);
        REQUIRE(arousal < 30.0f);
    }
}

TEST_CASE("OSL AND Integration - Baseline Range Validation", "[OSL][AND][Validation]") {

    SECTION("All valid AND scores produce valid baselines") {
        float baseLibido = 10.0f;
        std::vector<float> testScores = {0.0f, 8.0f, 12.0f, 15.0f, 20.0f, 30.0f, 37.0f, 50.0f};

        for (float score : testScores) {
            float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, score);
            REQUIRE(baseline >= 10.0f);  // At least base libido
            REQUIRE(baseline <= 100.0f); // Capped at 100
        }
    }

    SECTION("Baseline increases monotonically with AND score") {
        float baseLibido = 10.0f;
        float previousBaseline = 0.0f;

        for (float score = 0.0f; score <= 50.0f; score += 5.0f) {
            float baseline = TestHelpers::CalculateBaselineWithAND(baseLibido, score);
            REQUIRE(baseline >= previousBaseline);
            previousBaseline = baseline;
        }
    }

    SECTION("Baseline delta matches AND score delta") {
        float baseLibido = 10.0f;

        float baseline1 = TestHelpers::CalculateBaselineWithAND(baseLibido, 10.0f);
        float baseline2 = TestHelpers::CalculateBaselineWithAND(baseLibido, 30.0f);

        // Difference should be 20 (30 - 10)
        REQUIRE((baseline2 - baseline1) == Approx(20.0f));
    }
}
