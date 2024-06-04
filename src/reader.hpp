#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <map>
#include <iostream>

namespace almo {

struct Reader {
    std::vector<std::string> lines;
    std::map<std::string, std::string> &meta_data;
    int row, col;
    // already read eof
    bool eof_read_flg;

    Reader (const std::vector<std::string> &_lines,
            std::map<std::string, std::string> &_meta_data)
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
    std::string get_rest_row() const {
        return lines[row].substr(col);
    }
    void move_next_line(){
        row++;
        col = 0;
    }
    void move_next_char(int n){
        assert(0 <= n && n <= (int)(lines[row].size()) - col);
        col += n;
    }
    void read_eof(){
        assert(!eof_read_flg);
        eof_read_flg = true;
    }
    std::string get_meta_data(std::string key) const {
        // if meta_data do not contains key, throw out_of_range exception
        return meta_data.at(key);
    }
    void set_meta_data(std::string key, std::string value){
        // for required_pyodide (stop warning)
        if (key == "required_pyodide" && value == "true"){
            assert(meta_data.contains("required_pyodide"));
            if (meta_data["required_pyodide"] == "false"){
                meta_data[key] = value;
            }
            return ;
        }
        if (meta_data.contains(key)){
            if (meta_data[key] == value) return ;
            std::cerr << "Warning : key is duplicating. value is overwritten." << std::endl;
            std::cerr << "duplicated key : " << key << std::endl;
            std::cerr << "previous value : " << meta_data[key] << std::endl;
            std::cerr << "     new value : " << value << std::endl;
        }
        meta_data[key] = value;
    }
    // get near text from current position(row, col)
    std::string near(){
        std::string ret = "";
        if (row > 0){
            ret += lines[row-1] + "\n";
        }
        ret += lines[row];
        return ret;
    }
};

} // namespace almo