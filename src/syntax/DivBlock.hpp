#pragma once

#include <stack>

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "Markdown.hpp"

namespace almo {

struct DivBlock : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<DivBlock>();
    }

   private:
    std::string div_class;

   public:
    DivBlock(std::string div_class) : div_class(std::move(div_class)) { set_uuid(); }

    std::string to_html() const override {
        return "<div class=\"" + div_class + "\">" +
               concatenated_childs_html() + "</div>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
            {"div_class", div_class},
        };
    }
    std::string get_classname() const override { return "DivBlock"; }
};

struct DivBlockSyntax : public BlockSyntax {
    std::optional<BlockMatch> match(const Reader &read) const override {
        if (!read.is_line_begin()) return block_match(false);
        const auto line = read.line().text;
        if (!line.starts_with(":::")) return block_match(false);
        if (detail::rtrim_view(line) != ":::") return block_match(true);
        return block_match(false);
    }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        std::stack<std::shared_ptr<DivBlock>> scopes;
        std::string div_class = read.get_row().substr(3);

        scopes.push(std::make_shared<DivBlock>(std::move(div_class)));

        read.move_next_line();
        std::vector<SourceLine> text;

        while (!read.is_eof()) {
            // Invariant : scopes.size() >= 1
            if (rtrim(read.get_row()) == ":::") {
                auto inner_md_ptr = std::make_shared<Markdown>();
                auto nested = read.nested(text);
                detail::parse_into(nested, *inner_md_ptr);
                scopes.top()->pushback_child(std::move(inner_md_ptr));

                text = {};
                read.move_next_line();

                if (scopes.size() == 1u) {
                    ast.pushback_child(scopes.top());
                    scopes.pop();
                    break;
                }
                scopes.pop();
                continue;
            }
            if (read.get_row().starts_with(":::")) {
                auto inner_md_ptr = std::make_shared<Markdown>();
                auto nested = read.nested(text);
                detail::parse_into(nested, *inner_md_ptr);
                scopes.top()->pushback_child(std::move(inner_md_ptr));

                auto new_node_ptr = std::make_shared<DivBlock>(read.get_row().substr(3));
                scopes.top()->pushback_child(new_node_ptr);

                scopes.push(new_node_ptr);

                text = {};
                read.move_next_line();
                continue;
            }
            text.push_back(read.line());
            read.move_next_line();
        }
        // scopes.empty() iff completed DivBlock
        while (!scopes.empty()) {
            auto inner_md_ptr = std::make_shared<Markdown>();
            auto nested = read.nested(text);
            detail::parse_into(nested, *inner_md_ptr);
            scopes.top()->pushback_child(std::move(inner_md_ptr));

            text = {};

            if (scopes.size() == 1u) {
                ast.pushback_child(scopes.top());
                scopes.pop();
                break;
            }
            scopes.pop();
            continue;
        }
        assert(scopes.empty());
    }
};

}  // namespace almo
