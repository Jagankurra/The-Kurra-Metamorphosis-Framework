// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Low-Latency C-Compatible Bridge Interface Layer
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
// ==============================================================================

#include "MultiDimensionalTensorCore.hpp"

extern "C" {

    struct C_SymmetricTensor {
        double xx, yy, zz;
        double xy, yz, xz;
    };

    struct C_TelemetryFrame {
        double vonMisesStress;
        double phaseFactorK;
        double structuralEntropyS;
        double tensorOrderPo;
        double tensorChaosPc;
        uint8_t safetyState; 
    };

    void* create_processor(double ultimateYield, double warningLimit) {
        return reinterpret_cast<void*>(new TensorMetamorphosisEngine::MultiaxialProcessor(ultimateYield, warningLimit));
    }

    void destroy_processor(void* proc_ptr) {
        delete reinterpret_cast<TensorMetamorphosisEngine::MultiaxialProcessor*>(proc_ptr);
    }

    C_TelemetryFrame process_tensor_fields(void* proc_ptr, C_SymmetricTensor raw_fields) {
        auto* processor = reinterpret_cast<TensorMetamorphosisEngine::MultiaxialProcessor*>(proc_ptr);
        
        TensorMetamorphosisEngine::SymmetricStressTensor internal_tensor{
            raw_fields.xx, raw_fields.yy, raw_fields.zz,
            raw_fields.xy, raw_fields.yz, raw_fields.xz
        };

        auto cpp_frame = processor->processTensorField(internal_tensor);

        C_TelemetryFrame outbound_frame{};
        outbound_frame.vonMisesStress   = cpp_frame.vonMisesStress;
        outbound_frame.phaseFactorK     = cpp_frame.phaseFactorK;
        outbound_frame.structuralEntropyS = cpp_frame.structuralEntropyS;
        outbound_frame.tensorOrderPo    = cpp_frame.tensorOrderPo;
        outbound_frame.tensorChaosPc   = cpp_frame.tensorChaosPc;
        outbound_frame.safetyState      = static_cast<uint8_t>(cpp_frame.safetyState);

        return outbound_frame;
    }
}
