import os
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

os.system("bash scripts/setup.sh")
__version__ = "\"0.0.1\""

ext_modules = [
    Pybind11Extension(
        "almo",
        ["build/pyalmo.cpp"],
        cxx_std=23,
        define_macros=[("VERSION_INFO", __version__)],
    ),
]

setup(
    name="almo",
    author=["abap34", "ebi-fly13", "noya2"],
    url="https://github.com/abap34/ALMO",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)