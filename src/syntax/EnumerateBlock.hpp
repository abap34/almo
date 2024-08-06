#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "EOF.hpp"
#include "Item.hpp"
#include "NewLine.hpp"

namespace almo {

// 番号付き箇条書きを表すクラス
struct EnumerateBlock : public ASTNode {
   public:
    EnumerateBlock() { set_uuid(); }

    std::string to_html() const override {
        return "<ol>" + concatenated_childs_html() + "</ol>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "EnumerateBlock"; }
};

struct EnumerateBlockSyntax : public BlockSyntax {
    static inline const std::regex rex = std::regex(R"(\d+\. (.*))");
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        return std::regex_match(read.get_row(), rex);
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : rex match
        // END   : NewLine or EOF

        std::stack<std::shared_ptr<EnumerateBlock>> scopes;
        EnumerateBlock root_enum;
        scopes.push(std::make_shared<EnumerateBlock>(root_enum));

        std::string current_match = R"(\d+\. (.*))";
        // width : 3
        const std::string INDENT = "   ";

        // if s is considered a enum definition, return its depth"
        // otherwise return std::string::npos
        auto enum_depth = [](std::string s) -> std::size_t {
            std::size_t pos = s.find_first_not_of(" \n\r\t");
            // s is consist of only space
            if (pos == std::string::npos) return std::string::npos;
            // s is enum definition
            if (std::regex_match(s.substr(pos), rex)) return pos;
            // otherwise
            return std::string::npos;
        };

        std::string text = "";

        while (true) {
            // Invariant : read.is_eof() == false
            //             scopes.size() >= 1
            text += [&] {
                std::smatch sm;
                // smatch manage an iterator of the matched string `line`
                // --> the string should not be a temporary object
                std::string line = read.get_row();
                std::regex_search(line, sm, std::regex(current_match));
                return sm.format("$1");
            }();

            // within while loop, read changes only here
            read.move_next_line();

            // END
            if (EOFSyntax{}(read) || NewLineSyntax{}(read)) {
                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";
                break;
            }
            // same depth
            if (std::regex_match(read.get_row(), std::regex(current_match))) {
                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";
                continue;
            }
            // +3 depth
            if (std::regex_match(read.get_row(),
                                 std::regex(INDENT + current_match))) {
                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";

                // nested EnumerateBlock
                EnumerateBlock new_node;
                auto new_node_ptr = std::make_shared<EnumerateBlock>(new_node);
                scopes.top()->pushback_child(new_node_ptr);
                scopes.push(new_node_ptr);

                // update match
                current_match = INDENT + current_match;
                continue;
            }
            // -3n depth (n >= 1)
            if ([&] {
                    std::size_t depth = enum_depth(read.get_row());
                    if (depth == std::string::npos) return false;
                    std::size_t current = (scopes.size() - 1u) * 3u;
                    // depth == current, depth == current + 3 is already
                    // ditected depth has decreased and the difference is
                    // multiple of 3
                    if (depth < current && (current - depth) % 3 == 0)
                        return true;
                    // ambiguous enum definition detected
                    std::cerr << "Warning: ambiguous enum\n ... \n"
                              << read.near() << "\n^^^ parsing line"
                              << std::endl;
                    std::cerr << "indent width must be 3. this line is "
                                 "considered as raw text."
                              << std::endl;
                    return false;
                }()) {
                std::size_t delete_indent =
                    (scopes.size() - 1u) * 3u - enum_depth(read.get_row());
                current_match = current_match.substr(delete_indent);

                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";

                for (std::size_t del = 0; del < delete_indent; del += 3u) {
                    scopes.pop();
                }
                continue;
            }
            // Ensure : read.get_row() is considered a continuation of the
            // previous item
        }
        while (scopes.size() > 1u) {
            scopes.pop();
        }
        ast.pushback_child(scopes.top());
    }
};

}  // namespace almo