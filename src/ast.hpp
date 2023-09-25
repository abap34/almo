#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace almo {

    // 構文木の属性を表すenum
    enum Type {
        H1, // <h1>
        H2, // <h2>
        H3, // <h3>
        H4, // <h4>
        H5, // <h5>
        H6, // <h6>
        Block, // ブロック
        Judge, // プログラムの実行環境を作る独自記法
        ExecutableCodeBlock, // 実行可能なコードブロック
        LoadLib, // ライブラリの読み込み
        CodeBlock, // コードブロック
        MathBlock, // 数式ブロック
        InlineMath, // インラインの数式ブロック
        InlineOverline, // インラインの打消し
        InlineStrong, // インラインの強調
        InlineItalic, // イタリック
        InlineUrl, // インラインのurl
        InlineImage, // インラインの画像
        InlineCodeBlock, // インラインのコードブロック
        Url, // url
        PlainText, // 生のテキスト
        InfoBlock, // infoブロック
        NewLine, // 改行
        ListBlock, // 箇条書きのブロック
        EnumerateBlock, // 番号付き箇条書きのブロック
        Item, // 箇条書きの要素
        Table, // テーブル
    };

    // 構文木のノードを表す構造体
    // 各ノードにはTypeが付いており、それによってそのノードの役割を判定することができる。
    struct AST {
        using node_ptr = std::shared_ptr<AST>;
        AST() = default;
        AST(Type type, std::string content = "") : type(type), content(content) { }
        Type type;
        std::string content; // typeがPlainTextのときのみ使用されます。PlainTextのテキストの中身が格納されます。
        std::vector<node_ptr> childs; // 構文木の子ノードを保持します。

        // typeがJudgeのときのみ使用されます。judgeの情報を持ちます。
        std::vector<std::pair<std::string, std::string>> judge;

        // typeがExecutableCodeBlockのときのみ使用されます。ExecutableCodeBlockの情報を持ちます。
        std::string code;

        // typeがLoadLibのときのみ使用されます。読み込むライブラリの名前を持ちます。
        std::vector<std::string> libs;

        // typeがTableのときのみ使用されます。Tableの情報を持ちます。
        std::vector<std::pair<std::string, std::string>> table;
        std::vector<int> col_format;
        std::vector<std::string> col_names;

        // typeがCodeBlockのときのみ使用されます。CodeBlockの情報(言語) を持ちます。
        std::string language;
    };
}