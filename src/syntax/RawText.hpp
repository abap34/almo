#pragma once

#include <limits>

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct RawText : public ASTNode {
   private:
    std::string content;

   public:
    RawText(std::string _content) : content(_content) { set_uuid(); }

    std::string to_html() const override { return content; }

    std::map<std::string, std::string> get_properties() const override {
        return {{"content", content}};
    }
    std::string get_classname() const override { return "RawText"; }
};

// Raw text is emitted directly when no inline syntax matches.
}  // namespace almo
