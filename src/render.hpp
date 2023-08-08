#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream> 
#include <glob.h>
#include <curl/curl.h>
#include "json.hpp"


namespace almo {


    std::string TEMPLATE = R"(
<!DOCTYPE html>
<html>
<meta charset="UTF-8">

<head>
    <title>ALMO</title>

    <!-- syntax hightlihgt -->
    <link rel="stylesheet" href="//cdnjs.cloudflare.com/ajax/libs/highlight.js/10.7.2/styles/monokai-sublime.min.css">
    <script src="//cdnjs.cloudflare.com/ajax/libs/highlight.js/9.18.1/highlight.min.js"></script>
    <script>hljs.initHighlightingOnLoad();</script>

    <!-- Pythonのランタイム -->
    <script src="https://cdn.jsdelivr.net/pyodide/v0.23.4/full/pyodide.js"></script>

    <!-- ACした時にブワーって出るアニメーション -->
    <script src="https://cdn.jsdelivr.net/npm/canvas-confetti@1.3.2/dist/confetti.browser.min.js"></script>

    <!-- エディタ -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.2.0/ace.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.2.0/ext-language_tools.js"></script>

    <!-- Math Jax -->
    <script type="text/javascript" id="MathJax-script" async
        src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-chtml.js"></script>

    <!-- アニメーション関連のやつ -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/gsap/3.11.0/gsap.min.js"></script>

    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #292929;
            color: #EAEAEA;
            flex-direction: column;
        }

        h1 {
            color: #DAB4FF;
            text-align: center;
            margin-bottom: 20px;
            text-shadow: 2px 2px 4px rgba(218, 180, 255, 0.3);
        }

        h2 {
            color: #DAB4FF;
            text-align: center;
            margin-bottom: 20px;
            text-shadow: 2px 2px 4px rgba(218, 180, 255, 0.3);
            text-align: left;
            padding-left: 10px;
            border-bottom: solid 1px #DAB4FF;
        }

        p {
            color: #BFBFBF;
            margin-bottom: 10px;
        }

        a {
            color: #DAB4FF;
        }

        .sidebar {
            border-right: 1px solid #eee;
            height: 100%;
            width: auto;
            padding: 10px 10px;
            left: 0;
            overflow: auto;
            position: fixed;
            top: 0;
            width: 200px;
            z-index: 1000;
            background-color: #3a3a3a;
        }


        .each_problem_div {
            margin-bottom: 10px;
            float: left;
            width: 100%;
        }

        .content_list_h1 {
            font-size: 20px;
            font-weight: bold;
            margin-bottom: 10px;
            border-bottom: solid 1px #DAB4FF;
            color: #eddbff;
        }

        .content_list_h2 {
            font-size: 16px;
            font-weight: bold;
            margin-bottom: 10px;
            color: #DAB4FF;
        }

        .content_list_problem {
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 10px;
            color: #ecdaff;
        }

        .badge {
            padding: 8px 20px;
            border-radius: 25px;
            font-size: 14px;
            font-weight: bold;
            color: #514f4f;
            border: 1px solid #fff;
            margin: 0 8px;
            float: left;
        }


        .runbutton {
            display: inline-block;
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            color: #fff;
            background-color: #007bffa3;
            cursor: pointer;
            text-align: center;
            text-decoration: none;
        }

        .runbutton:hover {
            background-color: #0057b384;
        }


        .submitbutton {
            display: inline-block;
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            color: #fff;
            background-color: #22ff0070;
            cursor: pointer;
            text-align: center;
            text-decoration: none;
        }

        .submitbutton:hover {
            background-color: #294928;
        }


        .problem_title {
            font-size: 20px;
            font-weight: bold;
        }

        .editor {
            width: 100%;
            height: 300px;
            font-size: 16px;
            font-family: monospace;
            background-color: #514f4f;
            color: #fff;
        }


        .output {
            width: 100%;
            padding: 5px 5px;
            overflow-x: auto;
            font-size: 16px;
            font-family: monospace;
            background-color: #514f4f;
            color: #fff;
        }

        .expect_out,
        .sample_in,
        .sample_out {
            width: 100%;
            padding: 5px 5px;
            overflow-x: auto;
            font-size: 16px;
            font-family: monospace;
            background-color: #514f4f;
            color: #fff;
        }

        .box-title {
            font-size: 16px;
            font-weight: bold;
        }

        .problem_list {
            font-size: 14px;
            font-weight: bold;
            margin-bottom: 10px;
        }

        

        .center {
            display: block;
            margin-left: auto;
            margin-right: auto;
            width: 50%;
        }


        table {
            border-collapse: collapse;
            width: 100%;
        }

        th {
            border: 1px solid #ddd;
            padding: 8px;
        }

        td {
            border: 1px solid #ddd;
            padding: 8px;
        }
    </style>
</head>

<body>


    <script>
        all_sample_input = {}
        all_sample_output = {}


        all_input = {}
        all_output = {}

        judge_types = {}
        problem_status = {}
        page_contents = []

        const pyodidePromise = loadPyodide({
            stdin: stdin_func,
            stdout: stdout_func,
        });


        function stdin_func() {
            if (submit_run) {
                return all_input[target_objectid][judge_idx];
            } else {
                return all_sample_input[target_objectid];
            }
        }

        function stdout_func(answer) {
            let out_id = target_objectid + "_sample_out";
            let expect_out_id = target_objectid + "_expect_out";
            outputs += answer + "\n";
        }

        function error_handle(error) {
            document.getElementById(target_objectid + "_sample_out").innerText = error;
            document.getElementById(target_objectid + "_sample_out").style.color = "orange";
            status = "RE";
        }

        const runCode = async (objectid, require_judge) => {
            // update global variable
            target_objectid = objectid;
            submit_run = require_judge;

            let pyodide = await pyodidePromise;

            let editor = ace.edit(objectid);
            let code = editor.getValue();
            let result_bar_id = target_objectid + "_status";
            let result_bar = document.getElementById(result_bar_id)
            let each_problem_div = document.getElementById("problem_list_" + target_objectid + "_badge");
            let out_id = target_objectid + "_sample_out";

            document.getElementById(target_objectid + "_sample_out").innerText = "";
            document.getElementById(target_objectid + "_sample_out").style.color = "white";
            result_bar.innerText = "Running...";


            if (require_judge) {
                let n_input = all_input[target_objectid].length;
                for (let i = 0; i < n_input; i++) {
                    judge_idx = i;
                    result_bar.innerText = "Running...   " + judge_idx + "/" + (n_input);
                    try {
                        outputs = "";
                        await pyodide.runPythonAsync(code);
                        let expect_out = all_output[target_objectid][judge_idx];
                        if (outputs.slice(-1) == "\n") {
                            outputs = outputs.slice(0, -1);
                        }
                        judge(outputs, expect_out);
                    } catch (error) {
                        error_handle(error);
                        result_bar.innerText = "WJ";
                    }

                    if (status == "TLE" || status == "MLE" || status == "RE" || status == "WA") {
                        result_bar.style.backgroundColor = "#ffe500";
                        result_bar.innerText = status + "   " + judge_idx + "/" + (n_input);
                        each_problem_div.style.backgroundColor = "#ffe500";
                        each_problem_div.innerText = status;
                        break;
                    }
                }

                if (status == "AC") {
                    result_bar.style.backgroundColor = "lightgreen";
                    result_bar.innerText = "AC";
                    each_problem_div.style.backgroundColor = "lightgreen";
                    each_problem_div.innerText = "AC";
                    confetti();
                } else {
                    result_bar.style.backgroundColor = "#ffe500";
                }
            } else {
                try {
                    outputs = "";
                    await pyodide.runPythonAsync(code);
                    document.getElementById(target_objectid + "_sample_out").innerText = outputs;
                    result_bar.innerText = "WJ";
                } catch (error) {
                    error_handle(error);
                    result_bar.innerText = "RE";
                }
            }
        };

        function judge(answer, expect_out) {
            // trim \n from both
            if (answer.slice(-1) == "\n") {
                answer = answer.slice(0, -1);
            }

            if (expect_out.slice(-1) == "\n") {
                expect_out = expect_out.slice(0, -1);
            }

            
            let result_bar_id = target_objectid + "_status";
            let judge_type = judge_types[target_objectid];
            if (judge_type == 'equal') {
                if (answer === expect_out) {
                    status = "AC";
                } else {
                    status = "WA";
                }
            } else if (judge_type.includes('err')) {
                status = "AC";
                let admissible_absolute_error = parseFloat(judge_type.split('_')[1]);
                let answer_list = answer.split('\n');
                let expect_out_list = expect_out.split('\n');
                for (let i = 0; i < answer_list.length; i++) {
                    let answer_line = answer_list[i].split(' ');
                    let expect_out_line = expect_out_list[i].split(' ');
                    for (let j = 0; j < answer_line.length; j++) {
                        let answer_out = parseFloat(answer_line[j]);
                        let expect_out_out = parseFloat(expect_out_line[j]);
                        if (Math.abs(answer_out - expect_out_out) > admissible_absolute_error) {
                            status = "WA";
                            return;
                        }
                    }
                }

            }
        }
    </script>

    <!-- ___split___ -->

    <div class="sidebar">
        <div id="content_list">
            <script>
                for (var i = 0; i < page_contents.length; i++) {
                    let content = page_contents[i];
                    if (content.type == "Problem") {
                        let li = document.createElement("div");
                        li.setAttribute("class", "content_list_problem");
                        li.setAttribute("id", "problem_list_" + content.id);
                        li.innerHTML = content.title;


                        let badge = document.createElement("span");
                        badge.setAttribute("class", "badge");
                        badge.setAttribute("id", "problem_list_" + content.id + "_badge");
                        badge.innerText = "WJ";
                        badge.style.fontSize = "12px";
                        badge.style.padding = "5px 10px";
                        badge.style.position = "relative";

                        let each_problem_div = document.createElement("div");
                        each_problem_div.setAttribute("id", "problem_list_" + content.id + "_div");
                        each_problem_div.setAttribute("class", "each_problem_div");

                        each_problem_div.appendChild(badge);
                        each_problem_div.appendChild(li);

                        document.getElementById("content_list").appendChild(each_problem_div);
                    } else if (content.type == "H1") {
                        let li = document.createElement("div");
                        li.setAttribute("class", "content_list_h1");
                        li.innerHTML = content.content
                        document.getElementById("content_list").appendChild(li);
                    } else if (content.type == "H2") {
                        let li = document.createElement("div");
                        li.setAttribute("class", "content_list_h2");
                        li.innerHTML = content.content
                        document.getElementById("content_list").appendChild(li);
                    }
                }




                const sidebar = document.querySelector(".sidebar");

                let isMenuOpen = false;

                gsap.to(sidebar, {
                    x: "-200",
                    opacity: 0,
                    duration: 0,
                    ease: "power1.out"
                })

                const onCursorMove = (event) => {
                    const cursorX = event.clientX;

                    if (!isMenuOpen && (cursorX <= 30)) { 
                        gsap.to(sidebar, {
                            x: "0",
                            opacity: 1,
                            duration: 1,
                            ease: "power1.out"
                        });
                        isMenuOpen = true;
                    }

                    if (isMenuOpen && cursorX > 30) {
                        gsap.to(sidebar, {
                            x: "-200",
                            opacity: 0,
                            duration: 1,
                            ease: "power1.out"
                        });
                        isMenuOpen = false;
                    }
                };

                window.addEventListener("mousemove", onCursorMove);
            </script>
        </div>
    </div>
    </script>
    </div>

</body>



</html>
)";









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
        std::string html_template = TEMPLATE;
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

    std::string render_newline(nlohmann::json j, std::string content) {
        return "<br>" + content;
    }

    size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t real_size = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);
        response->append(static_cast<char*>(contents), real_size);
        return real_size;
    }

    std::string download_image(const std::string& url) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            return "";
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return "";
        }

        return response;
    }

    std::string base64_encode(const std::string& input) {
        static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string output;
        size_t in_len = input.length();
        size_t i = 0;

        while (i < in_len) {
            uint32_t octet_a = i < in_len ? (unsigned char)input[i++] : 0;
            uint32_t octet_b = i < in_len ? (unsigned char)input[i++] : 0;
            uint32_t octet_c = i < in_len ? (unsigned char)input[i++] : 0;

            uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

            output += base64_chars[(triple >> 18) & 0x3F];
            output += base64_chars[(triple >> 12) & 0x3F];
            output += base64_chars[(triple >> 6) & 0x3F];
            output += base64_chars[triple & 0x3F];
        }

        return output;
    }


    std::string render_inline_image(const std::string& url, const std::string& content) {
        std::string image_data;
        if (url.find("http://") == 0 || url.find("https://") == 0) {
            image_data = download_image(url);
            if (image_data.empty()) {
                return "";
            }
        }
        else {
            std::ifstream image_stream(url, std::ios::binary);
            if (!image_stream) {
                return "";
            }
            std::ostringstream oss;
            oss << image_stream.rdbuf();
            image_data = oss.str();
        }

        std::string base64_image = base64_encode(image_data);

        // Generate the HTML tag with Base64-encoded image data
        std::string output = "<img src=\"data:image/jpeg;base64," + base64_image + "\" alt=\"" + content + "\" width=\"50%\" class=\"center\">";
        return output;
    }


    std::string render_inline_url(std::string url, std::string content) {
        std::string output = "<a href=\"" + url + "\">" + content + "</a>";
        return output;
    }



    std::string render_list_block(nlohmann::json j, std::string content) {
        std::string output = "<ul>" + content + "</ul>";
        return output;
    }

    std::string render_item(nlohmann::json j, std::string content) {
        std::string output = "<li>" + content + "</li>";
        return output;
    }

    std::string render_table(std::vector<std::string> from_render, int n_row, int n_col, std::vector<std::string> col_names, std::vector<int> col_format) {
        // std::cout << "render table" << std::endl;
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


    void render(nlohmann::json json_ir) {

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
        render_map["InlineUrl"] = render_inline_url;
        render_map["InlineImage"] = render_inline_image;
        render_map["ListBlock"] = render_list_block;
        render_map["Item"] = render_item;

        std::string outputs;

        for (nlohmann::json block : json_ir) {
            if (block["class"] == "CodeRunner") {
                outputs += render_code_runner(block, "");
            }
            else {
                std::string render_str;
                render_str = build_block(block, render_map);
                outputs += render_str + "\n";
            }

        }

        std::pair<std::string, std::string> html_template = load_html_template();

        std::string output_html = html_template.first + outputs + html_template.second;

        std::cout << output_html << std::endl;
    }

}