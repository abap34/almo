import os
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

os.system("git describe --tags --abbrev=0 > version.txt")
os.system("git show --format=\"%h\" -s > commithash.txt")


with open("version.txt") as f:
    __version__ = f.read().strip()

ext_modules = [
    Pybind11Extension(
        "almo",
        ["src/pyalmo.cpp"],
        cxx_std=20,
        define_macros=[("VERSION_INFO", "\"" + __version__ + "\"")],
    ),
]

long_description = open("README.md").read()

setup(
    name="almo",
    author="abap34, ebi-fly13, noya2",
    url="https://github.com/abap34/ALMO",
    long_description_content_type="text/markdown",
    long_description=long_description,
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    version=__version__,
)
