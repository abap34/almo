#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <map>
#include <stack>

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

        std::map<std::string, std::vector<std::string>> map;
        std::shared_ptr<Block> processer(std::string s) {
            Block root = Block(uuid());
            map.clear();
            while (1) {
                if (std::regex_match(s, code_block_regex)) {
                    auto& memo = map["inline_code_block"];
                    int id = memo.size();
                    std::string format = "$1<__code>" + std::to_string(id) + "</__code>$3";
                    memo.push_back(std::regex_replace(s, code_block_regex, "$2"));
                    s = std::regex_replace(s, code_block_regex, format);
                }
                else if (std::regex_match(s, math_regex)) {
                    auto& memo = map["inline_math"];
                    int id = memo.size();
                    std::string format = "$1<__math>" + std::to_string(id) + "</__math>$3";
                    memo.push_back(std::regex_replace(s, math_regex, "$2"));
                    s = std::regex_replace(s, math_regex, format);
                }
                else if (std::regex_match(s, image_regex)) {
                    auto& memo = map["inline_image"];
                    int id_url = memo.size();
                    int id_str = id_url + 1;
                    std::string format = "$1<__image=" + std::to_string(id_url) + ">" + std::to_string(id_str) + "</__image>$4";
                    memo.push_back(std::regex_replace(s, image_regex, "$3"));
                    memo.push_back(std::regex_replace(s, image_regex, "$2"));
                    s = std::regex_replace(s, image_regex, format);
                }
                else if (std::regex_match(s, url_regex)) {
                    auto& memo = map["inline_url"];
                    int id_url = memo.size();
                    int id_str = id_url + 1;
                    std::string format = "$1<__url=" + std::to_string(id_url) + ">" + std::to_string(id_str) + "</__url>$4";
                    memo.push_back(std::regex_replace(s, url_regex, "$3"));
                    memo.push_back(std::regex_replace(s, url_regex, "$2"));

                    s = std::regex_replace(s, url_regex, format);
                }
                else if (std::regex_match(s, overline_regex)) {
                    auto& memo = map["overline"];
                    int id = memo.size();
                    std::string format = "$1<__overline>" + std::to_string(id) + "</__overline>$3";
                    memo.push_back(std::regex_replace(s, overline_regex, "$2"));
                    s = std::regex_replace(s, overline_regex, format);
                }
                else if (std::regex_match(s, strong_regex)) {
                    auto& memo = map["strong"];
                    int id = memo.size();
                    std::string format = "$1<__strong>" + std::to_string(id) + "</__strong>$3";
                    memo.push_back(std::regex_replace(s, strong_regex, "$2"));
                    s = std::regex_replace(s, strong_regex, format);
                }
                else if (std::regex_match(s, italic_regex)) {
                    auto& memo = map["italic"];
                    int id = memo.size();
                    std::string format = "$1<__i>" + std::to_string(id) + "</__i>$3";
                    memo.push_back(std::regex_replace(s, italic_regex, "$2"));
                    s = std::regex_replace(s, italic_regex, format);
                }
                else break;
            }
            root.childs.push_back(std::make_shared<Block>(dfs(s)));
            return std::make_shared<Block>(root);
        }

    private:
        Block dfs(std::string s) {
            Block root = Block(uuid());
            if (std::regex_match(s, italic_html_regex)) {
                InlineItalic node = InlineItalic(uuid());
                int id = std::stoi(std::regex_replace(s, italic_html_regex, "$2"));
                std::string before = std::regex_replace(s, italic_html_regex, "$1");
                std::string content = map["italic"][id];
                std::string after = std::regex_replace(s, italic_html_regex, "$3");
                
                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                node.childs.push_back(std::make_shared<Block>(dfs(content)));
                root.childs.push_back(std::make_shared<InlineItalic>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));
            }
            else if (std::regex_match(s, strong_html_regex)) {
                InlineStrong node = InlineStrong(uuid());
                int id = std::stoi(std::regex_replace(s, strong_html_regex, "$2"));
                std::string before = std::regex_replace(s, strong_html_regex, "$1");
                std::string content = map["strong"][id];
                std::string after = std::regex_replace(s, strong_html_regex, "$3");
                
                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                node.childs.push_back(std::make_shared<Block>(dfs(content)));
                root.childs.push_back(std::make_shared<InlineStrong>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));
            }
            else if (std::regex_match(s, overline_html_regex)) {
                InlineOverline node = InlineOverline(uuid());
                int id = std::stoi(std::regex_replace(s, overline_html_regex, "$2"));
                std::string before = std::regex_replace(s, overline_html_regex, "$1");
                std::string content = map["overline"][id];
                std::string after = std::regex_replace(s, overline_html_regex, "$3");
                
                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                node.childs.push_back(std::make_shared<Block>(dfs(content)));
                root.childs.push_back(std::make_shared<InlineOverline>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));
            }
            else if (std::regex_match(s, url_html_regex)) {
                int id_url = std::stoi(std::regex_replace(s, url_html_regex, "$2"));
                int id_str = std::stoi(std::regex_replace(s, url_html_regex, "$3"));
                std::string before = std::regex_replace(s, url_html_regex, "$1");
                std::string after = std::regex_replace(s, url_html_regex, "$4");
                
                std::string url = map["inline_url"][id_url];
                std::string alt = map["inline_url"][id_str];

                InlineUrl node = InlineUrl(url, alt, uuid());
                
                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                // Url は Leaf なので、子要素を持たないからそのまま追加する。
                root.childs.push_back(std::make_shared<InlineUrl>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));

            }
            else if (std::regex_match(s, image_html_regex)) {
                int id_url = std::stoi(std::regex_replace(s, image_html_regex, "$2"));
                int id_str = std::stoi(std::regex_replace(s, image_html_regex, "$3"));
                std::string before = std::regex_replace(s, image_html_regex, "$1");
                std::string after = std::regex_replace(s, image_html_regex, "$4");
                
                std::string url = map["inline_image"][id_url];
                std::string caption = map["inline_image"][id_str];
                InlineImage node = InlineImage(url, caption, uuid());

                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                root.childs.push_back(std::make_shared<InlineImage>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));
            }
            else if (std::regex_match(s, math_html_regex)) {
                int id = std::stoi(std::regex_replace(s, math_html_regex, "$2"));

                auto before = std::regex_replace(s, math_html_regex, "$1");
                auto after = std::regex_replace(s, math_html_regex, "$3");

                std::string expression = map["inline_math"][id];

                InlineMath node = InlineMath(expression, uuid());

                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                root.childs.push_back(std::make_shared<InlineMath>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));
            }
            else if (std::regex_match(s, code_block_html_regex)) {
                int id = std::stoi(std::regex_replace(s, code_block_html_regex, "$2"));
                std::string before = std::regex_replace(s, code_block_html_regex, "$1");
                std::string after = std::regex_replace(s, code_block_html_regex, "$3");
                std::string code = map["inline_code_block"][id];

                InlineCodeBlock node = InlineCodeBlock(code, uuid());

                root.childs.push_back(std::make_shared<Block>(dfs(before)));
                root.childs.push_back(std::make_shared<InlineCodeBlock>(node));
                root.childs.push_back(std::make_shared<Block>(dfs(after)));

            }
            else {
                std::string text = "";
                RawText node = RawText(s, uuid());
                root.childs.push_back(std::make_shared<RawText>(node));
            }
            return root;
        }

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
        Block processer(std::vector<std::string> lines) {
            InlineParser inline_parser = InlineParser();
            Block root = Block(uuid());
            int idx = 0;
            while (idx < (int)lines.size()) {
                std::string line = lines[idx];
                if (line.starts_with("# ")) {
                    Header node = Header(1, uuid());
                    node.childs.push_back(inline_parser.processer(line.substr(2)));
                    root.childs.push_back(std::make_shared<Header>(node));
                }
                else if (line.starts_with("## ")) {
                    Header node = Header(2, uuid());
                    node.childs.push_back(inline_parser.processer(line.substr(3)));
                    root.childs.push_back(std::make_shared<Header>(node));
                }
                else if (line.starts_with("### ")) {
                    Header node = Header(3, uuid());
                    node.childs.push_back(inline_parser.processer(line.substr(4)));
                    root.childs.push_back(std::make_shared<Header>(node));
                }
                else if (line.starts_with("#### ")) {
                    Header node = Header(4, uuid());
                    node.childs.push_back(inline_parser.processer(line.substr(5)));
                    root.childs.push_back(std::make_shared<Header>(node));
                }
                else if (line.starts_with("##### ")) {
                    Header node = Header(5, uuid());
                    node.childs.push_back(inline_parser.processer(line.substr(6)));
                    root.childs.push_back(std::make_shared<Header>(node));
                }
                else if (line.starts_with("###### ")) {
                    Header node = Header(6, uuid());
                    node.childs.push_back(inline_parser.processer(line.substr(6)));
                    root.childs.push_back(std::make_shared<Header>(node));
                }
                else if (line == ":::judge") {
                    idx++;
                    std::map<std::string, std::string> judge_info;
                    
                    // Judge の設定のうち、必ず必要なもの
                    std::vector<std::string> required_args = { "title", "sample_in", "sample_out", "in", "out" };

                    // Judge の設定のうち、オプションのものとそのデフォルト値
                    std::map<std::string, std::string> optional_args = {
                        { "judge", "equal" },
                        { "source", "" }
                    };

                    // 1行ずつ読み込み、judge_info に情報を追加していく
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == ":::") break;
                        std::string key = lines[idx].substr(0, lines[idx].find("="));
                        std::string value = lines[idx].substr(lines[idx].find("=") + 1);
                        judge_info[key] = value;
                        idx++;
                    }

                    // 必須の引数が揃っているかチェック. なければ SyntaxError を投げる
                    for (std::string arg : required_args) {
                        if (judge_info.find(arg) == judge_info.end()) {
                            // title がある場合
                            if (judge_info.find("title") != judge_info.end()) {
                                throw SyntaxError("問題" + judge_info["title"] + "の引数 " + arg + " がありません. 引数を追加してください.");
                            } else {
                                throw SyntaxError("問題タイトルがありません. 引数を追加してください.");
                            }
                        }
                    }

                    // オプションの引数が揃っているかチェック. なければデフォルト値を入れる
                    for (auto [arg, default_value] : optional_args) {
                        if (judge_info.find(arg) == judge_info.end()) {
                            judge_info[arg] = default_value;
                        }
                    }

                    Judge node = Judge(
                        judge_info["title"],
                        judge_info["sample_in"],
                        judge_info["sample_out"],
                        judge_info["in"],
                        judge_info["out"],
                        judge_info["judge"],
                        judge_info["source"],
                        uuid()
                    );

                    root.childs.push_back(std::make_shared<Judge>(node));
                }
                else if (line == ":::code") {
                    idx++;
                    assert(idx < (int)lines.size());
                    std::string code;
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == ":::") break;
                        code += lines[idx] + "\n";
                        idx++;
                    }
                    ExecutableCodeBlock node = ExecutableCodeBlock(code, uuid());
                    root.childs.push_back(std::make_shared<ExecutableCodeBlock>(node));
                }
                else if (line == ":::loadlib") {
                    idx++;
                    assert(idx < (int)lines.size());
                    std::vector<std::string> libs;
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == ":::") break;
                        libs.push_back(lines[idx]);
                        idx++;
                    }
                    LoadLib node = LoadLib(libs, uuid());
                    root.childs.push_back(std::make_shared<LoadLib>(node));
                }
                else if (line.starts_with("```")) {
                    std::string language = line.substr(3);
                    idx++;
                    std::string code;
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == "```") break;
                        code += lines[idx] + "\n";
                        idx++;
                    }

                    CodeBlock node = CodeBlock(code, language, uuid());
                    root.childs.push_back(std::make_shared<CodeBlock>(node));
                }
                else if (line == "$$") {
                    idx++;
                    std::string expression;
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == "$$") break;
                        expression += lines[idx] + "\n";
                        idx++;
                    }
                    
                    MathBlock node = MathBlock(expression, uuid());
                    root.childs.push_back(std::make_shared<MathBlock>(node));
                }
                else if (line.starts_with("- ")) {
                    // まずは今の行(リストの先頭)のテキストを取り出す。
                    // 次の行が、 空行またはファイルの末端またはリストの次の要素の場合、テキストは終了。
                    // 逆にこれ以外の場合、テキストは継続。
                    // 例) - Hello,
                    //       This is a pen.
                    //    この場合、リストの先頭のテキストは "Hello, This is a pen." となる。(This is a pen. の部分も含む点に注意。)

                    // 以下のアルゴリズムでテキストを取り出す。
                    // 1. 次の行の内容を読む.
                    // 2. 以下の条件がどれか true になるまで読み続ける
                    // 　　条件: 次の行が、　
                    //          1. 空行(lines[idx + 1] == "")
                    //          2. ファイルの末端(idx + 1 == size) 
                    //          3. リストの次の要素

                    
                    // 1, 2 が見つかったら終了.
                    // 3 について、 リストの次の要素は、
                    // - current_prefix 
                    // - current_prefix にインデント(スペース2) を加えたもの
                    // - インデントを2n個削除したもの  
                    // から始まる   
                    // current_prefix 始まりなら 上のアルゴリズムでテキストを取得して, Item を現在着目しているスコープの子要素の Item として追加する。
                    // current_prefix + インデント 始まりなら、 リストが一つネストされているので、
                    // 新しく ListBlock を作成して、 現在着目しているスコープに積む。 同様にテキストを取得して、 Item を現在着目しているスコープの子要素(Item) として追加する。
                    // インデントを削除したものの場合、削除した個数分だけスタックから pop すればよい。

                    // 現在着目しているスコープを表すためのスタック
                    std::stack<ListBlock> scopes;
                    
                    ListBlock node = ListBlock(uuid());
                    scopes.push(node);

                    std::string current_prefix = "- ";
                    std::string INDENT = "  ";

                    // 条件1, 2 に当てはまるかどうかを判定する
                    auto is_end = [&lines](int idx) -> bool {
                        if (idx == (int)(lines.size())) return true;
                        if (lines[idx] == "") return true;
                        return false;
                    }; 

                    // 先頭のスペースを削除したとき、 "- " 始まりになるかどうかを判定する・ ワーニング出してやるのに使う。
                    auto is_list_def = [](std::string s) -> bool {
                        while (s[0] == ' ') s = s.substr(1);
                        return s.starts_with("- ");
                    };

                    // 今集めているテキスト
                    std::string text = "";

                    while (true) {
                        // 現在の行のテキストを取る。
                        text += lines[idx].substr(current_prefix.size());
                        
                        // 条件 1, 2
                        if (is_end(idx + 1)) break;

                        // 条件 3
                        // current_prefix から始まる場合
                        if (lines[idx + 1].starts_with(current_prefix)) {
                            // 継続終了。収集してきたテキストを追加。
                            Item item = Item(uuid());
                            item.childs.push_back(inline_parser.processer(line));
                            // 現在のスタックのトップにある ListBlock に Item を追加する。
                            scopes.top().childs.push_back(std::make_shared<Item>(item));
                            std::string text = "";

                            // ネストは発生していないので、現在着目しているスコープは変化しない。
                        } else if (lines[idx + 1].starts_with(INDENT + current_prefix)) {
                            // 継続終了。収集してきたテキストを追加。
                            Item item = Item(uuid());
                            item.childs.push_back(inline_parser.processer(line));
                            // 現在のスタックのトップにある ListBlock に Item を追加する。
                            scopes.top().childs.push_back(std::make_shared<Item>(item));
                            std::string text = "";

                            // ネストが発生しているので、現在着目しているスコープを変更する。
                            // 新しく ListBlock を作成
                            ListBlock new_node = ListBlock(uuid());
                            
                            // 今のスコープの子ノードにして、
                            scopes.top().childs.push_back(std::make_shared<ListBlock>(new_node));

                            // 新しいスコープに変更する。
                            scopes.push(new_node);

                        }  else if ((int)scopes.size() > 1) {
                            for (int i = 1; i < (int)scopes.size(); i++)
                            {   
                                // インデントを削除していく. 1つのインデントは 2文字なので、2文字ずつ削除していく。
                                current_prefix = current_prefix.substr(2);
                                if (lines[idx + 1].starts_with(current_prefix)) {
                                    // 継続終了. 収集してきたテキストを追加。
                                    Item item = Item(uuid());
                                    item.childs.push_back(inline_parser.processer(line));
                                    // 現在のスタックのトップにある ListBlock に Item を追加する。
                                    scopes.top().childs.push_back(std::make_shared<Item>(item));
                                    std::string text = "";

                                    // i 個数分上のスコープのリストになる
                                    for (int j = 0; j < i; j++) {
                                        scopes.pop();
                                    }
                                }
                            }
                        } else if (is_list_def(lines[idx + 1])) {
                            // リストの定義っぽいのにインデントがあっていないということなので、警告を出してやる
                            std::cerr << "Warning  " << idx + 1 << "行目:" << std::endl;
                            std::cerr << "リストの定義が継続している可能性がありますが、インデント幅が一致しません。" << std::endl;
                            std::cerr << "リストの継続を意図している場合、インデントとしてスペース2個を使っているか確認してください。" << std::endl;
                        } else {
                            idx++;
                        }
                    }
                    root.childs.push_back(std::make_shared<ListBlock>(node));
                }
                else if (std::regex_match(line, std::regex("\\d+\\. (.*)"))) {
 
                    // 現在着目しているスコープを表すためのスタック
                    std::stack<EnumerateBlock> scopes;
                    
                    EnumerateBlock node = EnumerateBlock(uuid());
                    scopes.push(node);

                    std::string current_prefix = "\\d+\\. (.*)";
                    // こっちはスペース3個であることに注意！！！
                    std::string INDENT = "   ";

                    // 条件1, 2 に当てはまるかどうかを判定する
                    auto is_end = [&lines](int idx) -> bool {
                        if (idx == (int)(lines.size())) return true;
                        if (lines[idx] == "") return true;
                        return false;
                    }; 

                    // 先頭のスペースを削除したとき、 "- " 始まりになるかどうかを判定する・ ワーニング出してやるのに使う。
                    auto is_enum_def = [](std::string s) -> bool {
                        while (s[0] == ' ') s = s.substr(1);
                        return std::regex_match(s, std::regex("\\d+\\. (.*)"));
                    };

                    // 今集めているテキスト
                    std::string text = "";
                    int depth = 0;

                    while (true) {
                        // 現在の行のテキストを取る。
                        text += lines[idx].substr((depth + 1) * 3);
                        // 条件 1, 2
                        if (is_end(idx + 1)) break;

                        // 条件 3
                        // current_prefix から始まる場合
                        if (lines[idx + 1].starts_with(current_prefix)) {
                            // 継続終了。収集してきたテキストを追加。
                            Item item = Item(uuid());
                            item.childs.push_back(inline_parser.processer(line));
                            // 現在のスタックのトップにある EnumerateBlock に Item を追加する。
                            scopes.top().childs.push_back(std::make_shared<Item>(item));
                            std::string text = "";

                            // ネストは発生していないので、現在着目しているスコープは変化しない。
                        } else if (lines[idx + 1].starts_with(INDENT + current_prefix)) {
                            // 継続終了。収集してきたテキストを追加。
                            Item item = Item(uuid());
                            item.childs.push_back(inline_parser.processer(line));
                            // 現在のスタックのトップにある EnumerateBlock に Item を追加する。
                            scopes.top().childs.push_back(std::make_shared<Item>(item));
                            std::string text = "";

                            // ネストが発生しているので、現在着目しているスコープを変更する。
                            // 新しく EnumerateBlock を作成して、 現在着目しているスコープに積む。
                            EnumerateBlock new_node = EnumerateBlock(uuid());
                            scopes.top().childs.push_back(std::make_shared<Item>(item));
                            scopes.push(new_node);
                        }  else if ((int)scopes.size() > 1) {
                            // インデントを削除していく. 1つのインデントは 3文字なので、3文字ずつ削除していく。
                            for (int i = 1; i < (int)scopes.size(); i++)
                            {
                                current_prefix = current_prefix.substr(3);
                                if (lines[idx + 1].starts_with(current_prefix)) {
                                    // 継続終了. 収集してきたテキストを追加。
                                    Item item = Item(uuid());
                                    item.childs.push_back(inline_parser.processer(line));
                                    // 現在のスタックのトップにある EnumerateBlock に Item を追加する。
                                    scopes.top().childs.push_back(std::make_shared<Item>(item));
                                    std::string text = "";

                                    // i 個数分上のスコープのリストになる
                                    for (int j = 0; j < i; j++) {
                                        scopes.pop();
                                    }
                                }
                            }
                        } else if (is_enum_def(lines[idx + 1])) {
                            // 番号付きリストの定義っぽいのにインデントがあっていないということなので、警告を出してやる
                            std::cerr << "Warning  " << idx + 1 << "行目:" << std::endl;
                            std::cerr << "番号付きリストの定義が継続している可能性がありますが、インデント幅が一致しません。" << std::endl;
                            std::cerr << "番号付きリストの継続を意図している場合、インデントとしてスペース2個を使っているか確認してください。" << std::endl;
                        } else {
                            idx++;
                        }
                    }
                    root.childs.push_back(std::make_shared<EnumerateBlock>(node));
                }
                else if (line.starts_with(">")) {
                    // 空行がくるか末端に来るまで読み続ける
                    Quote node = Quote(uuid());
                    
                    std::vector<std::string> quote_contents;
                    while (idx < (int)lines.size()) {
                        if (lines[idx] == "") break;
                        quote_contents.push_back(lines[idx]);
                        idx++;
                    }

                    // 引用の中身もパース
                    BlockParser parser = BlockParser();
                    node.childs.push_back(std::make_shared<Block>(parser.processer(quote_contents)));
                }
                else if (line == "") {
                    NewLine node = NewLine(uuid());
                    root.childs.push_back(std::make_shared<NewLine>(node));
                }
                // 水平線
                else if ((line == "---") || (line == "___") || (line == "***")) {
                    HorizontalLine node = HorizontalLine(uuid());
                    root.childs.push_back(std::make_shared<HorizontalLine>(node));
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


                    Table node =  Table(n_row, n_col, col_format, col_names, uuid()); 
                    
                    for (int i = 0; i < (int)table.size(); i++) {
                        node.childs.push_back(inline_parser.processer(table[i]));
                    }

                    root.childs.push_back(std::make_shared<Table>(node));
                }
                else {
                    root.childs.push_back(inline_parser.processer(line));
                }
                idx++;
            }
            return root;
        }
    };

    
    // mdファイルのパスを入力として与えて、そのmdファイルをパースした結果を返す。
    // 返り値は、
    // メタデータ (std::map<std::string, std::string>) と
    // 抽象構文木の根 (Block) のペア
    std::pair<std::map<std::string, std::string>, Block> parse_md_file(std::string path) {
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

        // パース
        BlockParser parser = BlockParser();
        Block ast = parser.processer(md_lines);

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

        return { meta_data_map,  ast};
    }
}
