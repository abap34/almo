#pragma once

#include <locale>
#include <stack>

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
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<EnumerateBlock>();
    }

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
    std::optional<BlockMatch> match(const Reader &read) const override {
        if (!read.is_line_begin()) return block_match(false);
        const auto line = read.line().text;
        return block_match(std::regex_match(line.begin(), line.end(), rex));
    }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        // BEGIN : rex match
        // END   : NewLine or EOF

        std::stack<std::shared_ptr<EnumerateBlock>> scopes;
        scopes.push(std::make_shared<EnumerateBlock>());

        std::string current_match = R"(\d+\. (.*))";
        // width : 3
        const std::string INDENT = "   ";
        // These patterns change only when entering or leaving a nested scope.
        // Compiling them for each item dominated parsing of long flat lists.
        std::regex current_rex(current_match);
        std::regex nested_rex(INDENT + current_match);
        const bool literal_matching = current_rex.getloc() == std::locale::classic();
        auto item_content = [literal_matching](std::string_view line, std::size_t indent)
            -> std::optional<std::string_view> {
            if (!literal_matching || line.find_first_not_of(' ') != indent)
                return std::nullopt;
            auto end = indent;
            while (end < line.size() && line[end] >= '0' && line[end] <= '9') ++end;
            if (end == indent || line.substr(end, 2) != ". ") return std::nullopt;
            const auto content = line.substr(end + 2);
            if (content.find_first_of("\r\n") != content.npos) return std::nullopt;
            return content;
        };
        auto matches_item = [&](std::string_view line, std::size_t indent,
                                const std::regex& pattern) {
            return item_content(line, indent).has_value() ||
                   std::regex_match(line.begin(), line.end(), pattern);
        };

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
            const auto indent = (scopes.size() - 1u) * 3u;
            if (const auto content = item_content(read.line().text, indent)) {
                text += *content;
            } else {
                // Continuations may contain an unanchored match or CR/LF.
                // Preserve the original regex search for those cases.
                text += [&] {
                    std::smatch sm;
                    // smatch manages iterators into the matched string.
                    const std::string line = read.get_row();
                    std::regex_search(line, sm, current_rex);
                    return sm.format("$1");
                }();
            }

            // within while loop, read changes only here
            read.move_next_line();

            // END
            if (EOFSyntax{}.match(read) || NewLineSyntax{}.match(read)) {
                auto item = std::make_shared<Item>();
                InlineParser::process(text, *item);
                scopes.top()->pushback_child(std::move(item));
                text = "";
                break;
            }
            // same depth
            if (matches_item(read.line().text, indent, current_rex)) {
                auto item = std::make_shared<Item>();
                InlineParser::process(text, *item);
                scopes.top()->pushback_child(std::move(item));
                text = "";
                continue;
            }
            // +3 depth
            if (matches_item(read.line().text, indent + 3u, nested_rex)) {
                auto item = std::make_shared<Item>();
                InlineParser::process(text, *item);
                scopes.top()->pushback_child(std::move(item));
                text = "";

                // nested EnumerateBlock
                auto new_node_ptr = std::make_shared<EnumerateBlock>();
                scopes.top()->pushback_child(new_node_ptr);
                scopes.push(new_node_ptr);

                // update match
                current_match = INDENT + current_match;
                current_rex = std::move(nested_rex);
                nested_rex = std::regex(INDENT + current_match);
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
                    read.warn("Ambiguous enum: indent width must be 3. "
                              "This line is considered as raw text.");
                    return false;
                }()) {
                std::size_t delete_indent =
                    (scopes.size() - 1u) * 3u - enum_depth(read.get_row());
                current_match = current_match.substr(delete_indent);
                current_rex = std::regex(current_match);
                nested_rex = std::regex(INDENT + current_match);

                auto item = std::make_shared<Item>();
                InlineParser::process(text, *item);
                scopes.top()->pushback_child(std::move(item));
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
