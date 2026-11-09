from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        "kurra_engine",
        sources=["opt/edge/matrix/bin/KurraPyBind.cpp"],
        include_dirs=[
            pybind11.get_include(),
            "opt/edge/matrix/bin"
        ],
        language="c++",
        extra_compile_args=["-O3", "-std=c++17"],
    ),
]

setup(
    name="kurra_engine",
    version="2.0.0",
    author="Jagan Mohan Rao Kurra",
    description="Zero-copy C++ telemetry normalisation engine for The Kurra Matrix",
    ext_modules=ext_modules,
    install_requires=["numpy", "pybind11"],
)
