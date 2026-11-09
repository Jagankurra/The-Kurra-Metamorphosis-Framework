// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Portable Multi-Architecture SIMD Processor (Runtime Dispatch: AVX-512 / AVX2 / Scalar)
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
//  Licensed under the Apache License, Version 2.0
// ==============================================================================

#ifndef AVX512_TENSOR_PROCESSOR_HPP
#define AVX512_TENSOR_PROCESSOR_HPP

#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace MultiVectorMetamorphosis {

    constexpr double SYSTEMIC_LN_2 = 0.6931471805599453;
    constexpr double MATRIX_SAFETY_FLOOR = 1e-12;

    enum class SimdBackend : uint8_t {
        Scalar = 0,
        AVX2   = 1,
        AVX512 = 2
    };

    struct alignas(64) Avx512TensorInput {
        std::array<double, 8> xx, yy, zz;
        std::array<double, 8> xy, yz, xz;
    };

    struct alignas(64) Avx512TelemetryOutput {
        std::array<double, 8> vonMises;
        std::array<double, 8> phaseFactorK;
        std::array<double, 8> entropyS;
        std::array<uint64_t, 8> alertState;
    };

    class Avx512Processor {
    private:
        double yieldCeiling;
        double warningLimit;
        SimdBackend activeBackend;

        static SimdBackend detectHardwareBackend() noexcept {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__x86_64__)
            __builtin_cpu_init();
            if (__builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512dq")) {
                return SimdBackend::AVX512;
            }
            if (__builtin_cpu_supports("avx2")) {
                return SimdBackend::AVX2;
            }
    #endif
#endif
            return SimdBackend::Scalar;
        }

        inline void computeEntropyAndState(Avx512TelemetryOutput& output) const noexcept {
            for (int i = 0; i < 8; ++i) {
                double k = output.phaseFactorK[i];
                if (k < 1.0) {
                    double p_o = 1.0 - (0.5 * k * k);
                    double p_c = 1.0 - p_o;
                    p_o = (p_o < MATRIX_SAFETY_FLOOR) ? MATRIX_SAFETY_FLOOR : p_o;
                    p_c = (p_c < MATRIX_SAFETY_FLOOR) ? MATRIX_SAFETY_FLOOR : p_c;

                    output.entropyS[i] = -((p_o * std::log(p_o)) + (p_c * std::log(p_c)));
                    output.alertState[i] = (k >= warningLimit) ? 1 : 0;
                } else {
                    output.entropyS[i] = SYSTEMIC_LN_2;
                    output.alertState[i] = 2;
                }
            }
        }

        void processScalarFallback(const Avx512TensorInput& input, Avx512TelemetryOutput& output) const noexcept {
            for (int i = 0; i < 8; ++i) {
                double d1 = input.xx[i] - input.yy[i];
                double d2 = input.yy[i] - input.zz[i];
                double d3 = input.zz[i] - input.xx[i];
                double normal = (d1 * d1) + (d2 * d2) + (d3 * d3);
                double shear  = 6.0 * ((input.xy[i] * input.xy[i]) + (input.yz[i] * input.yz[i]) + (input.xz[i] * input.xz[i]));

                output.vonMises[i]     = std::sqrt(0.5 * (normal + shear));
                output.phaseFactorK[i] = output.vonMises[i] / yieldCeiling;
            }
            computeEntropyAndState(output);
        }

#if defined(__AVX512F__) && defined(__AVX512DQ__)
        void processAvx512(const Avx512TensorInput& input, Avx512TelemetryOutput& output) const noexcept {
            __m512d v_xx = _mm512_load_pd(input.xx.data());
            __m512d v_yy = _mm512_load_pd(input.yy.data());
            __m512d v_zz = _mm512_load_pd(input.zz.data());
            __m512d v_xy = _mm512_load_pd(input.xy.data());
            __m512d v_yz = _mm512_load_pd(input.yz.data());
            __m512d v_xz = _mm512_load_pd(input.xz.data());

            __m512d diff1 = _mm512_sub_pd(v_xx, v_yy);
            __m512d diff2 = _mm512_sub_pd(v_yy, v_zz);
            __m512d diff3 = _mm512_sub_pd(v_zz, v_xx);

            __m512d normal_sum = _mm512_add_pd(
                _mm512_add_pd(_mm512_mul_pd(diff1, diff1), _mm512_mul_pd(diff2, diff2)),
                _mm512_mul_pd(diff3, diff3)
            );

            __m512d shear_sum = _mm512_add_pd(
                _mm512_add_pd(_mm512_mul_pd(v_xy, v_xy), _mm512_mul_pd(v_yz, v_yz)),
                _mm512_mul_pd(v_xz, v_xz)
            );

            __m512d total_inner = _mm512_add_pd(normal_sum, _mm512_mul_pd(shear_sum, _mm512_set1_pd(6.0)));
            __m512d von_mises   = _mm512_sqrt_pd(_mm512_mul_pd(total_inner, _mm512_set1_pd(0.5)));
            __m512d k_factors   = _mm512_div_pd(von_mises, _mm512_set1_pd(yieldCeiling));

            _mm512_store_pd(output.vonMises.data(), von_mises);
            _mm512_store_pd(output.phaseFactorK.data(), k_factors);
            computeEntropyAndState(output);
        }
#endif

    public:
        Avx512Processor(double ultimateYield, double warningThreshold = 0.85) noexcept
            : yieldCeiling(ultimateYield > 0.0 ? ultimateYield : 1.0),
              warningLimit(warningThreshold > 0.0 && warningThreshold < 1.0 ? warningThreshold : 0.85),
              activeBackend(detectHardwareBackend()) {}

        SimdBackend getActiveBackend() const noexcept { return activeBackend; }

        void processEightTensors(const Avx512TensorInput& input, Avx512TelemetryOutput& output) const noexcept {
#if defined(__AVX512F__) && defined(__AVX512DQ__)
            if (activeBackend == SimdBackend::AVX512) {
                processAvx512(input, output);
                return;
            }
#endif
            processScalarFallback(input, output);
        }
    };
}

#endif // AVX512_TENSOR_PROCESSOR_HPP
