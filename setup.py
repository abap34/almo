import os
import sys
import subprocess
from pathlib import Path
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
from setuptools.command.build_ext import build_ext as _build_ext

os.system("bash scripts/setup.sh")
with open("version.txt") as f:
    __version__ = f.read().strip()

ext_modules = [
    Pybind11Extension(
        "almo",
        ["build/pyalmo.cpp"],
        cxx_std=20,
        define_macros=[("VERSION_INFO", "\"" + __version__ + "\"")],
    ),
]

class CustomBuildExt(_build_ext):
    """Custom build_ext command that generates stub files after building."""
    
    def run(self):
        # First, run the normal build
        super().run()
        
        # Then generate stub files
        self.generate_stubs()
    
    def generate_stubs(self):
        """Generate stub files using pybind11-stubgen."""
        print("Generating stub files...")
        
        try:
            # Import pybind11_stubgen to check if it's available
            import pybind11_stubgen
            
            # Get the build directory where the extension was built
            build_lib = self.build_lib
            if build_lib:
                # Add build directory to Python path temporarily
                sys.path.insert(0, build_lib)
            
            # Create stubs directory 
            stubs_dir = Path("almo-stubs")
            stubs_dir.mkdir(exist_ok=True)
            
            # Run pybind11-stubgen
            cmd = [
                sys.executable, "-m", "pybind11_stubgen",
                "--output-dir", ".",
                "almo"
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("Successfully generated stub files!")
                # List generated files
                stub_files = list(stubs_dir.glob("*.pyi"))
                for stub_file in stub_files:
                    print(f"  Generated: {stub_file}")
            else:
                print(f"Warning: Stub generation failed: {result.stderr}")
                
        except ImportError:
            print("Warning: pybind11-stubgen not available, skipping stub generation")
        except Exception as e:
            print(f"Warning: Stub generation failed: {e}")
        finally:
            # Remove build directory from path if we added it
            if build_lib and build_lib in sys.path:
                sys.path.remove(build_lib)

long_description = open("README.md").read()

setup(
    name="almo",
    author="abap34, ebi-fly13, noya2",
    url="https://github.com/abap34/ALMO",
    long_description_content_type="text/markdown",
    long_description=long_description,
    ext_modules=ext_modules,
    cmdclass={"build_ext": CustomBuildExt},
    zip_safe=False,
    version=__version__,
    # Include stub files in the distribution
    package_data={
        "": ["almo-stubs/*.pyi"],
    },
    include_package_data=True,
)