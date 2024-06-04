#pragma once

#include "../reader.hpp"
#include "ast.hpp"
#include <regex>
#include <limits>

namespace almo {

struct BlockSyntax {
    // determine if it matches as the syntax
    virtual bool operator()(Reader &read) const = 0;

    // update read and ast
    // assume that it matches as the syntax
    virtual void operator()(Reader &read, ASTNode &ast) const = 0;
};

struct InlineSyntax {
    // determine if the string matches as the syntax
    // if so, calculate minimum position of the matched location
    // example :
    //   rex = $(.*)$;
    //   str = "ab$c+d$e";
    //   ---> substring located at [2,7) detected : '$c+d$'
    //   ---> return 2
    // ( ---> captured substring is located at [3,6) : 'c+d' )
    virtual int operator()(const std::string &str) const = 0;

    // update read and ast
    // assume that the string matches as the syntax
    virtual void operator()(const std::string &str, ASTNode &ast) const = 0;
};

} // namespace almo