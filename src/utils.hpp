#pragma once

#include <string>
#include <sstream> 
#include <glob.h>

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
