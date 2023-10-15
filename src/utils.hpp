#pragma once

#include <string>
#include <sstream> 
#include <glob.h>
#include <regex>
#include <fstream>
#include <map>
#include <iostream>
#include <cstring>

// 文字列の vector を 結合する。第二引数はOptionalで区切り文字。デフォルトは空
std::string join(std::vector<std::string> v, std::string sep = "") {
    std::stringstream ss;
    for (int i = 0; i < v.size(); i++) {
        ss << v[i];
        if (i != v.size() - 1) {
            ss << sep;
        }
    }
    return ss.str();
}


// 文字列を sep で分割する
std::vector<std::string> split(std::string str, std::string sep) {
    std::vector<std::string> result;
    int start = 0;
    int end = str.find(sep);
    while (end != -1) {
        result.push_back(str.substr(start, end - start));
        start = end + sep.size();
        end = str.find(sep, start);
    }
    result.push_back(str.substr(start, end - start));
    return result;
}


// パースが失敗した時のエラー
class ParseError : public std::exception {
    std::string message;
public:
    ParseError(std::string message) : message(message) {}
    const char* what() const noexcept override {
        std::string error = "パースエラー  \n" + message;
        char* result = new char[error.length() + 1];
        strcpy(result, error.c_str());
        return result;
    }
};


// 構文が間違っている時のエラー
class SyntaxError : public std::exception {
    std::string message;
public:
    SyntaxError(std::string message) : message(message) {}
    const char* what() const noexcept override {
        std::string error = "構文エラー  \n" + message;
        char* result = new char[error.length() + 1];
        strcpy(result, error.c_str());
        return result;
    }
};

// ファイルが存在しない時のエラー
class NotFoundFileError : public std::exception {
    std::string message;
public:
    NotFoundFileError(std::string message) : message(message) {}
    const char* what() const noexcept override {
        std::string error = "ファイルが存在しません  \n" + message;
        char* result = new char[error.length() + 1];
        strcpy(result, error.c_str());
        return result;
    }
};

// コマンドライン引数が間違っている時のエラー
class InvalidCommandLineArgumentsError : public std::exception {
    std::string message;
public:
    InvalidCommandLineArgumentsError(std::string message) : message(message) {}
    const char* what() const noexcept override {
        std::string error = "不正なコマンドライン引数  \n" + message;
        char* result = new char[error.length() + 1];
        strcpy(result, error.c_str());
        return result;
    }
};


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


// ファイルを読んで、各行をvectorに入れて返す
// ファイルが存在しない場合はNotFoundFileErrorを投げる
std::vector<std::string> read_file(std::string path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw NotFoundFileError("ファイルが存在しません: " + path);
    }
    std::vector<std::string> result;
    std::string line;
    while (getline(ifs, line)) {
        result.push_back(line);
    }
    return result;
}


// 文字列を受け取り、コメントに囲まれた部分を削除して返す
std::string _remove_comment(std::string s) {
    std::vector<std::string> lines = split(s, "\n");


    std::regex comment_start = std::regex("(.*)<!--(.*)");
    std::regex comment_end = std::regex("(.*)-->(.*)");

    std::string ret = "";

    bool in_comment = false;

    for (std::string line : lines) {
        if (in_comment) {
            if (std::regex_match(line, comment_end)) {
                // --> がある場合
                ret += std::regex_replace(line, comment_end, "$2");
                ret += "\n";
                in_comment = false;
            }
            else {
                // --> がない場合
                continue;
            }
        }
        else {
            if (std::regex_match(line, comment_start)) {
                if (std::regex_match(line, comment_end)) {
                    // <!-- と --> が同じ行にある場合
                    ret += std::regex_replace(line, comment_start, "$1");
                    ret += std::regex_replace(line, comment_end, "$2");
                    ret += "\n";
                    in_comment = false;
                }
                else {
                    // <!-- があるが、 --> がない場合
                    ret += std::regex_replace(line, comment_start, "$1");
                    ret += "\n";
                    in_comment = true;
                }
            }
            else {
                ret += line;
                ret += "\n";
            }
        }
    }


    return ret;
}


// エイリアスを取り出す
std::map<std::string, std::string> get_alias(std::string s) {
    std::regex alias_regex = std::regex("\\{\\{\\s*([a-zA-Z1-9]*)\\s*=\\s*\"(.*)\"\\s*\\}\\}");

    std::map<std::string, std::string> result;

    std::smatch match;
    while (std::regex_search(s, match, alias_regex)) {
        result[match[1]] = match[2];
        s = match.suffix();
    }

    return result;
}

// エイリアスの定義文を削除
std::string _remove_alias(std::string s) {
    std::vector<std::string> lines = split(s, "\n");
    std::regex alias_regex = std::regex("(.*)\\{\\{\\s*([a-zA-Z1-9_]*)\\s*=\\s*\"(.*)\"\\s*\\}\\}(.*)");
    std::string ret = "";
    for (std::string line : lines) {
        if (line == "") {
            ret += "\n";
        }
        std::string _line = std::regex_replace(line, alias_regex, "$1$4");
        if (std::regex_match(_line, std::regex("\\s*$"))) {
            continue;
        }
        else {
            ret += _line + "\n";
        }
    }
    return ret;
}

// エイリアスを適用
std::string _apply_alias(std::string s, std::map<std::string, std::string> alias_map) {
    std::regex noarg_alias_regex = std::regex("\\{\\{\\s*([a-zA-Z1-9_]*)\\s*\\}\\}");
    std::regex alias_regex = std::regex("\\{\\{\\s*([a-zA-Z1-9_]*)\\s(.*)\\}\\}");

    std::smatch match;

    // 引数なしのエイリアスを適用
    while (std::regex_search(s, match, noarg_alias_regex)) {
        std::string var = match[1];
        std::string alias = alias_map[var];
        s = std::regex_replace(s, noarg_alias_regex, alias);
    }

    // 引数ありのエイリアスを適用
    while (std::regex_search(s, match, alias_regex)) {
        std::string args = match[2];
        std::string var = match[1];
        std::vector<std::string> args_vec = split(args, " ");

        // args_vecの最後の要素は空白になるので落とす
        args_vec.pop_back();


        std::string alias = alias_map[var];

        // args_vecの各要素を評価
        for (int i = 0; i < args_vec.size(); i++) {
            // ダブルクオーテーションで囲まれている場合は、そのまま
            if (args_vec[i][0] == '"' && args_vec[i][args_vec[i].size() - 1] == '"') {
                args_vec[i] = args_vec[i].substr(1, args_vec[i].size() - 2);

            }
            // エイリアスが存在する場合は、エイリアスを適用
            else if (alias_map.find(args_vec[i]) != alias_map.end()) {
                args_vec[i] = alias_map[args_vec[i]];
            }
            // それ以外はエラー
            else {
                throw ParseError("エイリアスが存在しません: " + args_vec[i]);
            }
        }

        for (int i = 0; i < args_vec.size(); i++) {
            alias = std::regex_replace(alias, std::regex("\\$" + std::to_string(i + 1)), args_vec[i]);
        }
        s = std::regex_replace(s, alias_regex, alias, std::regex_constants::format_first_only);
    }

    return s;
}


// 文字列を受け取り、ダブルクオーテーションや改行などをエスケープする
std::string escape(std::string s) {
    std::string result;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '"') {
            result += "\\\"";
        }
        else if (s[i] == '\n') {
            result += "\\n";
        }
        else if (s[i] == '\t') {
            result += "\\t";
        }
        else if (s[i] == '\\') {
            result += "\\\\";
        }
        else if (s[i] == '\r') {
            result += "\\r";
        }
        else if (s[i] == '{') {
            result += "\\{";
        }
        else if (s[i] == '}') {
            result += "\\}";
        }
        else {
            result += s[i];
        }
    }
    return result;
}