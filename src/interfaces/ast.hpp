#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace almo {

struct ASTNode : public std::enable_shared_from_this<ASTNode> {
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
    void pushback_child(std::shared_ptr<ASTNode> child);

    // remove child
    void remove_child(std::shared_ptr<ASTNode> child);

    std::vector<std::shared_ptr<ASTNode>> get_childs() const;

    // child's html
    std::string concatenated_childs_html() const;

    // get node's uuid from class name
    std::vector<std::shared_ptr<ASTNode>> nodes_byclass(
        const std::string &classname) const;

    void move_node(std::shared_ptr<ASTNode> node,
                   std::shared_ptr<ASTNode> new_parent);

   public:
    std::vector<std::shared_ptr<ASTNode>> childs;

   private:
    int uuid;
};

}  // namespace almo