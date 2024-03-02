#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream> 
#include <glob.h>
#include <regex>
#include "ast.hpp"
#include "utils.hpp"

namespace almo {
    std::string LIGHT_THEME =
            #include "light.css"
        ;
    std::string DARK_THEME =
            #include "dark.css"
        ;

    std::string RUNNER =
            #include "runner.js"
        ;

    std::string TEMPLATE =
            #include "template.html"
        ;

    std::string SIDEBAR_BULDER =
            #include "sidebar.js"
        ;

    
    std::string load_html_template(std::string html_path, std::string css_setting) {
        std::string html;

        if (html_path == "__default__") {
            html = TEMPLATE;
        }
        else {
            html = join(read_file(html_path), "\n");
        }

        std::string result;
        if (css_setting == "light") {
            std::string css = "<style>" + LIGHT_THEME + "</style>";
            result = std::regex_replace(html, std::regex("\\{\\{style\\}\\}"), css);
        }
        else if (css_setting == "dark") {
            std::string css = "<style>" + DARK_THEME + "</style>";
            result = std::regex_replace(html, std::regex("\\{\\{style\\}\\}"), css);
        }
        else if (css_setting.ends_with(".css")) {
            std::string css = "<style>" + join(read_file(css_setting), "\n") + "</style>";

            result = std::regex_replace(html, std::regex("\\{\\{style\\}\\}"), css);
        }
        else {
            throw InvalidCommandLineArgumentsError("不正なCSSの設定です。 `light`, `dark` もしくは `.css` で終了するファイル名を指定してください。");
        }

        std::string runner = "<script>" + RUNNER + "</script>";
        std::string sidebar_builder = "<script>" + SIDEBAR_BULDER + "</script>";

        // runner を挿入
        result = std::regex_replace(result, std::regex("\\{\\{runner\\}\\}"), runner);
        // sidebar_builder を挿入
        result = std::regex_replace(result, std::regex("\\{\\{sidebar_builder\\}\\}"), sidebar_builder);


        return result;
    }

    std::string replace_template(std::string html_template, std::map<std::string, std::string> meta_data, std::string content) {
        std::string output_html = html_template;
    
        for (auto [key, value] : meta_data) {
            std::string replace_key = "\\{\\{" + key + "\\}\\}";
            output_html = std::regex_replace(output_html, std::regex(replace_key), value);
        }

        std::string syntax_theme = meta_data["syntax_theme"];

        output_html = std::regex_replace(output_html, std::regex("\\{\\{syntax_theme\\}\\}"), syntax_theme);
        output_html = std::regex_replace(output_html, std::regex("\\{\\{contents\\}\\}"), content);

        return output_html;
    }

    std::string render(Block ast, std::map<std::string, std::string> meta_data) {
        
        std::string html_template = load_html_template(meta_data["template_file"], meta_data["css_setting"]);

        std::string content = ast.render();
        
        std::string output_html = replace_template(html_template, meta_data, content);

        return output_html;
    }

}
