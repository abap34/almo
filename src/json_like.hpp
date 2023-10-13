#include<vector>
#include<string>
#include<utility>
#include<iostream>
#include<string_view>
#include<ranges>

namespace json_like {

// 文字列が改行を含む場合に、適切にインデントをつけて出力する関数
void output_string_with_indent(std::string s, int current_indent, int indent){
    // s に改行が含まれているか判定
    bool eoln = false;
    for (char c : s){
        if (c == '\n'){
            eoln = true;
            break;
        }
    }
    // s に改行が含まれていないなら、一行しかないのでそのまま出力する
    if (!eoln){
        std::cout << s << '\n';
        return ;
    }
    // s に改行が含まれている場合は、:::や```で囲まれたブロック内で複数行をまとめて保持している
    // これをそのまま出力するとインデントが反映されないので、地道にインデントを追加する
    /*
    [current_indent]"hoge" :
    [current_indent]{
    [current_indent][indent]contents
    [current_indent]}
    */
    std::string str_indent(current_indent,' ');
    std::string str_indent_indent(current_indent+indent,' ');
    std::cout << '\n';
    std::cout << str_indent << "{\n";
    // 直前に改行をしたか
    bool is_pre_eoln = true;
    for (auto c : s){
        // 直前に改行をした場合は、インデントを追加
        if (is_pre_eoln) std::cout << str_indent_indent;
        is_pre_eoln = false;
        std::cout << c;
        if (c == '\n') is_pre_eoln = true;
    }
    std::cout << str_indent << "}\n";
}

struct JsonLike {
    // json とは違い、同じキーを複数持てる。追加順が保たれる。
    std::vector<std::string> keys, values;
    // value が JsonLike を持つ場合だけ別処理、必ず push_childs によって追加される。
    std::vector<JsonLike> jsons;
    // 追加順に、文字列が追加されたか(0)、もしくは JsonLike が追加されたか(1) を保持。
    std::vector<int> key_or_json;
    // map への追加や、既存の json への key を指定した value の追加と同じ記法がで追加できるようにした。
    std::string &operator[](std::string str_idx){
        key_or_json.push_back(0);
        keys.push_back(str_idx);
        values.push_back("");
        return values.back();
    }
    // JsonLike を追加する場合はこちらから
    void push_childs(JsonLike json){
        key_or_json.push_back(1);
        jsons.push_back(json);
    }
    // cout に JsonLike が持っている情報を書き出す
    // indent は一段下がると何スペース分下がるかを表す。 
    // current_indent は何スペース下がった位置が 0 インデントの位置かを表す（再帰するため相対的な行上の位置が必要）。
    void dump(int indent = 2, int current_indent = 0){
        int size = key_or_json.size();
        std::string str_indent(current_indent,' ');
        std::string str_indent_indent(indent+current_indent,' ');
        std::cout << str_indent << "{\n";
        int keys_id = 0, jsons_id = 0;
        for (int i = 0; i < size; i++){
            if (key_or_json[i] == 0){
                // 文字列の場合は、改行のあるなしで出力形式が変わる
                std::cout << str_indent_indent << "\"" << keys[keys_id] << "\": ";
                output_string_with_indent(values[keys_id],current_indent+indent,indent);
                keys_id++;
            }
            else {
                // JsonLike の場合は相対的なインデントを更新して再帰を投げる
                std::cout << str_indent_indent << "\"childs\":\n";
                jsons[jsons_id].dump(indent,current_indent+indent);
                jsons_id++;
            }
        }
        std::cout << str_indent << "}\n";
    }
};

} // namespace json_like