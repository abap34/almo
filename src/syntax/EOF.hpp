#pragma once

#include "../new_ast.hpp"
#include "../new_reader.hpp"
#include "../new_syntax.hpp"

namespace almo::feature {

struct EOF_syntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        return read.is_whole_end();
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // nothing
    }
};

} // namespace almo