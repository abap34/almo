#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

// 箇条書きの要素を表すクラス
struct Item : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<Item>();
    }

    Item() { set_uuid(); }

    std::string to_html() const override {
        return "<li>" + concatenated_childs_html() + "</li>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "Item"; }
};

}  // namespace almo