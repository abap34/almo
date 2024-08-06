#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct EOFSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override { return read.is_eof(); }
    void operator()(Reader &read, ASTNode &ast) const override {
        read.read_eof();
    }
};

}  // namespace almo