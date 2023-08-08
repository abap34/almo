#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <map>

#include "ast.hpp"

namespace almo {

    struct InlineParser;
    struct BlockParser;

    struct InlineParser {
        AST::node_ptr processer(std::string s) {
            map.clear();
            while (1) {
                if (std::regex_match(s, math_regex)) {
                    auto& memo = map[InlineMath];
                    int id = memo.size();
                    std::string format = "$1<math>" + std::to_string(id) + "</math>$3";
                    memo.emplace_back(std::regex_replace(s, math_regex, "$2"));
                    s = std::regex_replace(s, math_regex, format);
                }
                else if (std::regex_match(s, overline_regex)) {
                    auto& memo = map[InlineOverline];
                    int id = memo.size();
                    std::string format = "$1<overline>" + std::to_string(id) + "</overline>$3";
                    memo.emplace_back(std::regex_replace(s, overline_regex, "$2"));
                    s = std::regex_replace(s, overline_regex, format);
                }
                else if (std::regex_match(s, strong_regex)) {
                    auto& memo = map[InlineStrong];
                    int id = memo.size();
                    std::string format = "$1<strong>" + std::to_string(id) + "</strong>$3";
                    memo.emplace_back(std::regex_replace(s, strong_regex, "$2"));
                    s = std::regex_replace(s, strong_regex, format);
                }
                else if (std::regex_match(s, italic_regex)) {
                    auto& memo = map[InlineItalic];
                    int id = memo.size();
                    std::string format = "$1<i>" + std::to_string(id) + "</i>$3";
                    memo.emplace_back(std::regex_replace(s, italic_regex, "$2"));
                    s = std::regex_replace(s, italic_regex, format);
                }
                else break;
            }
            auto node = std::make_shared<AST>();
            node->type = Block;
            node->childs = dfs(s);
            return node;
        }

        std::vector<AST::node_ptr> dfs(std::string s) {
            std::vector<AST::node_ptr> nodes;
            if (std::regex_match(s, italic_html_regex)) {
                auto node = std::make_shared<AST>();
                node->type = InlineItalic;
                int id = std::stoi(std::regex_replace(s, italic_html_regex, "$2"));
                auto s1 = std::regex_replace(s, italic_html_regex, "$1");
                auto s2 = map[InlineItalic][id];
                auto s3 = std::regex_replace(s, italic_html_regex, "$3");
                auto d1 = dfs(s1);
                node->childs = dfs(s2);
                auto d3 = dfs(s3);

                nodes.insert(nodes.end(), d1.begin(), d1.begin());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else if (std::regex_match(s, strong_html_regex)) {
                auto node = std::make_shared<AST>();
                node->type = InlineStrong;
                int id = std::stoi(std::regex_replace(s, strong_html_regex, "$2"));
                auto s1 = std::regex_replace(s, strong_html_regex, "$1");
                auto s2 = map[InlineStrong][id];
                auto s3 = std::regex_replace(s, strong_html_regex, "$3");
                auto d1 = dfs(s1);
                node->childs = dfs(s2);
                auto d3 = dfs(s3);

                nodes.insert(nodes.end(), d1.begin(), d1.begin());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else if (std::regex_match(s, overline_html_regex)) {
                auto node = std::make_shared<AST>();
                node->type = InlineOverline;
                int id = std::stoi(std::regex_replace(s, overline_html_regex, "$2"));
                auto s1 = std::regex_replace(s, overline_html_regex, "$1");
                auto s2 = map[InlineOverline][id];
                auto s3 = std::regex_replace(s, overline_html_regex, "$3");
                auto d1 = dfs(s1);
                node->childs = dfs(s2);
                auto d3 = dfs(s3);

                nodes.insert(nodes.end(), d1.begin(), d1.begin());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else if (std::regex_match(s, math_html_regex)) {
                auto node = std::make_shared<AST>();
                node->type = InlineMath;
                int id = std::stoi(std::regex_replace(s, math_html_regex, "$2"));
                auto s1 = std::regex_replace(s, math_html_regex, "$1");
                auto s2 = map[InlineMath][id];
                auto s3 = std::regex_replace(s, math_html_regex, "$3");
                auto d1 = dfs(s1);
                node->childs.emplace_back(std::make_shared<AST>(PlainText, s2));
                auto d3 = dfs(s3);

                nodes.insert(nodes.end(), d1.begin(), d1.begin());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else {
                nodes.emplace_back(std::make_shared<AST>(PlainText, s));
            }
            return nodes;
        }

        std::map<Type, std::vector<std::string>> map;
        const std::regex math_regex = std::regex("(.*)\\$(.*)\\$(.*)");
        const std::regex overline_regex = std::regex("(.*)\\~\\~(.*)\\~\\~(.*)");
        const std::regex strong_regex = std::regex("(.*)\\*\\*(.*)\\*\\*(.*)");
        const std::regex italic_regex = std::regex("(.*)\\*(.*)\\*(.*)");
        const std::regex math_html_regex = std::regex("(.*)<math>(.*)</math>(.*)");
        const std::regex overline_html_regex = std::regex("(.*)<overline>(.*)</overline>(.*)");
        const std::regex strong_html_regex = std::regex("(.*)<strong>(.*)</strong>(.*)");
        const std::regex italic_html_regex = std::regex("(.*)<i>(.*)</i>(.*)");
    };

    struct BlockParser {
        static std::vector<AST::node_ptr> processer(const std::vector<std::string>& lines) {
            std::vector<AST::node_ptr> asts;
            InlineParser inline_parser;
            int idx = 0;
            while (idx < (int)lines.size()) {
                std::string line = lines[idx];
                if (line.starts_with("# ")) {
                    auto block = std::make_shared<AST>(H1);
                    auto plain_block = std::make_shared<AST>();
                    block->childs.emplace_back(inline_parser.processer(line.substr(2)));
                    asts.emplace_back(block);
                }
                else if (line.starts_with("## ")) {
                    auto block = std::make_shared<AST>(H2);
                    auto plain_block = std::make_shared<AST>();
                    block->childs.emplace_back(inline_parser.processer(line.substr(3)));
                    asts.emplace_back(block);
                }
                else if (line.starts_with("### ")) {
                    auto block = std::make_shared<AST>(H3);
                    auto plain_block = std::make_shared<AST>();
                    block->childs.emplace_back(inline_parser.processer(line.substr(4)));
                    asts.emplace_back(block);
                }
                else if (line.starts_with("#### ")) {
                    auto block = std::make_shared<AST>(H4);
                    auto plain_block = std::make_shared<AST>();
                    block->childs.emplace_back(inline_parser.processer(line.substr(5)));
                    asts.emplace_back(block);
                }
                else if (line.starts_with("##### ")) {
                    auto block = std::make_shared<AST>(H5);
                    auto plain_block = std::make_shared<AST>();
                    block->childs.emplace_back(inline_parser.processer(line.substr(6)));
                    asts.emplace_back(block);
                }
                else if (line.starts_with("###### ")) {
                    auto block = std::make_shared<AST>(H6);
                    auto plain_block = std::make_shared<AST>();
                    block->childs.emplace_back(inline_parser.processer(line.substr(7)));
                    asts.emplace_back(block);
                }
                else if (line == ":::code") {
                    idx++;
                    auto block = std::make_shared<AST>(CodeRunner);
                    for (std::string head : { "title", "sample_in", "sample_out", "in", "out" }) {
                        assert(idx < (int)lines.size());
                        assert(lines[idx].starts_with(head));
                        block->code_runner.emplace_back(head, lines[idx].substr(head.size() + 1));
                        idx++;
                    }
                    assert(idx < (int)lines.size());
                    if (lines[idx].starts_with("judge=")) {
                        std::string rhs = lines[idx].substr(6);
                        assert(rhs.starts_with("err_") || rhs == "equal");
                        block->code_runner.emplace_back("judge", rhs);
                        idx++;
                    }
                    else {
                        block->code_runner.emplace_back("judge", "equal");
                    }
                    assert(idx < (int)lines.size() && lines[idx].starts_with(":::"));
                    asts.emplace_back(block);
                }
                else if (line.starts_with("```")) {
                    idx++;
                    auto block = std::make_shared<AST>(CodeBlock);
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == "```") break;
                        block->childs.emplace_back(std::make_shared<AST>(PlainText, lines[idx]));
                        idx++;
                    }
                    asts.emplace_back(block);
                }
                else if (line == "$$") {
                    idx++;
                    auto block = std::make_shared<AST>(MathBlock);
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == "$$") break;
                        block->childs.emplace_back(std::make_shared<AST>(PlainText, lines[idx]));
                        idx++;
                    }
                    asts.emplace_back(block);
                }
                else if (line.starts_with("- ")){
                    // item が途切れるまで行を跨いでパースする。 cur は indent 分だけずらしたカーソルを表す。
                    // 返り値は item が終了した直後の行の idx と パース結果を表す構文木のポインタ 。
                    auto item_parser = [&](int line_id, int cur = 0) -> std::pair<int,AST::node_ptr> {
                        auto block = std::make_shared<AST>(Item);
                        assert(lines[line_id].substr(cur,2) == "- ");
                        std::string aitem = lines[line_id].substr(cur+2);
                        while (true){
                            line_id++;
                            if (line_id == (int)(lines.size())) break;
                            if (lines[line_id].starts_with("- ")) break;
                            if (lines[line_id].starts_with("#")) break;
                            if (lines[line_id].starts_with(":::")) break;
                            if (lines[line_id].starts_with("```")) break;
                            if (lines[line_id].starts_with("$$")) break;
                            if (lines[line_id] == "") break;
                            aitem += lines[line_id];
                        }
                        block->childs.emplace_back(inline_parser.processer(aitem));
                        return std::make_pair(line_id,block);
                    };
                    auto block = std::make_shared<AST>(ListBlock);
                    while (true){
                        auto [nxt, ptr] = item_parser(idx);
                        block->childs.emplace_back(ptr);
                        idx = nxt;
                        if (idx < (int)(lines.size()) && lines[idx].starts_with("- ")) continue;
                        break;
                    }
                    idx--;
                    asts.emplace_back(block);
                }
                else if (line.starts_with("- ")){
                    // // item が途切れるまで行を跨いでパースする。返り値は item が終了した直後の行の idx と パース結果を表す構文木のポインタ 。
                    // auto item_parser = [&](int line_id, int cur = 0) -> std::pair<int,AST::node_ptr> {
                    //     auto block = std::make_shared<AST>(Item);
                    //     assert(lines[line_id].substr(cur,2) == "- ");
                    //     std::string aitem = lines[line_id].substr(cur+2);
                    //     while (true){
                    //         line_id++;
                    //         if (line_id == (int)(lines.size())) break;
                    //         if (lines[line_id].starts_with("- ")) break;
                    //         if (lines[line_id].starts_with("#")) break;
                    //         if (lines[line_id].starts_with(":::")) break;
                    //         if (lines[line_id].starts_with("```")) break;
                    //         if (lines[line_id].starts_with("$$")) break;
                    //         if (lines[line_id] == "") break;
                    //         // TODO ここには、item -> ListBlock の遷移が書かれる
                    //         aitem += lines[line_id];
                    //     }
                    //     block->childs.emplace_back(inline_parser.processer(aitem));
                    // };
                    // auto get_indent = [](const std::string &____str){
                    //     int indent_cur = 0;
                    //     while (indent_cur < (int)(____str.size()) && ____str[indent_cur] == ' ') indent_cur++;
                    //     return indent_cur;
                    // };
                    // auto list_dfs = [&](auto list_dfs, int line_id, int ListBlock_Item_PlainText) -> std::pair<int,AST::node_ptr> {
                    //     if (ListBlock_Item_PlainText == 2){
                    //         int indent_cur = get_indent(lines[line_id]);
                    //         assert(lines[line_id].substr(indent_cur,2) == "- ");
                    //         auto block = std::make_shared<AST>();
                    //         block->childs.emplace_back(inline_parser.processer(lines[line_id].substr(indent_cur+2)));
                    //     }
                    //     int indent_cur = get_indent(lines[line_id]);
                    //     assert(lines[line_id].substr(indent_cur,2) == "- ");
                    //     auto block = std::make_shared<AST>(ListBlock);
                    //     int cur_id = line_id;
                    //     while (true){
                    //         if ((int)lines[cur_id].size() >= indent_cur+2 && lines[cur_id].substr(indent_cur,2) == "- "){
                    //             block->childs.emplace_back(std::make_shared<AST>())
                    //         }
                    //     }
                    // };
                    // auto [next_idx, block] = list_dfs(list_dfs,idx,0);
                    // idx = next_idx;
                    // asts.emplace_back(block);
                }
                else if (line == "") {
                    auto block = std::make_shared<AST>(NewLine);
                    asts.emplace_back(block);
                }
                else {
                    auto plain_block = std::make_shared<AST>();
                    asts.emplace_back(inline_parser.processer(line));
                }
                idx++;
            }
            return asts;
        }
    };

    std::vector<std::string> read_md(const std::string& path) {
        std::vector<std::string> lines;
        std::ifstream file(path);

        if (!file) {
            std::cerr << "Error: Unable to open file: " << path << std::endl;
            return lines;
        }

        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        file.close();
        return lines;
    }

    std::vector<AST::node_ptr> parse_md_file(std::string path) {
        auto lines = almo::read_md(path);
        return BlockParser::processer(lines);
    }
}