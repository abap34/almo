#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct EOFSyntax : public BlockSyntax {
    std::optional<BlockMatch> match(const Reader &read) const override { return block_match(read.is_eof()); }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        read.read_eof();
    }
};

}  // namespace almo