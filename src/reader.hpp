#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <map>

namespace almo {

struct Reader {
    std::vector<std::string> lines;
    std::map<std::string, std::string> meta_data;
    int row, col;
    // already read eof
    bool eof_read_flg;
    Reader (const std::vector<std::string> &_lines,
            const std::map<std::string, std::string> &_meta_data)
            : lines(_lines), meta_data(_meta_data), row(0), col(0), eof_read_flg(false) {}
    bool is_line_begin() const {
        return col == 0;
    }
    bool is_line_end() const {
        return col == (int)lines[row].size();
    }
    bool is_eof() const {
        return row == (int)lines.size();
    }
    std::string get_row() const {
        return lines[row];
    }
    void move_next_line(){
        row++;
        col = 0;
    }
    void read_eof(){
        assert(!eof_read_flg);
        eof_read_flg = true;
    }
    // TODO: implement method for getting meta_data
};

} // namespace almo