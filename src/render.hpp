#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream> 
#include <glob.h>
#include "json.hpp"

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
    std::string read_file(const std::string& path) {
        std::ifstream input_file(path);

        if (!input_file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::stringstream buffer;
        buffer << input_file.rdbuf();

        input_file.close();

        return buffer.str();
    }


    std::vector<std::string> glob(const std::string& pattern) {
        std::vector<std::string> result;
        glob_t globResult;

        if (glob(pattern.c_str(), GLOB_TILDE, nullptr, &globResult) == 0) {
            for (size_t i = 0; i < globResult.gl_pathc; ++i) {
                result.push_back(globResult.gl_pathv[i]);
            }
        }

        globfree(&globResult);
        return result;
    }

    std::string load_html_template(std::string theme, std::string custom_css_path) {
        std::string result;
        if (theme == "light") {
            // replace {{style}} in TEMPLATE
            result = std::regex_replace(TEMPLATE, std::regex("\\{\\{style\\}\\}"), LIGHT_THEME);
        }
        else if (theme == "dark") {
            result = std::regex_replace(TEMPLATE, std::regex("\\{\\{style\\}\\}"), DARK_THEME);
        } else {
            std::cerr << "Invalid theme: " << theme << ", available themes are 'dark' and 'light'" << std::endl;
            exit(1);
        }

        if (custom_css_path != "") {
            std::string custom_css = read_file(custom_css_path);
            result = std::regex_replace(TEMPLATE, std::regex("\\{\\{style\\}\\}"), custom_css);
        }
        

        // find </html> and put runner.js before it
        std::string runner = RUNNER;
        std::string sidebar_builder = SIDEBAR_BULDER;
        std::string::size_type pos = result.find("</head>");

        if (pos != std::string::npos) {
            result.insert(pos, runner);
        }
        else {
            std::cerr << "Invalid html template" << std::endl;
            exit(1);
        }

        pos = result.find("</html>");

        if (pos != std::string::npos) {
            result.insert(pos, sidebar_builder);
        }
        else {
            std::cerr << "Invalid html template" << std::endl;
            exit(1);
        }

        return result;
    }

    std::string replace_template(std::string html_template, nlohmann::json json_meta_data, std::string contents) {
        std::string output_html = html_template;
        // get all key
        std::vector<std::string> keys;
        for (auto it = json_meta_data.begin(); it != json_meta_data.end(); ++it) {
            keys.push_back(it.key());
        }

        for (std::string key : keys) {
            std::string replace_key = "\\{\\{" + key + "\\}\\}";
            std::string replace_value = json_meta_data[key];
            output_html = std::regex_replace(output_html, std::regex(replace_key), replace_value);
        }

        std::string syntax_theme;
        if (json_meta_data["theme"] == "dark") {
            syntax_theme = "monokai-sublime.min";
        }
        else if (json_meta_data["theme"] == "light") {
            syntax_theme = "github.min";
        }
        else {
            std::cerr << "Invalid theme: " << json_meta_data["theme"] << ", available themes are 'dark' and 'light'" << std::endl;
            exit(1);
        }

        output_html = std::regex_replace(output_html, std::regex("\\{\\{syntax_theme\\}\\}"), syntax_theme);
        output_html = std::regex_replace(output_html, std::regex("\\{\\{contents\\}\\}"), contents);

        return output_html;
    }


    std::string build_page_content_script(std::string type, std::string id, std::string content) {
        std::string script = "<script>"
            "page_contents.push({ "
            "    \"type\":\"" + type + "\", "
            "    \"id\":\"" + id + "\", "
            "    \"content\":\"" + content + "\" "
            "})"
            "</script> \n";
        return script;
    }

    std::string render_h1(nlohmann::json j, std::string content) {
        std::string uuid = j["uuid"];
        std::string add_page_content = build_page_content_script("H1", uuid, content);
        std::string output = "<h1 id=\"" + uuid + "\">" + content + "</h1>";
        return add_page_content + output;
    }

    std::string render_h2(nlohmann::json j, std::string content) {
        std::string uuid = j["uuid"];
        std::string add_page_content = build_page_content_script("H2", uuid, content);
        std::string output = "<h2 id=\"" + uuid + "\">" + content + "</h2>";
        return add_page_content + output;
    }

    std::string render_h3(nlohmann::json j, std::string content) {
        std::string uuid = j["uuid"];
        std::string add_page_content = build_page_content_script("H3", uuid, content);
        std::string output = "<h3 id=\"" + uuid + "\">" + content + "</h3>";
        return add_page_content + output;
    }

    std::string render_h4(nlohmann::json j, std::string content) {
        std::string uuid = j["uuid"];
        std::string add_page_content = build_page_content_script("H4", uuid, content);
        std::string output = "<h4 id=\"" + uuid + "\">" + content + "</h4>";
        return add_page_content + output;
    }

    std::string render_h5(nlohmann::json j, std::string content) {
        std::string uuid = j["uuid"];
        std::string add_page_content = build_page_content_script("H5", uuid, content);
        std::string output = "<h5 id=\"" + uuid + "\">" + content + "</h5>";
        return add_page_content + output;
    }

    std::string render_h6(nlohmann::json j, std::string content) {
        std::string uuid = j["uuid"];
        std::string add_page_content = build_page_content_script("H6", uuid, content);
        std::string output = "<h6 id=\"" + uuid + "\">" + content + "</h6>";
        return add_page_content + output;
    }

    std::string render_strong(nlohmann::json j, std::string content) {
        std::string output = "<strong>" + content + "</strong>";
        return output;
    }

    std::string render_italic(nlohmann::json j, std::string content) {
        std::string output = "<i>" + content + "</i>";
        return output;
    }

    std::string render_over_line(nlohmann::json j, std::string content) {
        std::string output = "<s>" + content + "</s>";
        return output;
    }

    std::string render_inline_math(nlohmann::json j, std::string content) {
        std::string output = "\\(" + content + "\\)";
        return output;
    }

    std::string render_math_block(nlohmann::json j, std::string content) {
        std::string output = "\\[ \n" + content + "\n \\]";
        return output;
    }

    std::string render_code_block(nlohmann::json j, std::string content) {
        std::string language = j["language"];
        std::string output = "<pre><code class=\"" + language + "\">" + content + "</code></pre>";
        return output;
    }

    std::string render_judge(nlohmann::json j, std::string theme) {
        std::string uuid = j["uuid"];
        std::string title = j["title"];
        std::string sample_in_path = j["sample_in"];
        std::string sample_out_path = j["sample_out"];
        std::string source_path = j["source"];

        std::vector<std::string> in_files = glob(j["in"]);
        std::vector<std::string> out_files = glob(j["out"]);
        std::string judge = j["judge"];

        std::string title_h3 =
            "<h3 class=\"problem_title\"> <div class='badge' id='" + uuid + "_status'>WJ</div>   " + title + " </h2>\n";

        std::string editor_div = "<div class=\"editor\" id=\"" + uuid + "\" rows=\"3\" cols=\"80\"></div> \n";

        std::string ace_theme;
        if (theme == "dark") {
            ace_theme = "ace/theme/monokai";
        }
        else if (theme == "light") {
            ace_theme = "ace/theme/xcode";
        }
        else {
            std::cerr << "Invalid theme: " << theme << ", available themes are 'dark' and 'light'" << std::endl;
            exit(1);
        }

        std::string source = "";

        if (source_path != "") {
            source = read_file(source_path);
        }

        std::string ace_editor = ""
            "<script>"
            "editor = ace.edit(\"" + uuid + "\"); "
            "editor.setTheme(\"" + ace_theme + "\");"
            "editor.session.setMode(\"ace/mode/python\");"
            "editor.setShowPrintMargin(false);"
            "editor.setHighlightActiveLine(false);"
            "editor.setOptions({"
            "    enableBasicAutocompletion: true,"
            "    enableSnippets: true,"
            "    enableLiveAutocompletion: true,"
            "    minLines: 25, "
            "    maxLines: 25, "
            "    fontSize: \"14px\""
            "});"
            "editor.renderer.setScrollMargin(10, 10);"
            "editor.setValue(`" + source + "`, -1);"
            "</script>\n";


        std::string sample_in = read_file(sample_in_path);
        std::string sample_out = read_file(sample_out_path);


        std::string sample_in_area =
            "<div class=\"box-title\"> サンプルの入力 </div>"
            "<pre class=\"sample_in\" id=\"" + uuid + "_sample_in\">" + sample_in + "</pre>\n";

        std::string sample_out_area =
            "<div class=\"box-title\"> 出力 </div>"
            "<pre class=\"sample_out\" id=\"" + uuid + "_sample_out\"></pre>\n";

        std::string expect_out_area =
            "<div class=\"box-title\"> サンプルの答え </div>"
            "<pre class=\"expect_out\" id=\"" + uuid + "_expect_out\">" + sample_out + "</pre>\n";


        std::string define_data =
            "<script>"
            "all_input[\"" + uuid + "\"] = [];\n"
            "all_output[\"" + uuid + "\"] = [];\n"
            "all_sample_input[\"" + uuid + "\"] = `" + sample_in + "`;\n"
            "all_sample_output[\"" + uuid + "\"] = `" + sample_out + "`;\n"
            "problem_status[\"" + uuid + "\"] = \"WJ\";\n"
            "page_contents.push({\n"
            "    \"type\":\"Problem\",\n"
            "    \"id\":\"" + uuid + "\",\n"
            "    \"title\":\"" + title + "\"\n"
            "});\n";



        for (std::string in_file : in_files) {
            std::string input = read_file(in_file);
            define_data += "\n all_input[\"" + uuid + "\"].push(`" + input + "`)";
        }

        for (std::string in_file : out_files) {
            std::string output = read_file(in_file);
            define_data += "\n all_output[\"" + uuid + "\"].push(`" + output + "`)";
        }

        define_data += "</script> \n";


        std::string test_run_button =
            "<button class=\"runbutton\" onclick=\"runCode('" + uuid + "', false)\"> Run Sample </button>\n";

        std::string submit_button =
            "<button class=\"submitbutton\" onclick=\"runCode('" + uuid + "', true)\"> Submit </button>\n";

        std::string judge_code =
            "<script>\n"
            "judge_types[\"" + uuid + "\"] = `" + judge + "`\n"
            "</script>\n";


        std::string output = title_h3 + editor_div + ace_editor + sample_in_area + sample_out_area + expect_out_area + define_data + test_run_button + submit_button + judge_code;

        return output;
    }

    std::string render_executable_codeblock(nlohmann::json j, std::string theme) {
        std::string uuid = j["uuid"];
        std::string code = j["code"];

        int n_line = 0;

        for (char c : code) {
            if (c == '\n') {
                n_line++;
            }
        }

        std::string ace_theme;
        if (theme == "dark") {
            ace_theme = "ace/theme/monokai";
        }
        else if (theme == "light") {
            ace_theme = "ace/theme/xcode";
        }
        else {
            std::cerr << "Invalid theme: " << theme << ", available themes are 'dark' and 'light'" << std::endl;
            exit(1);
        }

        std::string editor_div = "<br> \n <div class=\"editor\" id=\"" + uuid + "\" rows=\"3\" cols=\"80\"></div> \n";

        std::string ace_editor = ""
            "<script>"
            "editor = ace.edit(\"" + uuid + "\"); "
            "editor.setTheme(\"" + ace_theme + "\");"
            "editor.session.setMode(\"ace/mode/python\");"
            "editor.setShowPrintMargin(false);"
            "editor.setHighlightActiveLine(false);"
            "editor.setOptions({"
            "    enableBasicAutocompletion: true,"
            "    enableSnippets: true,"
            "    enableLiveAutocompletion: true,"
            "    minLines: " + std::to_string(n_line + 1) + ", "
            "    maxLines: " + std::to_string(n_line + 1) + ", "
            "    fontSize: \"14px\""
            "});"
            "editor.renderer.setScrollMargin(10, 10);"
            "editor.setValue(`" + code + "`, -1);"
            "</script>\n";

        std::string out_area = "<pre class=\"sample_out\" id=\"" + uuid + "_out\"></pre>\n";

        std::string plot_area = "<div class=\"plot\" id=\"" + uuid + "_plot\"></div>\n";

        std::string run_button =
            "<button class=\"runbutton\" onclick=\"runBlock('" + uuid + "')\"> Run </button>\n";

        std::string output = editor_div + ace_editor + run_button + out_area + plot_area;

        return output;
    }

    std::string render_load_libs(nlohmann::json j, std::string content) {
        std::vector<std::string> libs = j["libs"];
        for (std::string lib : libs) {
            std::string output = "<script> use_libs.push(\"" + lib + "\"); </script>";
            content += output;
        }
        return content;
    }


    std::string render_plain_text(nlohmann::json j, std::string content) {
        std::string output = content;
        return output;
    }

    std::string render_block(nlohmann::json j, std::string content) {
        return content;
    }

    std::string render_newline(nlohmann::json j, std::string content) {
        return "<br>" + content;
    }

    size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t real_size = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);
        response->append(static_cast<char*>(contents), real_size);
        return real_size;
    }


    std::string render_inline_image(const std::string& url, const std::string& content) {
        std::string output = "<img src=\" " + url + " \" alt=\" " + content + " \" >";
        std::string figcaption = "<figcaption>" + content + "</figcaption>";
        return "<figure>" + output + figcaption + "</figure>";
    }


    std::string render_inline_url(std::string url, std::string content) {
        std::string output = "<a href=\"" + url + "\">" + content + "</a>";
        return output;
    }

    std::string render_list_block(nlohmann::json j, std::string content) {
        std::string output = "<ul>" + content + "</ul>";
        return output;
    }

    std::string render_enumerate_block(nlohmann::json j, std::string content) {
        std::string output = "<ol>" + content + "</ol>";
        return output;
    }

    std::string render_item(nlohmann::json j, std::string content) {
        std::string output = "<li>" + content + "</li>";
        return output;
    }

    std::string render_table(std::vector<std::string> from_render, int n_row, int n_col, std::vector<std::string> col_names, std::vector<int> col_format) {

        std::string output = "<table>";
        output += "<tr>";
        for (int i = 0; i < n_col; i++) {
            output += "<th>" + col_names[i] + "</th>";
        }

        output += "</tr>";

        for (int i = 0; i < n_row; i++) {
            output += "<tr>";
            for (int j = 0; j < n_col; j++) {
                std::string align = col_format[j] == 0 ? "left" : col_format[j] == 1 ? "center" : "right";
                output += "<td align=\"" + align + "\">" + from_render[i * n_col + j] + "</td>";
            }
            output += "</tr>";
        }

        output += "</table>";
        return output;
    }


    std::string render_inline_code(nlohmann::json j, std::string content) {
        std::string output = "<code>" + content + "</code>";
        return output;
    }


    std::string render_quote(nlohmann::json j, std::string content) {
        std::string output = "<blockquote>" + content + "</blockquote>";
        return output;
    }

    std::string render_horizonal_line(nlohmann::json j, std::string content) {
        std::string output = "<hr>";
        return output;
    }


    bool haschild(nlohmann::json j) {
        return !(j["class"] == "PlainText" || j["class"] == "NewLine" || j["class"] == "Url");
    }


    std::string build_block(nlohmann::json j, std::map<std::string, std::function<std::string(nlohmann::json, std::string)>> render_map) {
        std::string render_str;
        if (j["class"] == "InlineImage" || j["class"] == "InlineUrl") {
            nlohmann::json content = j["content"];
            std::string from_render;
            std::string url;
            for (nlohmann::json child : content) {
                if (child["class"] != "Url") {
                    from_render += build_block(child, render_map);
                }
                else {
                    url = child["content"];
                }
            }
            render_str = render_map[j["class"]](url, from_render);
        }
        else if (j["class"] == "Table") {
            std::vector<std::string> from_render;
            for (nlohmann::json child : j["content"]) {
                from_render.push_back(build_block(child, render_map));
            }


            std::vector<int> col_format = j["col_format"];
            std::vector<std::string> col_names = j["col_names"];

            std::string n_row_str = j["n_row"];
            std::string n_col_str = j["n_col"];


            int n_row = std::stoi(n_row_str);
            int n_col = std::stoi(n_col_str);


            return render_table(from_render, n_row, n_col, col_names, col_format);

        }
        else if (haschild(j)) {
            for (nlohmann::json child : j["content"]) {
                std::string from_render = build_block(child, render_map);
                render_str += from_render;
            }
            render_str = render_map[j["class"]](j, render_str);
        }
        else {
            render_str = render_map[j["class"]](j, j["content"]);
        }
        return render_str;
    }



    void render(nlohmann::json json_ir, nlohmann::json json_meta_data, std::string output_path) {
        // クラス名とレンダリング関数の対応map
        std::map<std::string, std::function<std::string(nlohmann::json, std::string)>> render_map;
        render_map["H1"] = render_h1;
        render_map["H2"] = render_h2;
        render_map["H3"] = render_h3;
        render_map["H4"] = render_h4;
        render_map["H5"] = render_h5;
        render_map["H6"] = render_h6;
        render_map["InlineStrong"] = render_strong;
        render_map["InlineItalic"] = render_italic;
        render_map["InlineOverline"] = render_over_line;
        render_map["InlineMath"] = render_inline_math;
        render_map["MathBlock"] = render_math_block;
        render_map["CodeBlock"] = render_code_block;
        render_map["Judge"] = render_judge;
        render_map["ExecutableCodeBlock"] = render_executable_codeblock;
        render_map["LoadLib"] = render_load_libs;
        render_map["PlainText"] = render_plain_text;
        render_map["Block"] = render_block;
        render_map["NewLine"] = render_newline;
        render_map["InlineUrl"] = render_inline_url;
        render_map["InlineImage"] = render_inline_image;
        render_map["ListBlock"] = render_list_block;
        render_map["EnumerateBlock"] = render_enumerate_block;
        render_map["Item"] = render_item;
        render_map["InlineCodeBlock"] = render_inline_code;
        render_map["Quote"] = render_quote;
        render_map["HorizontalLine"] = render_horizonal_line;

        std::string contents;

        std::string theme = json_meta_data["theme"];

        for (nlohmann::json block : json_ir) {
            if (block["class"] == "Judge") {
                contents += render_judge(block, theme);
            }
            else if (block["class"] == "ExecutableCodeBlock") {
                contents += render_executable_codeblock(block, theme);
            }
            else {
                std::string render_str;
                render_str = build_block(block, render_map);
                contents += render_str + "\n";
            }
        }

        std::string html_template = load_html_template(theme, json_meta_data["custom_css_path"]);

        std::vector<std::string> keys;

        std::string output_html;

        output_html = replace_template(html_template, json_meta_data, contents);

        std::ofstream output_file(output_path);
        output_file << output_html;
        output_file.close();
    }

}
