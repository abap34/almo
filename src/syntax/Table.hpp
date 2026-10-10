#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "EOF.hpp"
#include "Markdown.hpp"
#include "NewLine.hpp"

namespace almo {

namespace detail {

// Preserve the former \|[^\|]+ matches: empty cells are skipped and text
// before the first pipe is ignored. Views avoid copying the remaining row for
// every cell, which made wide rows particularly expensive.
inline std::vector<std::string_view> table_cells(std::string_view line) {
    std::vector<std::string_view> cells;
    auto pipe = line.find('|');
    while (pipe != line.npos) {
        const auto begin = pipe + 1;
        const auto next = line.find('|', begin);
        const auto end = next == line.npos ? line.size() : next;
        if (begin != end) cells.push_back(line.substr(begin, end - begin));
        pipe = next;
    }
    return cells;
}

}  // namespace detail

struct Table : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        auto copy = clone_as<Table>();
        copy->columns.clear();
        for (const auto& column : columns)
            copy->columns.push_back(std::static_pointer_cast<Markdown>(column->clone()));
        return copy;
    }

   private:
    std::vector<std::shared_ptr<Markdown>> columns;
    int n_row;
    int n_col;

    // 各列の名前をインライン記法のパースをしていない素の文字列として持つ.
    std::vector<std::string> col_names;
    // 各列について、左寄せ(0), 中央寄せ(1), 右寄せ(2) のいずれかを指定する.
    std::vector<int> col_format;

   public:
    Table(std::vector<std::shared_ptr<Markdown>> columns, int n_row, int n_col,
          std::vector<std::string> col_names, std::vector<int> col_format)
        : columns(columns),
          n_row(n_row),
          n_col(n_col),
          col_names(col_names),
          col_format(col_format) {
        set_uuid();
    }

    // テーブル用の特別な to_html. テーブルの to_html
    // でしか呼ばれないので引数が違ってもOK.
    std::string to_html(std::vector<std::string> headers_html,
                        std::vector<std::string> childs_html) const {
        std::string output = "<table>\n";
        output += "<thead>\n";
        output += "<tr>\n";
        for (int i = 0; i < n_col; i++) {
            std::string align = col_format[i] == 0   ? "left"
                                : col_format[i] == 1 ? "center"
                                                     : "right";
            output +=
                "<th align=\"" + align + "\">" + headers_html[i] + "</th>\n";
        }
        output += "</tr>\n";
        output += "</thead>\n";
        output += "<tbody>\n";
        for (int i = 0; i < n_row; i++) {
            output += "<tr>\n";
            for (int j = 0; j < n_col; j++) {
                std::string align = col_format[j] == 0   ? "left"
                                    : col_format[j] == 1 ? "center"
                                                         : "right";
                output += "<td align=\"" + align + "\">" +
                          childs_html[i * n_col + j] + "</td>\n";
            }
            output += "</tr>\n";
        }
        output += "</tbody>\n";
        output += "</table>\n";
        return output;
    }

    std::string to_html() const override {
        std::vector<std::string> columns_html;
        for (auto child : columns) {
            columns_html.push_back(child->to_html());
        }

        std::vector<std::string> contents_html;

        for (auto child : childs) {
            contents_html.push_back(child->to_html());
        }

        return to_html(columns_html, contents_html);
    }

    std::map<std::string, std::string> get_properties() const override {
        std::string col_format_str = "";
        for (int i = 0; i < n_col; i++) {
            std::string align = col_format[i] == 0   ? "l"
                                : col_format[i] == 1 ? "c"
                                                     : "r";
            col_format_str += align;
        }

        std::string col_names_str = "[";

        for (int i = 0; i < n_col; i++) {
            col_names_str += "\"" + col_names[i] + "\"";
            if (i != n_col - 1) {
                col_names_str += ", ";
            }
        }

        col_names_str += "]";

        return {{"n_row", std::to_string(n_row)},
                {"n_col", std::to_string(n_col)},
                {"col_format", col_format_str},
                {"col_names", col_names_str}};
    }
    std::string get_classname() const override { return "Table"; }
};

struct TableSyntax : public BlockSyntax {
    std::optional<BlockMatch> match(const Reader &read) const override {
        if (!read.is_line_begin()) return block_match(false);
        static const std::regex pattern(R"((\|[^\|]+).+\|)");
        const auto line = detail::rtrim_view(read.line().text);
        if (std::regex_match(line.begin(), line.end(), pattern))
            return block_match(true);
        return block_match(false);
    }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        // BEGIN : R"((\|[^\|]+).+\|)"
        // END   : EOF or NewLine
        std::vector<std::string> col_names;
        for (const auto cell : detail::table_cells(read.line().text))
            col_names.emplace_back(cell);
        const int n_col = static_cast<int>(col_names.size());

        // 0 --> left (default), 1 --> center, 2 --> right
        std::vector<int> col_format;
        static const std::regex Lrex(R"(\s*:-+\s*)");
        static const std::regex Crex(R"(\s*:-+:\s*)");
        static const std::regex Rrex(R"(\s*-+:\s*)");

        read.move_next_line();

        if (read.is_eof())
            throw SyntaxError("Table alignment row is missing");
        for (const auto cell : detail::table_cells(read.line().text)) {
            if (std::regex_match(cell.begin(), cell.end(), Lrex)) {
                col_format.emplace_back(0);
            } else if (std::regex_match(cell.begin(), cell.end(), Crex)) {
                col_format.emplace_back(1);
            } else if (std::regex_match(cell.begin(), cell.end(), Rrex)) {
                col_format.emplace_back(2);
            } else {
                col_format.emplace_back(0);
            }
        }

        if (col_names.size() != col_format.size()) {
            throw SyntaxError("Number of columns of Table must be same");
        }

        read.move_next_line();

        int n_row = 0;
        std::vector<std::string_view> table;

        while (true) {
            if (EOFSyntax{}.match(read) || NewLineSyntax{}.match(read)) break;
            n_row++;
            const auto cells = detail::table_cells(read.line().text);
            table.insert(table.end(), cells.begin(), cells.end());
            if (table.size() != static_cast<std::size_t>(n_row * n_col))
                throw SyntaxError("Number of columns of Table must be same");
            read.move_next_line();
        }

        std::vector<std::shared_ptr<Markdown>> columns_markdowns;

        for (int i = 0; i < n_col; i++) {
            Markdown inner_md;
            InlineParser::process(col_names[i], inner_md);
            columns_markdowns.push_back(std::make_shared<Markdown>(inner_md));
        }

        Table node(std::move(columns_markdowns), n_row, n_col,
                   std::move(col_names), std::move(col_format));

        for (auto cell : table) {
            Markdown inner_md;
            InlineParser::process(std::string(cell), inner_md);
            node.pushback_child(std::make_shared<Markdown>(inner_md));
        }

        ast.pushback_child(std::make_shared<Table>(node));
    }
};

}  // namespace almo
