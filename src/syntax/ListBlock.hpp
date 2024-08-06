#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "EOF.hpp"
#include "Item.hpp"
#include "NewLine.hpp"

namespace almo {

// 箇条書き(番号なし) を表すクラス
struct ListBlock : public ASTNode {
   public:
    ListBlock() { set_uuid(); }

    std::string to_html() const override {
        return "<ul>" + concatenated_childs_html() + "</ul>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "ListBlock"; }
};

struct ListBlockSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        return read.get_row().starts_with("- ");
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : '- .*'
        // END   : NewLine or EOF

        std::stack<std::shared_ptr<ListBlock>> scopes;
        ListBlock root_list;
        scopes.push(std::make_shared<ListBlock>(root_list));

        std::string current_prefix = "- ";
        const std::string INDENT = "  ";

        // if s is considered a list definition, return its depth"
        // otherwise return std::string::npos
        auto list_depth = [](std::string s) -> std::size_t {
            std::size_t pos = s.find_first_not_of(" \n\r\t");
            // s is consist of only space
            if (pos == std::string::npos) return std::string::npos;
            // s is list definition
            if (s.substr(pos).starts_with("- ")) return pos;
            // otherwise
            return std::string::npos;
        };

        std::string text = "";

        while (true) {
            // Invariant : read.is_eof() == false
            //             scopes.size() >= 1
            text += remove_listdef(ltrim(read.get_row()));

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
            if (read.get_row().starts_with(current_prefix)) {
                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";
                continue;
            }
            // +2 depth
            if (read.get_row().starts_with(INDENT + current_prefix)) {
                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";

                // nested ListBlock
                ListBlock new_node;
                auto new_node_ptr = std::make_shared<ListBlock>(new_node);
                scopes.top()->pushback_child(new_node_ptr);
                scopes.push(new_node_ptr);

                // update prefix
                current_prefix = INDENT + current_prefix;
                continue;
            }
            // -2n depth (n >= 1)
            if ([&] {
                    std::size_t depth = list_depth(read.get_row());
                    if (depth == std::string::npos) return false;
                    std::size_t current = (scopes.size() - 1u) * 2u;
                    // depth == current, depth == current + 2 is already
                    // ditected depth has decreased and the difference is even
                    if (depth < current && (current - depth) % 2 == 0)
                        return true;
                    // ambiguous list definition detected
                    std::cerr << "Warning: ambiguous list\n ... \n"
                              << read.near() << "\n^^^ parsing line"
                              << std::endl;
                    std::cerr << "indent width must be 2. this line is "
                                 "considered as raw text."
                              << std::endl;
                    return false;
                }()) {
                std::size_t delete_indent =
                    (scopes.size() - 1u) * 2u - list_depth(read.get_row());
                current_prefix = current_prefix.substr(delete_indent);

                Item item;
                InlineParser::process(text, item);
                scopes.top()->pushback_child(std::make_shared<Item>(item));
                text = "";

                for (std::size_t del = 0; del < delete_indent; del += 2u) {
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