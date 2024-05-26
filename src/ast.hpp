#pragma once

#include "utils.hpp"
#include "interfaces/ast.hpp"

namespace almo {

// レスポンス時間短縮のため、pyodideが不要なら読み込みをスキップするためのフラグ.
bool loaded_pyodide = false;

std::string pyodide_loader = "<script src=\"https://cdn.jsdelivr.net/pyodide/v0.24.0/full/pyodide.js\"></script>";

struct uuid_gen_ {
    int operator()(){
        static int uuid = 0;
        return uuid++;
    }
} uuid_gen;

std::string ASTNode::to_json() const {
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

std::string ASTNode::get_uuid_str() const {
    return std::to_string(uuid);
}

void ASTNode::set_uuid(){
    uuid = uuid_gen();
}

void ASTNode::add_child(std::shared_ptr<ASTNode> child){
    childs.push_back(child);
}

std::string ASTNode::concatenated_childs_html() const {
    std::string ret = "";
    for (auto child : childs){
        ret += child->to_html();
    }
    return ret;
}

} // namespace almo