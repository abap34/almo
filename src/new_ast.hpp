#pragma once

#include <string>
#include <vector>
#include <map>

#include "utils.hpp"

namespace almo::feature {

// to_html, to_config するためにグローバルな設定が保存されるグローバル変数.
// 格納されるもの:
// - editor_theme: Ace Editorのテーマ
// - syntax_theme: Highlight.jsのテーマ
std::map<std::string, std::string> meta_data;

// レスポンス時間短縮のため、pyodideが不要なら読み込みをスキップするためのフラグ.
bool loaded_pyodide = false;

std::string pyodide_loader = "<script src=\"https://cdn.jsdelivr.net/pyodide/v0.24.0/full/pyodide.js\"></script>";


struct uuid_gen_ {
    int operator()() {
        static int uuid = 0;
        return uuid++;
    }
} uuid_gen;

struct ASTNode {

    // html
    virtual std::string to_html() const = 0;

    // json (already implemented)
    std::string to_json() const {
        std::map<std::string, std::string> properties = get_properties();

        std::string json = "{";

        // add classname
        json += "\"class\":\"" + escape(get_classname()) + "\",";

        // add uuid
        json += "\"uuid\":\"" + escape(get_uuid_str()) + "\",";

        // add other properties
        for (auto property : properties){
            json += "\"" + property.first + "\":\"" + escape(property.second) + "\",";
        }

        // has childs
        if (!childs.empty()){
            json += "\"childs\":[";
            for (auto child : childs){
                json += child->to_json();
                json += ',';
            }
            // has 1 or more childs, so the last char is ,
            // delete the ,
            json.pop_back();
            json += "],";
        }

        // the last char is , regardless of throughing `if`
        // delete the ,
        json.pop_back();
        json += '}';
        return json;
    }

    // properties
    virtual std::map<std::string, std::string> get_properties() const = 0;

    // classname
    virtual std::string get_classname() const = 0;

    // get uuid as string
    std::string get_uuid_str() const {
        return std::to_string(uuid);
    }

    // set uuid
    void set_uuid(){
        uuid = uuid_gen();
    }

    // add child
    void add_child(std::shared_ptr<ASTNode> child){
        childs.push_back(child);
    }

    std::string concatenated_childs_html() const {
        std::string ret = "";
        for (auto child : childs){
            ret += child->to_html();
        }
        return ret;
    }

  protected:
    std::vector<std::shared_ptr<ASTNode>> childs;
  private:
    int uuid;
};

} // namespace almo