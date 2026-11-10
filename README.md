# The Kurra Matrix of Systemic Metamorphosis

[![CI Pipeline](https://github.com)](https://github.com)
[![DOI](https://zenodo.org)](https://doi.org)
[![License](https://shields.io)](https://opensource.org)

Reference engineering implementation for **The Kurra Matrix of Systemic Metamorphosis**, a dialectical heuristic and unified telemetry normalisation architecture designed to track and anticipate non-linear phase transitions across diverse operational domains.

Developed by **Jagan Mohan Rao Kurra**, this framework draws on classical dialectical principles—specifically the unity of opposing tendencies and the transformation of quantitative accumulation into qualitative change—to translate disparate domain stresses into a single, dimensionless utilisation ratio: **The Kurra Phase Factor (\(K_K\))**. By pairing \(K_K\) with a bounded two-state Shannon entropy surrogate scaled to \(\ln(2) \approx 0.6931\), the engine provides systems architects with a uniform alerting scale for real-time anomaly detection.

---

## Conceptual Architecture

1. **Law I — Polar Entanglement (Unity of Opposites):** Stable systems operate as dynamic equilibria holding an inward stabilising tendency (**Centripetal Vector**) and an outward disruptive tendency (**Centrifugal Vector**) in tension.
2. **Law II — Quantitative Threshold:** Systems absorb incremental load linearly up to a finite capacity ceiling, tracked via the normalised utilisation ratio \(K_K(t) = Q_{\text{accumulated}}(t) / C_{\text{boundary}}\).
3. **Law III — Equivalent Metamorphosis (Quantity into Quality):** Crossing \(K_K \ge 1.0\) triggers a discontinuous qualitative shift (**Polar Inversion**), where the system reconfigures into a new operational state.
4. **Law IV — Strict Conservation:** Systemic transitions reorganise existing mass, energy, and informational inputs (\(\sum \text{Inputs} = \sum \text{Outputs}\)) rather than annihilating them.

---

## Repository Structure

* **`opt/edge/matrix/bin/MultiDimensionalTensorCore.hpp`**: Deterministic \(\mathcal{O}(1)\) C++ core mapping \(3 \times 3\) Cauchy symmetric stress tensors into Von Mises invariants, \(K_K\) ratios, and bounded entropy surrogates.
* **`opt/edge/matrix/bin/KurraPyBind.cpp`**: Zero-copy `pybind11` extension allowing Python and NumPy pipelines to pass contiguous `float64` memory buffers directly to C++ with the GIL released.
* **`opt/edge/matrix/bin/Avx512TensorProcessor.hpp`**: Vectorised SIMD processing module with runtime hardware capability detection (`AVX-512` / `AVX2` / `Scalar`).
* **`opt/edge/matrix/bin/KurraPrometheusExporter.hpp`**: Allocation-free, non-blocking metrics endpoint exposing live \(K_K\) and entropy gauges on port `9100/metrics`.
* **`domain_registry.json`**: Heuristic parameter mappings across 38 interdisciplinary domains.

---

## Zero-Copy Python Integration

To avoid string-parsing overhead in high-frequency telemetry loops, compile the native C++ extension and pass contiguous 2D NumPy arrays directly into the engine:

```bash
# Install build dependencies and compile the native extension
pip install pybind11 numpy setuptools
pip install .
```

### Usage Example

```python
import numpy as np
from kurra_engine import ZeroCopyTensorEngine

# Initialise engine for Structural Steel (Yield ceiling = 550.0 MPa, Warning at K = 0.85)
engine = ZeroCopyTensorEngine(ultimate_yield=550.0, warning_limit=0.85)

# Contiguous float64 buffer of shape (N, 6): [xx, yy, zz, xy, yz, xz]
stress_batch = np.array([
    [120.0,  90.0,  45.0, 12.0,  5.0,  8.0],  # Nominal state
    [340.5, 280.2, 190.4, 45.1, 30.4, 22.2],  # Elevated strain
    [900.0, 100.0, 100.0, 250.0, 150.0, 100.0],  # Critical threshold breach (K >= 1.0)
], dtype=np.float64)

# Process batch with zero input copies and Python GIL released
# Returns shape (N, 4): [von_mises_mpa, phase_factor_k, entropy_s, state_code]
results = engine.process_batch(stress_batch)

for idx, row in enumerate(results):
    print(f"Frame {idx} | Von Mises: {row:.2f} MPa | K_K: {row:.4f} | Entropy S: {row:.4f} | State: {int(row)}")
```

---

## Citation & Licensing

If you reference this telemetry architecture or heuristic mapping in academic or engineering work, please cite the Version 2 Zenodo publication:

> Kurra, J. M. R. (2026). *The Kurra Matrix of Systemic Metamorphosis: A Dialectical Heuristic and Unified Telemetry Normalisation Architecture for Tracking Non-Linear Phase Transitions* (Version v2). Zenodo. https://doi.org

Released under the **Apache License, Version 2.0**. Copyright (c) 2026 Jagan Mohan Rao Kurra.
