#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace almo {

struct ASTNode {
    // html
    virtual std::string to_html() const = 0;

    // json
    std::string to_json() const;

    // dot
    std::string concatenated_childs_dot() const;

    // properties
    virtual std::map<std::string, std::string> get_properties() const = 0;

    // classname
    virtual std::string get_classname() const = 0;

    // get uuid as string
    std::string get_uuid_str() const;

    // set uuid
    void set_uuid();

    // add child
    void add_child(std::shared_ptr<ASTNode> child);

    // remove child
    void remove_child(std::shared_ptr<ASTNode> child);

    std::vector<std::shared_ptr<ASTNode>> get_childs() const;

    // child's html
    std::string concatenated_childs_html() const;

    // get node's uuid from class name
    std::vector<std::string> nodes_byclass(const std::string &classname) const;

   public:
    std::vector<std::shared_ptr<ASTNode>> childs;

   private:
    int uuid;
};

}  // namespace almo