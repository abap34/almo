#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace almo {

struct Reader {
    std::vector<std::string> lines;
    int row, col;
    // already read eof
    bool eof_read_flg;

    Reader(const std::vector<std::string> &_lines)
        : lines(_lines), row(0), col(0), eof_read_flg(false) {}
    bool is_line_begin() const { return col == 0; }
    bool is_line_end() const { return col == (int)lines[row].size(); }
    bool is_eof() const { return row == (int)lines.size(); }
    std::string get_row() const { return lines[row]; }
    std::string get_rest_row() const { return lines[row].substr(col); }
    void move_next_line() {
        assert(row < (int)(lines.size()));
        row++;
        col = 0;
    }
    void move_next_char(int n) {
        assert(0 <= n && n <= (int)(lines[row].size()) - col);
        col += n;
    }
    void read_eof() {
        assert(!eof_read_flg);
        eof_read_flg = true;
    }


    // get near text from current position(row, col)
    std::string near() {
        std::string ret = "";
        if (row > 0) {
            ret += lines[row - 1] + "\n";
        }
        ret += lines[row];
        return ret;
    }
};

}  // namespace almo