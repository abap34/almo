#include<vector>
#include<string>
#include<utility>
#include<iostream>
#include<string_view>
#include<ranges>

namespace json_like {

void output_string_with_indent(std::string s, int current_indent, int indent){
    bool eoln = false;
    for (char c : s){
        if (c == '\n'){
            eoln = true;
            break;
        }
    }
    if (!eoln){
        std::cout << s << '\n';
        return ;
    }
    std::string str_indent(current_indent,' ');
    std::string str_indent_indent(current_indent+indent,' ');
    std::cout << '\n';
    std::cout << str_indent << "{\n";
    bool is_pre_eoln = true;
    for (auto c : s){
        if (is_pre_eoln) std::cout << str_indent_indent;
        is_pre_eoln = false;
        std::cout << c;
        if (c == '\n') is_pre_eoln = true;
    }
    std::cout << str_indent << "}\n";
}

struct JsonLike {
    std::vector<std::string> keys, values;
    std::vector<JsonLike> jsons;
    std::vector<int> key_or_json;
    std::string &operator[](std::string str_idx){
        key_or_json.push_back(0);
        keys.push_back(str_idx);
        values.push_back("");
        return values.back();
    }
    void push_childs(JsonLike json){
        key_or_json.push_back(1);
        jsons.push_back(json);
    }
    void dump(int indent = 2, int current_indent = 0){
        int size = key_or_json.size();
        std::string str_indent(current_indent,' ');
        std::string str_indent_indent(indent+current_indent,' ');
        std::cout << str_indent << "{\n";
        int keys_id = 0, jsons_id = 0;
        for (int i = 0; i < size; i++){
            if (key_or_json[i] == 0){
                std::cout << str_indent_indent << "\"" << keys[keys_id] << "\": ";
                output_string_with_indent(values[keys_id],current_indent+indent,indent);
                keys_id++;
            }
            else {
                std::cout << str_indent_indent << "\"childs\":\n";
                jsons[jsons_id].dump(indent,current_indent+indent);
                jsons_id++;
            }
        }
        std::cout << str_indent << "}\n";
    }
};

} // namespace json_like