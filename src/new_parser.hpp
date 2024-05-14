#pragma once

#include <cassert>
#include <regex>
#include <stack>
#include <string>
#include <iostream>
#include <functional>

#include "utils.hpp"

#include "new_ast.hpp"
#include "new_reader.hpp"
#include "new_syntax_all.hpp"

namespace almo::feature {


struct InlineParser;

struct MarkdownParser {
    Reader reader;
    MarkdownParser (std::vector<std::string> lines) : reader(lines) {}

    void process_block(std::string &str_inline, ASTNode &ast){
        if (str_inline != ""){
            str_inline += ' ';
        }
        str_inline += reader.whole_row();
        reader.move_next_line();
    }
    template<class HeadSyntax, class... TailSyntax>
    void process_block(std::string &str_inline, ASTNode &ast, HeadSyntax&& hsyn, TailSyntax&&... tsyn){
        if (std::invoke(hsyn, reader)){
            if (str_inline != ""){
                InlineParser::process(str_inline, ast);
                str_inline = "";
            }
            std::invoke(hsyn, reader, ast);
            return ;
        }
        return process_block(str_inline, ast, tsyn...);
    }

    void process(ASTNode &ast){
        std::string str_inline = "";
        while (!reader.is_whole_end()){
            process_block(
                str_inline, ast,
                NewLine_syntax{},
                Header_syntax{},
                EOF_syntax{}
            );
        }
        if (str_inline != ""){
            throw SyntaxError("Internal Error");
        }
    }
};

struct InlineParser {

    static void process_inline(const std::string &str, ASTNode &ast){
        RawText node(str);
        ast.add_child(std::make_shared<RawText>(node));
    }
    template<class HeadSyntax, class... TailSyntax>
    static void process_inline(const std::string &str, ASTNode &ast, HeadSyntax&& hsyn, TailSyntax&&... tsyn){
        if (std::invoke(hsyn, str)){
            std::invoke(hsyn, str, ast);
            return "";
        }
        return process_inline(str, ast, tsyn...);
    }

    static void process(const std::string &str, ASTNode &ast){
        process_inline(
            str, ast,
            InlineMath_syntax{}
        );
    }
};

// mdファイルのパスから
// メタデータ (std::map<std::string, std::string>) と
// 抽象構文木の根 (Block) のペアを返す。
std::pair<std::map<std::string, std::string>, Markdown> parse_md_file(std::string path) {
    std::vector<std::string> lines = read_file(path);

    std::vector<std::pair<std::string, std::string>> meta_data;
    int meta_data_end = 0;

    if (!lines.empty() && lines[0] == "---") {
        int index = 1;
        while (index < (int)lines.size() && lines[index] != "---") {
            std::string key = std::regex_replace(lines[index], std::regex("(.*):\\s(.*)"), "$1");
            std::string data = std::regex_replace(lines[index], std::regex("(.*):\\s(.*)"), "$2");
            meta_data.emplace_back(key, data);
            index++;
        }
        meta_data_end = index + 1;
    }

    // メタデータ以降の行を取り出し
    std::vector<std::string> md_lines;
    for (int i = meta_data_end; i < (int)lines.size(); i++) {
        md_lines.push_back(lines[i]);
    }

    std::string md_str = join(md_lines, "\n");

    // コメントを削除
    auto remove_comment = [](std::string text) {
        return _remove_comment(text);
        };


    std::vector<std::function<std::string(std::string)>> hooks = {
        remove_comment,
    };

    for (auto hook : hooks) {
        md_str = hook(md_str);
    }

    md_lines = split(md_str, "\n");

    // パース
    Markdown ast;
    MarkdownParser parser(md_lines);
    parser.process(ast);

    // meta_data を std::map に変換する
    std::map<std::string, std::string> meta_data_map;

    // デフォルト値を設定
    meta_data_map["title"] = "";
    meta_data_map["date"] = "";
    meta_data_map["author"] = "";
    meta_data_map["twitter_id"] = "";
    meta_data_map["github_id"] = "";
    meta_data_map["mail"] = "";
    meta_data_map["ogp_url"] = "https://www.abap34.com/almo_logo.jpg";
    meta_data_map["tag"] = "";
    meta_data_map["url"] = "";
    meta_data_map["site_name"] = "";
    meta_data_map["twitter_site"] = "";

    for (auto [key, data] : meta_data) {
        meta_data_map[key] = data;
    }

    return { meta_data_map,  ast };
}

} // namespace almo