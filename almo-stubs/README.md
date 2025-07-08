# Python Stub Files for ALMO

This directory contains Python stub files (`.pyi` files) for the ALMO Python extension module. These stub files provide type hints and enable better editor support, autocompletion, and static type checking.

## What are stub files?

Stub files contain type information for Python modules without the actual implementation. They allow IDEs and type checkers like mypy to understand the types and methods available in extension modules built with C++ (like pybind11 modules).

## How are they generated?

The stub files are generated using `pybind11-stubgen`, which analyzes the built extension module and creates corresponding `.pyi` files.

## How to regenerate stub files

If you modify the C++ code and want to regenerate the stub files:

1. Make sure you have `pybind11-stubgen` installed:
   ```bash
   pip install pybind11-stubgen
   ```

2. Build the extension and generate stubs:
   ```bash
   ./scripts/build_with_stubs.sh
   ```

   Or manually:
   ```bash
   python setup.py build_ext --inplace
   python scripts/generate_stubs.py
   ```

## Using the stub files

When you install the almo package, the stub files are automatically included. Most modern Python IDEs will automatically detect and use these stub files to provide:

- Code completion
- Type hints
- Function signatures
- Documentation strings
- Static type checking

## IDE Support

These stub files work with:
- PyCharm
- VS Code with Python extension
- Vim/Neovim with appropriate plugins
- Emacs with Python mode
- Any editor that supports Language Server Protocol (LSP)

## Type Checking

You can use mypy or other type checkers with these stub files:

```bash
mypy your_code.py
```