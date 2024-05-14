#pragma once

#include "new_ast.hpp"
#include "new_reader.hpp"

namespace almo::feature {

struct BlockSyntax {
    virtual bool operator()(Reader &read) const = 0;
    virtual void operator()(Reader &read, ASTNode &ast) const = 0;
};

struct InlineSyntax {
    virtual bool operator()(const std::string &str) const = 0;
    virtual void operator()(const std::string &str, ASTNode &ast) const = 0;
};

} // namespace almo