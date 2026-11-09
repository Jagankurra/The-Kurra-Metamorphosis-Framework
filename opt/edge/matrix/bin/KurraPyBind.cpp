// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Zero-Copy NumPy / pybind11 High-Throughput Binding Layer
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
//  Licensed under the Apache License, Version 2.0
// ==============================================================================

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <cmath>
#include <stdexcept>
#include "MultiDimensionalTensorCore.hpp"

namespace py = pybind11;

class ZeroCopyTensorEngine {
private:
    TensorMetamorphosisEngine::MultiaxialProcessor processor;

public:
    ZeroCopyTensorEngine(double ultimateYield, double warningLimit = 0.85)
        : processor(ultimateYield, warningLimit) {}

    /**
     * Processes a contiguous NumPy array of shape (N, 6) [xx, yy, zz, xy, yz, xz]
     * with zero input memory copies. Returns a NumPy array of shape (N, 4)
     * [von_mises_stress, phase_factor_k, entropy_s, safety_state_code].
     */
    py::array_t<double> process_batch(py::array_t<double, py::array::c_style | py::array::forcecast> input_array) {
        py::buffer_info buf_in = input_array.request();

        if (buf_in.ndim != 2 || buf_in.shape[1] != 6) {
            throw std::runtime_error("Input tensor batch must be a 2D NumPy array of shape (N, 6).");
        }

        const ssize_t num_frames = buf_in.shape[0];
        const double* ptr_in = static_cast<const double*>(buf_in.ptr);

        // Allocate single contiguous output buffer of shape (N, 4)
        py::array_t<double> output_array({num_frames, static_cast<ssize_t>(4)});
        py::buffer_info buf_out = output_array.request();
        double* ptr_out = static_cast<double*>(buf_out.ptr);

        // Release the Python Global Interpreter Lock (GIL) during C++ computation
        {
            py::gil_scoped_release release;

            for (ssize_t i = 0; i < num_frames; ++i) {
                const ssize_t in_offset = i * 6;
                TensorMetamorphosisEngine::SymmetricStressTensor tensor{
                    ptr_in[in_offset + 0], // xx
                    ptr_in[in_offset + 1], // yy
                    ptr_in[in_offset + 2], // zz
                    ptr_in[in_offset + 3], // xy
                    ptr_in[in_offset + 4], // yz
                    ptr_in[in_offset + 5]  // xz
                };

                auto frame = processor.processTensorField(tensor);

                const ssize_t out_offset = i * 4;
                ptr_out[out_offset + 0] = frame.vonMisesStress;
                ptr_out[out_offset + 1] = frame.phaseFactorK;
                ptr_out[out_offset + 2] = frame.structuralEntropyS;
                ptr_out[out_offset + 3] = static_cast<double>(frame.safetyState);
            }
        }

        return output_array;
    }
};

PYBIND11_MODULE(kurra_engine, m) {
    m.doc() = "Zero-copy C++ telemetry normalisation engine for The Kurra Matrix";

    py::class_<ZeroCopyTensorEngine>(m, "ZeroCopyTensorEngine")
        .def(py::init<double, double>(),
             py::arg("ultimate_yield"),
             py::arg("warning_limit") = 0.85)
        .def("process_batch", &ZeroCopyTensorEngine::process_batch,
             py::arg("tensor_batch"),
             "Process a contiguous (N, 6) float64 NumPy array without string parsing.");
}
