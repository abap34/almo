# Stub File Generation for ALMO

This document explains how to use the stub file generation feature for better Python IDE support.

## Overview

The ALMO Python extension now includes Python stub files (`.pyi`) that provide type hints for all exported functions and classes. This enables:

- **Autocompletion** in IDEs like VS Code, PyCharm, etc.
- **Type checking** with mypy and other static analyzers  
- **Better error detection** during development
- **Enhanced documentation** and function signatures in editors

## What's Included

The stub files provide type hints for:

### Functions
- `parse(md_content: str) -> Markdown` - Parse markdown to AST
- `move_footnote_to_end()` - Move footnote definitions to end
- `required_pyodide()` - Check if pyodide is required

### Classes
- `ASTNode` - Base class for all AST nodes
- `Markdown` - Main markdown document node
- All syntax node classes: `CodeBlock`, `Header`, `Table`, etc.

### Attributes  
- `__version__: str` - Module version string

## Automatic Generation

Stub files are automatically generated during the build process when you install the package:

```bash
pip install almo
```

## Manual Generation

To regenerate stub files during development:

```bash
# Install build dependencies
pip install pybind11-stubgen

# Build extension and generate stubs
./scripts/build_with_stubs.sh

# Or run manually
python setup.py build_ext --inplace
python scripts/generate_stubs.py
```

## Usage Example

```python
import almo

# IDE provides autocompletion and type hints
markdown = "# Hello\n\nThis is **bold** text."
ast = almo.parse(markdown)  # Returns: Markdown

# Method calls have proper signatures
html = ast.to_html()        # Returns: str
json_data = ast.to_json()   # Returns: str

# Class constructors have type hints
header = almo.Header(1)     # level: int
code = almo.CodeBlock("python", "print('hi')")  # lang: str, code: str

# Properties are properly typed
children = ast.childs       # List[ASTNode]
```

## IDE Configuration

Most modern Python IDEs automatically detect and use `.pyi` files. No additional configuration is needed for:

- **VS Code** with Python extension
- **PyCharm**
- **Vim/Neovim** with LSP
- **Emacs** with python-mode

## Type Checking

Use mypy for static type checking:

```bash
pip install mypy
mypy your_script.py
```

## Distribution

The stub files are automatically included when you install the almo package via pip. They are distributed as part of the source distribution and wheel packages.