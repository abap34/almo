#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional> 
#include <glob.h>
#include "json.hpp"

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }

    std::string line;
    std::string output;
    while (std::getline(file, line)) {
        output += line + "\n";
    }
    return output;
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

std::pair<std::string, std::string> load_html_template() {
    std::string html_template = read_file("template.html");
    std::string head = html_template.substr(0, html_template.find("<!-- ___split___ -->"));
    std::string tail = html_template.substr(html_template.find("<!-- ___split___ -->") + 20);
    return std::make_pair(head, tail);
}

std::string render_h1(nlohmann::json j, std::string content) {
    std::string uuid = j["uuid"];
    std::string add_page_content = "<script>"
        "page_contents.push({ "
        "    \"type\":\"H1\", "
        "    \"id\":\"" + uuid + "\", "
        "    \"content\":\"" + content + "\" "
        "})"
        "</script> \n";
    std::string output = "<h1>" + content + "</h1>";
    return add_page_content + output;
}

std::string render_h2(nlohmann::json j, std::string content) {
    std::string uuid = j["uuid"];
    std::string add_page_content = "<script>"
        "page_contents.push({ "
        "    \"type\":\"H2\", "
        "    \"id\":\"" + uuid + "\", "
        "    \"content\":\"" + content + "\" "
        "})"
        "</script> \n";
    std::string output = "<h2>" + content + "</h2>";
    return add_page_content + output;
}

std::string render_h3(nlohmann::json j, std::string content) {
    std::string output = "<h3>" + content + "</h3>";
    return output;
}

std::string render_h4(nlohmann::json j, std::string content) {
    std::string output = "<h4>" + content + "</h4>";
    return output;
}

std::string render_h5(nlohmann::json j, std::string content) {
    std::string output = "<h5>" + content + "</h5>";
    return output;
}

std::string render_h6(nlohmann::json j, std::string content) {
    std::string output = "<h6>" + content + "</h6>";
    return output;
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
    std::string output = "\\[" + content + "\\]";
    return output;
}

std::string render_math_block(nlohmann::json j, std::string content) {
    std::string output = "\\[ \n" + content + "\n \\]";
    return output;
}

std::string render_code_block(nlohmann::json j, std::string content) {
    std::string output = "<pre><code>" + content + "</code></pre>";
    return output;
}

std::string render_code_runner(nlohmann::json j, std::string content) {
    std::string uuid = j["uuid"];
    std::string title = j["title"];
    std::string sample_in_path = j["sample_in"];
    std::string sample_out_path = j["sample_out"];
    std::vector<std::string> in_files = glob(j["in"]);
    std::vector<std::string> out_files = glob(j["out"]);
    std::string judge = j["judge"];


    std::string title_h3 =
        "<h3 class=\"problem_title\"> <div class='badge' id='" + uuid + "_status'>WJ</div>   " + title + " </h2>\n";

    std::string editor_div = "<div class=\"editor\" id=\"" + uuid + "\" rows=\"3\" cols=\"80\"></div> \n";
    std::string ace_editor = ""
        "<script>"
        "editor = ace.edit(\"" + uuid + "\"); "
        "editor.setTheme(\"ace/theme/monokai\");"
        "editor.session.setMode(\"ace/mode/python\");"
        "editor.setShowPrintMargin(false);"
        "editor.setOptions({"
        "  enableBasicAutocompletion: true,"
        "  enableSnippets: true,"
        "  enableLiveAutocompletion: true"
        "});"
        "editor.setValue(\"\");"
        "</script>"
        "<br>";

    std::string sample_in = read_file(sample_in_path);
    std::string sample_out = read_file(sample_out_path);


    std::string sample_in_area =
        "<div class=\"box-title\"> サンプルの入力 </div>"
        "<pre class=\"sample_in\" id=\"" + uuid + "_sample_in\"><code>" + sample_in + "</code></pre>\n";

    std::string sample_out_area =
        "<div class=\"box-title\"> 出力 </div>"
        "<pre class=\"sample_out\" id=\"" + uuid + "_sample_out\"><code></code></pre>\n";

    std::string expect_out_area =
        "<div class=\"box-title\"> サンプルの答え </div>"
        "<pre class=\"expect_out\" id=\"" + uuid + "_expect_out\"><code>" + sample_out + "</code></pre>\n";


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

std::string render_plain_text(nlohmann::json j, std::string content) {
    std::string output = content;
    return output;
}

std::string render_block(nlohmann::json j, std::string content) {
    return content;
}

std::string render_newline(nlohmann::json j, std::string content){
    return "<br>" + content;
}

bool haschild(nlohmann::json j) {
   return !(j["class"] == "PlainText" || j["class"] == "NewLine");
}

std::string build_block(nlohmann::json j, std::map<std::string, std::function<std::string(nlohmann::json, std::string)>> render_map) {
    std::string render_str;
    if (haschild(j)) {
        for (nlohmann::json child : j["content"]) {
            std::string from_render = build_block(child, render_map);
            render_str += from_render;
        }
        render_str = render_map[j["class"]](j, render_str);
    } else {
        render_str = render_map[j["class"]](j, j["content"]);
    }
    return render_str;
}


    int main() {

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
        render_map["CodeRunner"] = render_code_runner;
        render_map["PlainText"] = render_plain_text;
        render_map["Block"] = render_block;
        render_map["NewLine"] = render_newline;


        nlohmann::json json_ir;
        std::ifstream expect_file("noya2.json");
        expect_file >> json_ir;

        std::string outputs;

        for (nlohmann::json block : json_ir) {
            if (block["class"] == "CodeRunner"){
                outputs += render_code_runner(block, "");
            } else {
                std::string render_str;
                render_str = build_block(block, render_map);
                outputs += render_str + "\n";
            }
            
        }

        std::pair<std::string, std::string> html_template = load_html_template();

        std::string output_html = html_template.first + outputs + html_template.second;

        std::cout << output_html << std::endl;
    }

