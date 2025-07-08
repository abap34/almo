#!/usr/bin/env python3
"""
Generate stub files for the almo Python extension module.
This script uses pybind11-stubgen to create .pyi files for better editor support.
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

def generate_stubs():
    """Generate stub files for the almo module."""
    print("Generating stub files for almo module...")
    
    # Get the directory where this script is located
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    
    # Create stubs directory if it doesn't exist
    stubs_dir = repo_root / "almo-stubs"
    stubs_dir.mkdir(exist_ok=True)
    
    try:
        # Import the module to make sure it's available
        import almo
        print(f"Found almo module at: {almo.__file__}")
        
        # Run pybind11-stubgen
        cmd = [
            sys.executable, "-m", "pybind11_stubgen", 
            "--output-dir", str(stubs_dir.parent),
            "almo"
        ]
        
        print(f"Running command: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
        print("Stub generation completed successfully!")
        print(f"Generated stubs in: {stubs_dir}")
        
        # List generated files
        if stubs_dir.exists():
            stub_files = list(stubs_dir.glob("*.pyi"))
            if stub_files:
                print("Generated stub files:")
                for stub_file in stub_files:
                    print(f"  - {stub_file}")
            else:
                print("No .pyi files found in stubs directory")
                
    except ImportError:
        print("Error: almo module not found. Please build the extension first.")
        print("Run: python setup.py build_ext --inplace")
        return False
    except subprocess.CalledProcessError as e:
        print(f"Error running pybind11-stubgen: {e}")
        print(f"stdout: {e.stdout}")
        print(f"stderr: {e.stderr}")
        return False
    except Exception as e:
        print(f"Unexpected error: {e}")
        return False
        
    return True

if __name__ == "__main__":
    success = generate_stubs()
    sys.exit(0 if success else 1)