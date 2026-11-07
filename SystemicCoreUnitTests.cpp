#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdint>

// Inlined core logic blocks to maintain compile-time encapsulation
namespace MetamorphosisEngine {
    constexpr double LN_2 = 0.6931471805599453;
    constexpr double TRANSITION_LIMIT = 1e-11;

    // Test Harness Domain 32: QCD Core
    class QcdProcessor {
    private:
        double criticalEnergyDensity; // GeV/fm^3
    public:
        constexpr QcdProcessor(double criticalCeiling) noexcept 
            : criticalEnergyDensity(criticalCeiling > 0.0 ? criticalCeiling : 1.0) {}

        double calculateEntropy(double k) noexcept {
            if (k >= 1.0) return LN_2;
            double p_o = 1.0 - (0.5 * k * k);
            double p_c = 1.0 - p_o;
            p_o = p_o < TRANSITION_LIMIT ? TRANSITION_LIMIT : p_o;
            p_c = p_c < TRANSITION_LIMIT ? TRANSITION_LIMIT : p_c;
            return -((p_o * std::log(p_o)) + (p_c * std::log(p_c)));
        }
    };
}

// Global Validation Tracking Flags
uint32_t totalTestsRun = 0;
uint32_t totalTestsPassed = 0;

#define RUN_TEST(test_func) \
    do { \
        totalTestsRun++; \
        std::cout << "[RUNNING] " << #test_func << "..." << std::endl; \
        if (test_func()) { \
            totalTestsPassed++; \
            std::cout << "[SUCCESS] " << #test_func << " passed successfully." << std::endl; \
        } else { \
            std::cerr << "[FAILURE] " << #test_func << " FAILED CRITICALLY." << std::endl; \
        } \
    } while (0)

// Test 1: Validate that the QCD Phase Core locks entropy to exactly ln(2) when K >= 1.0
bool test_qcd_critical_crossover_limit() {
    MetamorphosisEngine::QcdProcessor qcdEngine(1.0); // Critical density at 1.0 GeV/fm^3
    
    // Test exact crossover boundary
    double entropyAtBoundary = qcdEngine.calculateEntropy(1.0);
    double errorDelta = std::abs(entropyAtBoundary - MetamorphosisEngine::LN_2);
    
    // Verify system remains stable and locked when driven past the threshold (K = 1.5)
    double entropyPastBoundary = qcdEngine.calculateEntropy(1.5);
    double hyperDelta = std::abs(entropyPastBoundary - MetamorphosisEngine::LN_2);

    return (errorDelta < 1e-15) && (hyperDelta < 1e-15);
}

// Test 2: Validate the linear entropy growth profile in sub-critical states (K < 1.0)
bool test_linear_entropy_accumulation_curve() {
    MetamorphosisEngine::QcdProcessor qcdEngine(1.0);
    
    // At baseline K approaching 0, statistical entropy must approach 0.0
    double baselineEntropy = qcdEngine.calculateEntropy(0.001);
    
    // At intermediate stress state (K = 0.5), check mathematical scaling
    double intermediateEntropy = qcdEngine.calculateEntropy(0.5);
    
    return (baselineEntropy >= 0.0) && (intermediateEntropy > baselineEntropy) && (intermediateEntropy < MetamorphosisEngine::LN_2);
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "--- STARTING SYSTEMIC MATRIX INTEGRITY TESTING PORTFOLIO ---" << std::endl;
    std::cout << "==========================================================" << std::endl;

    RUN_TEST(test_qcd_critical_crossover_limit);
    RUN_TEST(test_linear_entropy_accumulation_curve);

    std::cout << "==========================================================" << std::endl;
    std::cout << "TEST EXECUTION SUMMARY: " << totalTestsPassed << " / " << totalTestsRun << " PASSED." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (totalTestsPassed == totalTestsRun) ? 0 : 1;
}
