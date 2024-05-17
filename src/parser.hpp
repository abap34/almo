#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <map>
#include <stack>
#include <functional>
#include "interfaces/parser.hpp"
#include "ast.hpp"
#include "syntax_all.hpp"

namespace almo {

template<class Syntax>
void InlineParser::process_inline(const std::string &str, ASTNode &ast, Syntax&& syn, int pos){
    // std::cerr << "str : '" << str << "'" << std::endl;
    std::invoke(syn, str, ast);
}

// syn has matched at position pos
template<class Syntax, class HeadSyntax, class... TailSyntax>
void InlineParser::process_inline(const std::string &str, ASTNode &ast, Syntax&& syn, int pos, HeadSyntax&& hsyn, TailSyntax&&... tsyn){
    int newpos = std::invoke(hsyn, str);
    if (newpos < pos){
        // syntax with nearer match detected
        // std::cerr << "newpos " << pos << " -> " << newpos << std::endl;
        // std::cerr << typeid(hsyn).name() << std::endl;
        return process_inline(str, ast, hsyn, newpos, tsyn...);
    }
    else {
        return process_inline(str, ast, syn, pos, tsyn...);
    }
}

void InlineParser::process(const std::string &str, ASTNode &ast){
    if (str == "") return ;
    process_inline(
        str, ast,
        RawText_syntax{}, std::numeric_limits<int>::max(),
        InlineMath_syntax{},
        InlineOverline_syntax{},
        InlineStrong_syntax{},
        InlineItalic_syntax{}
    );
}

MarkdownParser::MarkdownParser(const std::vector<std::string> &lines, const std::map<std::string, std::string> &meta_data) : reader(lines, meta_data) {}

void MarkdownParser::process_block(std::string &str_inline, ASTNode &ast){
    if (str_inline != ""){
        str_inline += ' ';
    }
    str_inline += reader.get_row();
    reader.move_next_line();
}

template<class HeadSyntax, class... TailSyntax>
void MarkdownParser::process_block(std::string &str_inline, ASTNode &ast, HeadSyntax&& hsyn, TailSyntax&&... tsyn){
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

void MarkdownParser::process(ASTNode &ast){
    std::string str_inline = "";
    while (!reader.eof_read_flg){
        process_block(
            str_inline, ast,
            EOF_syntax{},
            NewLine_syntax{},
            Header_syntax{},
            CodeBlock_syntax{}
        );
    }
    if (str_inline != ""){
        std::cerr << "Not empty inline string : '" << str_inline << "'" << std::endl;
        throw SyntaxError("Internal Error");
    }
}


// md ファイルの中身 (frot YAML を含まない)
// を受け取って、前処理 ([コメントの削除, ])　を行う
std::vector<std::string> preprocess(std::vector<std::string> content) {
    std::string content_join = join(content, "\n");

    // コメントを削除
    auto remove_comment = [](std::string text) {
        return _remove_comment(text);
        };


    std::vector<std::function<std::string(std::string)>> hooks = {
        remove_comment,
    };

    for (auto hook : hooks) {
        content_join = hook(content_join);
    }

    content = split(content_join, "\n");

    return content;
}

// md ファイルの中身 (front YAML) 
// を受け取って、 front YAML をパースした結果と残りの md ファイルの開始位置を返す
// TODO: きちんとした YAML パーサを使うようにする。
std::pair<std::vector<std::pair<std::string, std::string>>, int> parse_front(std::vector<std::string> content) {
    std::vector<std::pair<std::string, std::string>> front_yaml;
    int front_yaml_end = 0;

    if (!content.empty() && content[0] == "---") {
        int index = 1;
        while (index < (int)content.size() && content[index] != "---") {
            std::string key = std::regex_replace(content[index], std::regex("(.*):\\s(.*)"), "$1");
            std::string data = std::regex_replace(content[index], std::regex("(.*):\\s(.*)"), "$2");
            front_yaml.emplace_back(key, data);
            index++;
        }
        front_yaml_end = index + 1;
    }

    return { front_yaml, front_yaml_end };
}

// md ファイルの内容から
// メタデータと md の本文の組を返す
std::pair<std::map<std::string, std::string>, std::vector<std::string>> split_markdown(std::vector<std::string> content){
    auto [meta_data, meta_data_end] = parse_front(content);

    // メタデータ以降の行を取り出し
    std::vector<std::string> md_lines(content.begin() + meta_data_end, content.end());

    // 前処理
    md_lines = preprocess(md_lines);

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

    return {meta_data_map, md_lines};
}

// md ファイルのパスから
// メタデータと md の本文の組を返す
std::pair<std::map<std::string, std::string>, std::vector<std::string>> split_markdown_from_path(std::string path){
    return split_markdown(read_file(path));
}

Markdown parse(std::map<std::string, std::string> meta_data, std::vector<std::string> true_markdown){
    Markdown ast;
    MarkdownParser parser(true_markdown, meta_data);
    parser.process(ast);
    return ast;
}

// mdファイルの内容から
// メタデータ (std::map<std::string, std::string>) と
// 抽象構文木の根 (Block) のペアを返す。
Markdown parse(std::vector<std::string> content) {
    auto [meta_data, meta_data_end] = parse_front(content);

    // メタデータ以降の行を取り出し
    std::vector<std::string> md_lines;
    for (int i = meta_data_end; i < (int)content.size(); i++) {
        md_lines.push_back(content[i]);
    }

    // 前処理
    md_lines = preprocess(md_lines);

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

    // パース
    MarkdownParser parser(md_lines, meta_data_map);
    Markdown ast(meta_data_map);
    parser.process(ast);

    return ast;
}


// mdファイルのパスから
// メタデータ (std::map<std::string, std::string>) と
// 抽象構文木の根 (Block) のペアを返す。
Markdown parse_md_file(std::string path) {
    std::vector<std::string> content = read_file(path);
    return parse(content);
}

} // namespace almo
