#pragma once

#include <string>
#include <sstream> 
#include <glob.h>
#include <regex>
#include <fstream>
#include <map>
#include <iostream>
#include <cstring>
#include <iomanip>

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


// 文字列を受け取り、ダブルクオーテーションや改行などをエスケープする
// https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
std::string escape(const std::string &s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c == '"' || *c == '\\' || ('\x00' <= *c && *c <= '\x1f')) {
            o << "\\u"
              << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
        } else {
            o << *c;
        }
    }
    return o.str();
}


//　文字列を受け取り、 HTMLに埋め込んでも大丈夫なようにエスケープする.
//  ref: https://github.com/abap34/ALMO/issues/91
std::string escape_for_html(const std::string &s) {
    std::string ret = s;
    ret = std::regex_replace(ret, std::regex("&"), "&amp;");
    ret = std::regex_replace(ret, std::regex("<"), "&lt;");
    ret = std::regex_replace(ret, std::regex(">"), "&gt;");
    ret = std::regex_replace(ret, std::regex("\""), "&quot;");
    ret = std::regex_replace(ret, std::regex("'"), "&#39;");
    return ret;
}



// 文字列を受け取り、末尾のスペースを削除する
std::string rtrim(std::string s) {
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
    return s;
}


// 文字列を受け取り、先頭にあるスペースを削除する
std::string ltrim(std::string s) {
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    return s;
}



// 文字列を受け取って、先頭が "- " だったら削除
std::string remove_listdef(std::string s){
    if (s.size() >= 2 && s[0] == '-' && s[1] == ' ') {
        return s.substr(2);
    }
    return s;
}