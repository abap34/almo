#pragma once

#include "../new_ast.hpp"
#include "../new_reader.hpp"
#include "../new_syntax.hpp"

namespace almo::feature {

struct RawText : public ASTNode {

    std::string content;
    RawText(std::string _content) : content(_content) {
        set_uuid();
    }

    std::string to_html() const override {
        return content;
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
            {"content", content}
        };
    }
    std::string get_classname() const override {
        return "RawText";
    }
};

} // namespace almo