#pragma once

#include "interfaces/ast.hpp"

namespace almo {

class IncrementalParser;

// Borrowed, read-only access. Invalidated by an update or parser destruction.
class ASTView {
public:
    std::string get_classname() const { return node_->get_classname(); }
    std::string get_uuid_str() const { return node_->get_uuid_str(); }
    std::map<std::string, std::string> get_properties() const { return node_->get_properties(); }
    std::string to_html() const { return node_->to_html(); }
    std::string to_json() const { return node_->to_json(); }
    std::vector<ASTView> get_childs() const {
        std::vector<ASTView> children;
        children.reserve(node_->childs.size());
        for (const auto& child : node_->childs) children.push_back(ASTView(*child));
        return children;
    }

private:
    explicit ASTView(const ASTNode& node) : node_(&node) {}
    const ASTNode* node_;
    friend class IncrementalParser;
};

}  // namespace almo
