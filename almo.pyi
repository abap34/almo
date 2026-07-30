"""ALMO - 拡張Markdownパーサ / 静的サイトジェネレータ"""

from typing import Optional

__doc__: str
__version__: str

def render(
    input_file: str,
    output_file: Optional[str] = ...,
    theme: str = ...,
    css_file: Optional[str] = ...,
    editor_theme: str = ...,
    debug: bool = ...,
    generate_ast: bool = ...,
    help: bool = ...,
) -> None: ...
