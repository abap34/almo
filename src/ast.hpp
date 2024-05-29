#pragma once

#include "utils.hpp"
#include "interfaces/ast.hpp"

namespace almo {

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

std::string ASTNode::to_dot() const {
    std::map<std::string, std::string> properties = get_properties();

    std::string node = get_uuid_str();
    std::string label = "";

    for (int i = 1; auto property : properties){
        label += "<f" + std::to_string(i) + "> " + property.first + ": " + escape(property.second) + " | ";
        i++;
    }

    // is Leaf
    if (childs.empty()){
        return node + "[label=\"" + get_classname() + " | " + label + "\", shape=\"record\"]\n";
    }

    // add child node
    std::string childs_dot = "";
    for (auto child : childs){
        childs_dot += child->to_dot();
    }

    // connect child node
    std::string edges = "";
    for (auto child : childs){
        edges += node + ":f" + std::to_string(edges.length()) + " -> " + child->get_uuid_str() + "\n";
    }

    // if this node is root, you must format returned string r as follows :
    //     r = "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + r + "}";
    return node + "[label=\"<f0> " + get_classname() + " | " + label + "\", shape=\"record\"]\n" + childs_dot + edges;
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