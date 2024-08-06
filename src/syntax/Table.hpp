#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "EOF.hpp"
#include "Markdown.hpp"
#include "NewLine.hpp"

namespace almo {

struct Table : public ASTNode {
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
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (std::regex_match(rtrim(read.get_row()),
                             std::regex(R"((\|[^\|]+).+\|)")))
            return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : R"((\|[^\|]+).+\|)"
        // END   : EOF or NewLine
        const std::regex each_col_regex(R"(\|[^\|]+)");

        int n_col = 0;
        std::vector<std::string> col_names;
        std::smatch sm;

        std::string line = read.get_row();

        while (std::regex_search(line, sm, each_col_regex)) {
            col_names.push_back(sm[0].str().substr(1));
            line = sm.suffix();
            n_col++;
        }

        // 0 --> left (default), 1 --> center, 2 --> right
        std::vector<int> col_format;
        std::regex Lrex(R"(\|\s*:-+\s*)");
        std::regex Crex(R"(\|\s*:-+:\s*)");
        std::regex Rrex(R"(\|\s*-+:\s*)");

        read.move_next_line();

        line = read.get_row();

        while (std::regex_search(line, sm, each_col_regex)) {
            if (std::regex_match(sm[0].str(), Lrex)) {
                col_format.emplace_back(0);
            } else if (std::regex_match(sm[0].str(), Crex)) {
                col_format.emplace_back(1);
            } else if (std::regex_match(sm[0].str(), Rrex)) {
                col_format.emplace_back(2);
            } else {
                col_format.emplace_back(0);
            }

            line = sm.suffix();
        }

        if (col_names.size() != col_format.size()) {
            throw SyntaxError("Number of columns of Table must be same");
        }

        read.move_next_line();

        int n_row = 0;
        std::vector<std::string> table;

        while (true) {
            if (EOFSyntax{}(read) || NewLineSyntax{}(read)) break;
            n_row++;
            line = read.get_row();
            while (std::regex_search(line, sm, each_col_regex)) {
                table.push_back(sm[0].str().substr(1));
                line = sm.suffix();
            }
            read.move_next_line();
        }

        std::vector<std::shared_ptr<Markdown>> columns_markdowns;

        for (int i = 0; i < n_col; i++) {
            Markdown inner_md;
            InlineParser::process(col_names[i], inner_md);
            columns_markdowns.push_back(std::make_shared<Markdown>(inner_md));
        }

        Table node(columns_markdowns, n_row, n_col, col_names, col_format);

        for (auto cell : table) {
            Markdown inner_md;
            InlineParser::process(cell, inner_md);
            node.pushback_child(std::make_shared<Markdown>(inner_md));
        }

        ast.pushback_child(std::make_shared<Table>(node));
    }
};

}  // namespace almo