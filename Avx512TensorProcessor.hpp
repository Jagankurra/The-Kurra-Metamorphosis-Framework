// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  512-bit AVX-512 SIMD Vectorized Processing Core
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
// ==============================================================================

#ifndef AVX512_TENSOR_PROCESSOR_HPP
#define AVX512_TENSOR_PROCESSOR_HPP

#include <immintrin.h>
#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>

namespace MultiVectorMetamorphosis {

    constexpr double SYSTEMIC_LN_2 = 0.6931471805599453;
    constexpr double MATRIX_SAFETY_FLOOR = 1e-12;

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
        alignas(64) std::array<double, 8> yieldCeilingPack;
        alignas(64) std::array<double, 8> warningLimitPack;

    public:
        Avx512Processor(double ultimateYield, double warningLimit = 0.85) noexcept {
            std::fill(yieldCeilingPack.begin(), yieldCeilingPack.end(), ultimateYield);
            std::fill(warningLimitPack.begin(), warningLimitPack.end(), warningLimit);
        }

        ~Avx512Processor() = default;

        void processEightTensors(const Avx512TensorInput& input, Avx512TelemetryOutput& output) noexcept {
            __m512d v_xx = _mm512_load_pd(input.xx.data());
            __m512d v_yy = _mm512_load_pd(input.yy.data());
            __m512d v_zz = _mm512_load_pd(input.zz.data());
            __m512d v_xy = _mm512_load_pd(input.xy.data());
            __m512d v_yz = _mm512_load_pd(input.yz.data());
            __m512d v_xz = _mm512_load_pd(input.xz.data());

            __m512d diff1 = _mm512_sub_pd(v_xx, v_yy);
            __m512d term1 = _mm512_mul_pd(diff1, diff1);

            __m512d diff2 = _mm512_sub_pd(v_yy, v_zz);
            __m512d term2 = _mm512_mul_pd(diff2, diff2);

            __m512d diff3 = _mm512_sub_pd(v_zz, v_xx);
            __m512d term3 = _mm512_mul_pd(diff3, diff3);

            __m512d xy_sq = _mm512_mul_pd(v_xy, v_xy);
            __m512d yz_sq = _mm512_mul_pd(v_yz, v_yz);
            __m512d xz_sq = _mm512_mul_pd(v_xz, v_xz);
            __m512d shear_sum = _mm512_add_pd(_mm512_add_pd(xy_sq, yz_sq), xz_sq);
            __m512d v_six = _mm512_set1_pd(6.0);
            __m512d shear_term = _mm512_mul_pd(shear_sum, v_six);

            __m512d normal_sum = _mm512_add_pd(_mm512_add_pd(term1, term2), term3);
            __m512d total_inner = _mm512_add_pd(normal_sum, shear_term);
            __m512d v_half = _mm512_set1_pd(0.5);
            __m512d von_mises = _mm512_sqrt_pd(_mm512_mul_pd(total_inner, v_half));

            __m512d ceiling = _mm512_load_pd(yieldCeilingPack.data());
            __m512d k_factors = _mm512_div_pd(von_mises, ceiling);

            _mm512_store_pd(output.vonMises.data(), von_mises);
            _mm512_store_pd(output.phaseFactorK.data(), k_factors);

            for (int i = 0; i < 8; ++i) {
                if (output.phaseFactorK[i] < 1.0) {
                    double p_o = 1.0 - (0.5 * output.phaseFactorK[i] * output.phaseFactorK[i]);
                    double p_c = 1.0 - p_o;
                    p_o = (p_o < MATRIX_SAFETY_FLOOR) ? MATRIX_SAFETY_FLOOR : p_o;
                    p_c = (p_c < MATRIX_SAFETY_FLOOR) ? MATRIX_SAFETY_FLOOR : p_c;
                    
                    output.entropyS[i] = -((p_o * std::log(p_o)) + (p_c * std::log(p_c)));
                    output.alertState[i] = (output.phaseFactorK[i] >= warningLimitPack[i]) ? 1 : 0;
                } else {
                    output.entropyS[i] = SYSTEMIC_LN_2;
                    output.alertState[i] = 2; 
                }
            }
        }
    };
}

#endif // AVX512_TENSOR_PROCESSOR_HPP
