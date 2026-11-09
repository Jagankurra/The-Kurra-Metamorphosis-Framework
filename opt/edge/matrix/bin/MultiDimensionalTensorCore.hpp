// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Multi-Dimensional Tensor Stress Core Header
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
// ==============================================================================

#ifndef MULTI_DIMENSIONAL_TENSOR_CORE_HPP
#define MULTI_DIMENSIONAL_TENSOR_CORE_HPP

#include <cmath>
#include <cstdint>
#include <array>

namespace TensorMetamorphosisEngine {

    constexpr double SYSTEMIC_LN_2 = 0.6931471805599453;
    constexpr double MATRIX_FLOOR  = 1e-11;

    enum class TensorSafetyState : uint8_t {
        ElasticStable      = 0,
        CriticalIncubation = 1,
        PlasticFracture    = 2
    };

    struct TensorTelemetryFrame {
        double vonMisesStress;
        double phaseFactorK;
        double structuralEntropyS;
        double tensorOrderPo;
        double tensorChaosPc;
        TensorSafetyState safetyState;
    };

    struct SymmetricStressTensor {
        double xx, yy, zz; 
        double xy, yz, xz; 
    };

    class MultiaxialProcessor {
    private:
        double ultimateMaterialYieldCeiling;
        double warningThresholdK;

        static inline double enforceCoherenceBounds(double p) noexcept {
            if (p < MATRIX_FLOOR) return MATRIX_FLOOR;
            if (p > 1.0) return 1.0;
            return p;
        }

    public:
        constexpr MultiaxialProcessor(double ultimateYield, double warningLimit = 0.85) noexcept
            : ultimateMaterialYieldCeiling(ultimateYield > 0.0 ? ultimateYield : 1.0),
              warningThresholdK(warningLimit > 0.0 && warningLimit < 1.0 ? warningLimit : 0.85) {}

        ~MultiaxialProcessor() = default;

        TensorTelemetryFrame processTensorField(const SymmetricStressTensor& tensor) noexcept {
            TensorTelemetryFrame frame{};

            double term1 = (tensor.xx - tensor.yy) * (tensor.xx - tensor.yy);
            double term2 = (tensor.yy - tensor.zz) * (tensor.yy - tensor.zz);
            double term3 = (tensor.zz - tensor.xx) * (tensor.zz - tensor.xx);
            double shearTerms = 6.0 * (tensor.xy * tensor.xy + tensor.yz * tensor.yz + tensor.xz * tensor.xz);
            
            frame.vonMisesStress = std::sqrt(0.5 * (term1 + term2 + term3 + shearTerms));
            frame.phaseFactorK = frame.vonMisesStress / ultimateMaterialYieldCeiling;

            if (frame.phaseFactorK < 1.0) {
                double k_sq = frame.phaseFactorK * frame.phaseFactorK;
                frame.tensorOrderPo = 1.0 - (0.5 * k_sq);
                frame.tensorChaosPc = 1.0 - frame.tensorOrderPo;

                double p_o = enforceCoherenceBounds(frame.tensorOrderPo);
                double p_c = enforceCoherenceBounds(frame.tensorChaosPc);

                frame.structuralEntropyS = -((p_o * std::log(p_o)) + (p_c * std::log(p_c)));

                if (frame.phaseFactorK >= warningThresholdK) {
                    frame.safetyState = TensorSafetyState::CriticalIncubation;
                } else {
                    frame.safetyState = TensorSafetyState::ElasticStable;
                }
            } else {
                frame.tensorOrderPo = 0.50;
                frame.tensorChaosPc = 0.50;
                frame.structuralEntropyS = SYSTEMIC_LN_2;
                frame.safetyState = TensorSafetyState::PlasticFracture;
            }

            return frame;
        }
    };
}

#endif // MULTI_DIMENSIONAL_TENSOR_CORE_HPP
