#pragma once

#include <vector>
#include <string>
#include <cassert>

namespace almo::feature {

struct Reader {
    std::vector<std::string> lines;
    int row, col;
    Reader (const std::vector<std::string> &_lines) : lines(_lines), row(0), col(0) {}
    bool is_line_begin() const {
        return col == 0;
    }
    bool is_line_end() const {
        return col == (int)lines[row].size();
    }
    bool is_whole_end() const {
        return row == (int)lines.size();
    }
    std::string whole_row() const {
        return lines[row];
    }
    std::string restline() const {
        return lines[row].substr(col);
    }
    void move_next_line(){
        row++;
        col = 0;
    }
    void eoln(){
        assert(is_line_end());
        row++;
        col = 0;
    }
    // bool starts_with(const char *_x){}
};

} // namespace almo