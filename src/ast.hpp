#pragma once

#include "interfaces/ast.hpp"
#include "utils.hpp"

namespace almo {

struct uuid_gen_ {
    int operator()() {
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
    for (auto property : properties) {
        json +=
            "\"" + property.first + "\":\"" + escape(property.second) + "\",";
    }

    // has childs
    if (!childs.empty()) {
        json += "\"childs\":[";
        for (auto child : childs) {
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

std::string ASTNode::concatenated_childs_dot() const {
    std::map<std::string, std::string> properties = get_properties();

    std::string node = get_uuid_str();
    std::string label = "";

    for (int i = 1; auto property : properties) {
        label += "<f" + std::to_string(i) + "> " + property.first + ": " +
                 escape(property.second) + " | ";
        i++;
    }

    // is Leaf
    if (childs.empty()) {
        return node + "[label=\"" + get_classname() + " | " + label +
               "\", shape=\"record\"]\n";
    }

    // add child node
    std::string childs_dot = "";
    for (auto child : childs) {
        childs_dot += child->concatenated_childs_dot();
    }

    // connect child node
    std::string edges = "";
    for (auto child : childs) {
        edges += node + ":f" + std::to_string(edges.length()) + " -> " +
                 child->get_uuid_str() + "\n";
    }

    // if this node is root, you must format returned string r as follows :
    //     r = "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + r + "}";
    return node + "[label=\"<f0> " + get_classname() + " | " + label +
           "\", shape=\"record\"]\n" + childs_dot + edges;
}

std::string ASTNode::get_uuid_str() const { return std::to_string(uuid); }

void ASTNode::set_uuid() { uuid = uuid_gen(); }

void ASTNode::pushback_child(std::shared_ptr<ASTNode> child) {
    childs.push_back(child);
}

void ASTNode::remove_child(std::shared_ptr<ASTNode> child) {
    for (auto it = childs.begin(); it != childs.end(); it++) {
        if (*it == child) {
            childs.erase(it);
            return;
        }

        (*it)->remove_child(child);
    }
}

std::string ASTNode::concatenated_childs_html() const {
    std::string ret = "";
    for (auto child : childs) {
        ret += child->to_html();
    }
    return ret;
}

std::vector<std::shared_ptr<ASTNode>> ASTNode::get_childs() const {
    return childs;
}

std::vector<std::shared_ptr<ASTNode>> ASTNode::nodes_byclass(
    const std::string &classname) const {
    std::vector<std::shared_ptr<ASTNode>> ret;
    if (get_classname() == classname) {
        ret.push_back(const_cast<ASTNode *>(this)->shared_from_this());
    }
    for (auto child : childs) {
        std::vector<std::shared_ptr<ASTNode>> childs_ret =
            child->nodes_byclass(classname);
        for (auto child_ret : childs_ret) {
            ret.push_back(child_ret);
        }
    }
    return ret;
}

void ASTNode::move_node(std::shared_ptr<ASTNode> node,
                        std::shared_ptr<ASTNode> new_parent) {
    remove_child(node);
    new_parent->pushback_child(node);
}

}  // namespace almo