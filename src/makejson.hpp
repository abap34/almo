#include<iostream>
#include<string>
#include<utility>
#include"ast.hpp"
#include"json.hpp"

namespace almo {

struct UUID_gen{
    std::string operator()(){
        static int uuid = 0;
        return std::to_string(uuid++);
    }
}uuid;

nlohmann::json dp_on_AST(AST::node_ptr ptr){
    nlohmann::json cur_json;
    if (ptr->type == PlainText){
        cur_json["class"] = "PlainText";
        cur_json["content"] = ptr->content;
    }
    else if (ptr->type == NewLine){
        cur_json["class"] = "NewLine";
        cur_json["content"] = "";
    }
    else if (ptr->type == Url) {
        cur_json["class"] = "Url";
        cur_json["content"] = ptr->content;
    }
    else if (ptr->type == CodeRunner){
        cur_json["class"] = "CodeRunner";
        for (auto [property, name] : ptr->code_runner){
            cur_json[property] = name;
        }
    }
    else if (ptr->type == CodeBlock){
        cur_json["class"] = "CodeBlock";
        std::string codes = "";
        for (AST::node_ptr child : ptr->childs){
            codes += child->content;
            codes += '\n';
        }
        nlohmann::json sub;
        sub["class"] = "PlainText";
        sub["content"] = codes;
        cur_json["content"].push_back(sub);
    }
    else if (ptr->type == MathBlock){
        cur_json["class"] = "MathBlock";
        std::string codes = "";
        for (AST::node_ptr child : ptr->childs){
            codes += child->content;
            codes += '\n';
        }
        nlohmann::json sub;
        sub["class"] = "PlainText";
        sub["content"] = codes;
        cur_json["content"].push_back(sub);
    }


    else {
        if (ptr->type == H1) cur_json["class"] = "H1";
        if (ptr->type == H2) cur_json["class"] = "H2";
        if (ptr->type == H3) cur_json["class"] = "H3";
        if (ptr->type == H4) cur_json["class"] = "H4";
        if (ptr->type == H5) cur_json["class"] = "H5";
        if (ptr->type == H6) cur_json["class"] = "H6";
        if (ptr->type == Block)          cur_json["class"] = "Block";
        if (ptr->type == InlineOverline) cur_json["class"] = "InlineOverline";
        if (ptr->type == InlineStrong)   cur_json["class"] = "InlineStrong";
        if (ptr->type == InlineItalic)   cur_json["class"] = "InlineItalic";
        if (ptr->type == InlineMath)     cur_json["class"] = "InlineMath";
        if (ptr->type == ListBlock)      cur_json["class"] = "ListBlock";
        if (ptr->type == Item)           cur_json["class"] = "Item";
        if (ptr->type == InlineUrl)      cur_json["class"] = "InlineUrl";
        if (ptr->type == InlineImage)    cur_json["class"] = "InlineImage";

        for (AST::node_ptr child : ptr->childs){
            cur_json["content"].push_back(dp_on_AST(child));
        }
    }
    cur_json["uuid"] = uuid();
    return cur_json;
}

nlohmann::json make_json(std::vector<AST::node_ptr> ast) {
    nlohmann::json output_json;
    for (AST::node_ptr ptr : ast){
        output_json.push_back(dp_on_AST(ptr));
    }
    
    return output_json;
}
} // namespace almo
