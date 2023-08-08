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
        CodeRunner, // プログラムの実行環境を作る独自記法
        CodeBlock, // コードブロック
        MathBlock, // 数式ブロック
        InlineMath, // インラインの数式ブロック
        InlineOverline, // インラインの打消し
        InlineStrong, // インラインの強調
        InlineItalic, // イタリック
        PlainText, // 生のテキスト
        InfoBlock, // infoブロック
        NewLine, // 改行
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

        // typeがCodeRunnerのときのみ使用されます。CodeRunnerの情報を持ちます。
        std::vector<std::pair<std::string, std::string>> code_runner;
    };
}