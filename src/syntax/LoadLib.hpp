#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

// ライブラリの読み込みをする独自記法.
struct LoadLib : public ASTNode {
   private:
    // 読み込むライブラリの名前のリスト
    std::vector<std::string> libs;

   public:
    LoadLib(std::vector<std::string> libs) : libs(libs) { set_uuid(); }

    // use_libs に追加しておくと JS側で読み込み処理を行う。
    std::string to_html() const override {
        std::string output = "";
        for (std::string lib : libs) {
            output += "<script> use_libs.push(\"" + lib + "\"); </script>";
        }
        return output;
    };

    std::map<std::string, std::string> get_properties() const override {
        return {{"libs", join(libs)}};
    }
    std::string get_classname() const override { return "LoadLib"; }
};

struct LoadLibSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (rtrim(read.get_row()) == ":::loadlib") return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        std::vector<std::string> libs;

        // skip :::loadlib
        read.move_next_line();

        if (read.is_eof()) {
            throw SyntaxError("Empty Library");
        }

        while (!read.is_eof()) {
            if (rtrim(read.get_row()) == ":::") {
                read.move_next_line();
                break;
            }
            libs.push_back(rtrim(read.get_row()));
            read.move_next_line();
        }

        LoadLib node(libs);
        ast.pushback_child(std::make_shared<LoadLib>(node));
    }
};

}  // namespace almo