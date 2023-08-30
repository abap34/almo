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

    // インラインのmd記法をパースします
    // 使用例:
    //   InlineParser inline_parser;
    //   inline_parser.processer(s);
    struct InlineParser {
        // mdの1行を入力しインラインのmd記法をパースしてその行の構文木の根ノードを返します。
        // パースの例:
        //    "**a**b$c$" をhtml表記にすると"<strong>a</strong>b\[c\]"です。
        //    最終的にhtmlを生成するためにこの関数ではパースしてできる構文木を作ります。
        AST::node_ptr processer(std::string s) {
            map.clear();
            while (1) {
                if (std::regex_match(s, code_block_regex)) {
                    auto& memo = map[InlineCodeBlock];
                    int id = memo.size();
                    std::string format = "$1<__code>" + std::to_string(id) + "</__code>$3";
                    memo.emplace_back(std::regex_replace(s, code_block_regex, "$2"));
                    s = std::regex_replace(s, code_block_regex, format);
                }
                else if (std::regex_match(s, math_regex)) {
                    auto& memo = map[InlineMath];
                    int id = memo.size();
                    std::string format = "$1<__math>" + std::to_string(id) + "</__math>$3";
                    memo.emplace_back(std::regex_replace(s, math_regex, "$2"));
                    s = std::regex_replace(s, math_regex, format);
                }
                else if (std::regex_match(s, image_regex)) {
                    auto& memo = map[InlineImage];
                    int id_url = memo.size();
                    int id_str = id_url + 1;
                    std::string format = "$1<__image=" + std::to_string(id_url) + ">" + std::to_string(id_str) + "</__image>$4";
                    memo.emplace_back(std::regex_replace(s, url_regex, "$3"));
                    memo.emplace_back(std::regex_replace(s, url_regex, "$2"));

                    s = std::regex_replace(s, url_regex, format);
                }
                else if (std::regex_match(s, url_regex)) {
                    auto& memo = map[InlineUrl];
                    int id_url = memo.size();
                    int id_str = id_url + 1;
                    std::string format = "$1<__url=" + std::to_string(id_url) + ">" + std::to_string(id_str) + "</__url>$4";
                    memo.emplace_back(std::regex_replace(s, url_regex, "$3"));
                    memo.emplace_back(std::regex_replace(s, url_regex, "$2"));

                    s = std::regex_replace(s, url_regex, format);
                }
                else if (std::regex_match(s, overline_regex)) {
                    auto& memo = map[InlineOverline];
                    int id = memo.size();
                    std::string format = "$1<__overline>" + std::to_string(id) + "</__overline>$3";
                    memo.emplace_back(std::regex_replace(s, overline_regex, "$2"));
                    s = std::regex_replace(s, overline_regex, format);
                }
                else if (std::regex_match(s, strong_regex)) {
                    auto& memo = map[InlineStrong];
                    int id = memo.size();
                    std::string format = "$1<__strong>" + std::to_string(id) + "</__strong>$3";
                    memo.emplace_back(std::regex_replace(s, strong_regex, "$2"));
                    s = std::regex_replace(s, strong_regex, format);
                }
                else if (std::regex_match(s, italic_regex)) {
                    auto& memo = map[InlineItalic];
                    int id = memo.size();
                    std::string format = "$1<__i>" + std::to_string(id) + "</__i>$3";
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

    private:
        // パースの内部で呼ばれるdfsです。
        // processer以外で呼ばれることはないです
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

                nodes.insert(nodes.end(), d1.begin(), d1.end());
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

                nodes.insert(nodes.end(), d1.begin(), d1.end());
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

                nodes.insert(nodes.end(), d1.begin(), d1.end());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else if (std::regex_match(s, url_html_regex)) {
                auto node = std::make_shared<AST>(InlineUrl);
                int id_url = std::stoi(std::regex_replace(s, url_html_regex, "$2"));
                int id_str = std::stoi(std::regex_replace(s, url_html_regex, "$3"));
                auto s1 = std::regex_replace(s, url_html_regex, "$1");
                auto s4 = std::regex_replace(s, url_html_regex, "$4");
                auto d1 = dfs(s1);
                node->childs.emplace_back(std::make_shared<AST>(Url, map[InlineUrl][id_url]));
                node->childs.emplace_back(std::make_shared<AST>(PlainText, map[InlineUrl][id_str]));
                auto d4 = dfs(s4);

                nodes.insert(nodes.end(), d1.begin(), d1.end());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d4.begin(), d4.end());
            }
            else if (std::regex_match(s, image_html_regex)) {
                auto node = std::make_shared<AST>(InlineImage);
                int id_url = std::stoi(std::regex_replace(s, image_html_regex, "$2"));
                int id_str = std::stoi(std::regex_replace(s, image_html_regex, "$3"));
                auto s1 = std::regex_replace(s, image_html_regex, "$1");
                auto s4 = std::regex_replace(s, image_html_regex, "$4");
                auto d1 = dfs(s1);
                node->childs.emplace_back(std::make_shared<AST>(Url, map[InlineImage][id_url]));
                node->childs.emplace_back(std::make_shared<AST>(PlainText, map[InlineImage][id_str]));
                auto d4 = dfs(s4);

                nodes.insert(nodes.end(), d1.begin(), d1.end());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d4.begin(), d4.end());
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

                nodes.insert(nodes.end(), d1.begin(), d1.end());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else if (std::regex_match(s, code_block_html_regex)) {
                auto node = std::make_shared<AST>();
                node->type = InlineCodeBlock;
                int id = std::stoi(std::regex_replace(s, code_block_html_regex, "$2"));
                auto s1 = std::regex_replace(s, code_block_html_regex, "$1");
                auto s2 = map[InlineCodeBlock][id];
                auto s3 = std::regex_replace(s, code_block_html_regex, "$3");
                auto d1 = dfs(s1);
                node->childs.emplace_back(std::make_shared<AST>(PlainText, s2));
                auto d3 = dfs(s3);
                nodes.insert(nodes.end(), d1.begin(), d1.end());
                nodes.emplace_back(node);
                nodes.insert(nodes.end(), d3.begin(), d3.end());
            }
            else {
                nodes.emplace_back(std::make_shared<AST>(PlainText, s));
            }
            return nodes;
        }

        std::map<Type, std::vector<std::string>> map;
        const std::regex code_block_regex = std::regex("(.*)\\`(.*)\\`(.*)");
        const std::regex math_regex = std::regex("(.*)\\$(.*)\\$(.*)");
        const std::regex image_regex = std::regex("(.*)\\!\\[(.*)\\]\\((.*)\\)(.*)");
        const std::regex url_regex = std::regex("(.*)\\[(.*)\\]\\((.*)\\)(.*)");
        const std::regex overline_regex = std::regex("(.*)\\~\\~(.*)\\~\\~(.*)");
        const std::regex strong_regex = std::regex("(.*)\\*\\*(.*)\\*\\*(.*)");
        const std::regex italic_regex = std::regex("(.*)\\*(.*)\\*(.*)");
        const std::regex code_block_html_regex = std::regex("(.*)<__code>(.*)</__code>(.*)");
        const std::regex math_html_regex = std::regex("(.*)<__math>(.*)</__math>(.*)");
        const std::regex image_html_regex = std::regex("(.*)<__image=(.*)>(.*)</__image>(.*)");
        const std::regex url_html_regex = std::regex("(.*)<__url=(.*)>(.*)</__url>(.*)");
        const std::regex overline_html_regex = std::regex("(.*)<__overline>(.*)</__overline>(.*)");
        const std::regex strong_html_regex = std::regex("(.*)<__strong>(.*)</__strong>(.*)");
        const std::regex italic_html_regex = std::regex("(.*)<__i>(.*)</__i>(.*)");
    };

    // md全体をパースするための関数をメンバーに持つ構造体です。
    struct BlockParser {
        // md全体を入力として与え、それをパースした構文木の列を返す関数です。
        // mdは始め、行ごとに分割されて入力として与えます。その後関数内でパースし意味のブロック毎に構文木を作ります。
        // 使用例:
        //    BlockParser::processer(lines);
        static std::vector<AST::node_ptr> processer( std::vector<std::string> lines) {
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
                    for (std::string head : { "title", "sample_in", "sample_out", "in", "out"}) {
                        assert(idx < (int)lines.size());
                        assert(lines[idx].starts_with(head));
                        block->code_runner.emplace_back(head, lines[idx].substr(head.size() + 1));
                        idx++;
                    }
                    assert(idx < (int)lines.size());
                    bool find_judge = false;
                    bool find_source = false;
                    if (lines[idx].starts_with("judge=")) {
                        std::string rhs = lines[idx].substr(6);
                        assert(rhs.starts_with("err_") || rhs == "equal");
                        block->code_runner.emplace_back("judge", rhs);
                        find_judge = true;
                        idx++;
                    } 
                    assert(idx < (int)lines.size());
                    if (lines[idx].starts_with("source=")) {
                        std::string source_path = lines[idx].substr(7);
                        block->code_runner.emplace_back("source", source_path);
                        find_source = true;
                        idx++;
                    }
                    assert(idx < (int)lines.size());
                    assert(lines[idx] == ":::");
                    if (!find_judge) {
                        block->code_runner.emplace_back("judge", "equal");
                    }
                    if (!find_source) {
                        block->code_runner.emplace_back("source", "");
                    }
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
                    assert(lines[idx] == "```");
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
                else if (line.starts_with("- ")) {
                    // item が途切れるまで行を跨いでパースする。 cur は indent 分だけずらしたカーソルを表す。
                    // 返り値は item が終了した直後の行の idx と パース結果を表す構文木のポインタ 。
                    auto list_parser = [&](auto &&self, int line_id, int cur = 0) -> std::pair<int, AST::node_ptr> {
                        auto block = std::make_shared<AST>(ListBlock);
                        assert(lines[line_id].substr(cur, 2) == "- ");
                        AST::node_ptr now = nullptr;
                        std::string content = "";
                        while (true) {
                            if (line_id == (int)(lines.size())) break;
                            if(is_header(lines[line_id])) break;
                            else if (lines[line_id].starts_with(":::")) break;
                            else if (lines[line_id].starts_with("```")) break;
                            else if (lines[line_id].starts_with("$$")) break;
                            else if (lines[line_id] == "") break;
                            else {
                                std::string list_header = "- ";
                                bool is_upper = false;
                                for(int i = 0; i < cur; i += 2) {
                                    if(lines[line_id].starts_with(list_header)) {
                                        assert(now != nullptr);
                                        is_upper = true;
                                        break;
                                    }
                                    list_header = "  " + list_header;
                                }
                                if(is_upper) break;
                                if(lines[line_id].starts_with(list_header)) {
                                    if(now != nullptr) {
                                        now->childs.insert(now->childs.begin(), inline_parser.processer(content));
                                        block->childs.emplace_back(now);
                                        content.clear();
                                    }
                                    now = std::make_shared<AST>(Item);
                                    content = lines[line_id].substr(list_header.size());
                                }
                                else if (lines[line_id].starts_with("  " + list_header)) {
                                    assert(now != nullptr);
                                    auto [next_line, item_ptr] = self(self, line_id, cur + 2);
                                    line_id = next_line - 1;
                                    now->childs.emplace_back(item_ptr);
                                }
                                else {
                                    assert(now != nullptr);
                                    content += " " + lines[line_id];
                                }
                                line_id++;
                            }
                        }
                        now->childs.insert(now->childs.begin(), inline_parser.processer(content));
                        block->childs.emplace_back(now);
                        return std::make_pair(line_id, block);
                    };
                    auto [next, list_ast] = list_parser(list_parser, idx);
                    idx = next;
                    asts.emplace_back(list_ast);
                }
                else if (line == "") {
                    auto block = std::make_shared<AST>(NewLine);
                    asts.emplace_back(block);
                }
                else if (std::regex_match(line, std::regex("(\\|[^\\|]+).+\\|"))) {
                    const std::regex each_col_regex = std::regex("\\|[^\\|]+");

                    int n_col = 0;

                    std::vector<std::string> col_names(n_col);
                    std::smatch match;

                    while (std::regex_search(line, match, each_col_regex)) {
                        col_names.push_back(match[0].str().substr(1, match[0].str().size()));
                        line = match.suffix();
                        n_col++;
                    }

                    // 0 -> 左寄せ, 1 -> 中央寄せ, 2 -> 右寄せ
                    std::vector<int> col_format(0);

                    idx++;

                    std::string line2 = lines[idx];
                    std::smatch match2;
                    while (std::regex_search(line2, match2, each_col_regex)) {
                        if (match2[0].str().starts_with("|:") && match2[0].str().ends_with(":")) {
                            col_format.push_back(1);
                        }
                        else if (match2[0].str().starts_with("|:")) {
                            col_format.push_back(0);
                        }
                        else if (match2[0].str().ends_with(":")) {
                            col_format.push_back(2);
                        }
                        else {
                            col_format.push_back(0);
                        }

                        line2 = match2.suffix();
                    }


                    idx++;
                    int n_row = 0;
                    std::vector<std::string> table;


                    while (idx < (int)(lines.size()) && lines[idx] != "") {
                        n_row++;
                        std::string line = lines[idx];
                        std::smatch match;
                        std::regex_search(line, match, each_col_regex);
                        while (std::regex_search(line, match, each_col_regex)) {
                            table.push_back(match[0].str().substr(1, match[0].str().size()));
                            line = match.suffix();
                        }
                        idx++;
                    }


                    auto block = std::make_shared<AST>(Table);
                    block->table.emplace_back("n_col", std::to_string(n_col));
                    block->table.emplace_back("n_row", std::to_string(n_row));
                    block->col_format = col_format;
                    block->col_names = col_names;

                    for (int i = 0; i < (int)table.size(); i++) {
                        block->childs.emplace_back(inline_parser.processer(table[i]));
                    }

                    asts.emplace_back(block);
                }
                else {
                    asts.emplace_back(inline_parser.processer(line));
                }
                idx++;
            }
            return asts;
        }

        private:
        // headerブロックであるか判定
        static bool is_header(const std::string &s) {
            std::string header = "";
            for(int i = 1; i <= 6; i++) {
                header += '#';
                if(s.starts_with(header + " ")) return true;
            }
            return false;
        }
    };

    // mdファイルのパスを入力として与えて、mdファイルの中身を行ごとに分割したstd::vector<std::string>を返します。
    // 仕様例:
    //    read_md("example.md");
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

    // mdファイルのパスを入力として与えて、そのmdファイルをパースした結果（構文木のリスト）を返します。
    // 使用例:
    //     parse_md_file("example.md");
    std::pair<std::vector<std::pair<std::string, std::string>>, std::vector<AST::node_ptr>> parse_md_file(std::string path) {
        std::vector<std::string> lines = almo::read_md(path);
        
        std::string all_line_str;

        for (std::string line : lines) {
            all_line_str += line;
            all_line_str += '\n';
        }

        std::regex comment = std::regex("([\\s\\S]*)<!--([\\s\\S]*)-->([\\s\\S]*)");
        std::smatch match;

        while (std::regex_search(all_line_str, match, comment)) {
            all_line_str = std::regex_replace(all_line_str, comment, match[1].str() + match[3].str());
        }

        std::vector<std::string> processed_lines;

        std::string current_line = "";
        for (char c : all_line_str) {
            if (c == '\n') {    
                processed_lines.push_back(current_line);
                current_line = "";
                
            }
            else {
                current_line += c;
            }
        }
        std::vector<std::pair<std::string, std::string>> meta_data;
        int meta_data_end = 0;
        if(!processed_lines.empty() && processed_lines[0] == "---") {
            int index = 1;
            while(index < (int)processed_lines.size() && processed_lines[index] != "---") {
                std::string key = std::regex_replace(processed_lines[index], std::regex("(.*):(.*)"), "$1");
                std::string data = std::regex_replace(processed_lines[index], std::regex("(.*):(.*)"), "$2");
                meta_data.emplace_back(key, data);
                index++;
            }
            meta_data_end = index + 1;
        }
        return {meta_data, BlockParser::processer({processed_lines.begin() + meta_data_end, processed_lines.end()})};
    }
}