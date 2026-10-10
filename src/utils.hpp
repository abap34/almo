#pragma once

#include <glob.h>
#include "source.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>

// コンパイル時に埋め込む定数
namespace almo {
std::string LIGHT_THEME = {
#include "light.css"
};

std::string DARK_THEME = {
#include "dark.css"
};

std::string RUNNER = {
#include "runner.js"
};

std::string TEMPLATE = {
#include "template.html"
};

std::string VERSION = {
#include "version.txt"
};

std::string COMMITHASH = {
#include "commithash.txt"
};


}  // namespace almo

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

// Keep the existing exception names and messages for callers.
class ParseError : public almo::ParseException {
public:
    explicit ParseError(std::string message)
        : ParseException(std::move(message), "パースエラー  \n") {}
};

class SyntaxError : public almo::ParseException {
public:
    explicit SyntaxError(std::string message)
        : ParseException(std::move(message), "構文エラー  \n") {}
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
            } else {
                // --> がない場合
                continue;
            }
        } else {
            if (std::regex_match(line, comment_start)) {
                if (std::regex_match(line, comment_end)) {
                    // <!-- と --> が同じ行にある場合
                    ret += std::regex_replace(line, comment_start, "$1");
                    ret += std::regex_replace(line, comment_end, "$2");
                    ret += "\n";
                    in_comment = false;
                } else {
                    // <!-- があるが、 --> がない場合
                    ret += std::regex_replace(line, comment_start, "$1");
                    ret += "\n";
                    in_comment = true;
                }
            } else {
                ret += line;
                ret += "\n";
            }
        }
    }

    return ret;
}

// 出力フォーマットを指定するenum
enum class EscapeFormat {
    JSON,
    HTML,
    DOT
};

// 文字列を受け取り、指定されたフォーマットに応じてエスケープする
// JSON: ダブルクオーテーションや改行などをエスケープする
// HTML: HTMLエンティティにエスケープする
// DOT: DOT言語の特殊文字をエスケープする
std::string escape(const std::string& s, EscapeFormat format = EscapeFormat::JSON) {
    switch (format) {
        case EscapeFormat::JSON: {
            static constexpr char hex[] = "0123456789abcdef";
            std::string ret;
            ret.reserve(s.size());
            for (const unsigned char ch : s) {
                if (ch == '"' || ch == '\\' || ch <= 0x1f) {
                    ret += "\\u00";
                    ret += hex[ch >> 4];
                    ret += hex[ch & 0x0f];
                } else {
                    ret += static_cast<char>(ch);
                }
            }
            return ret;
        }
        case EscapeFormat::HTML: {
            std::string ret;
            ret.reserve(s.size());
            for (const char ch : s) {
                switch (ch) {
                    case '&': ret += "&amp;"; break;
                    case '<': ret += "&lt;"; break;
                    case '>': ret += "&gt;"; break;
                    case '"': ret += "&quot;"; break;
                    case '\'': ret += "&#39;"; break;
                    default: ret += ch; break;
                }
            }
            return ret;
        }
        case EscapeFormat::DOT: {
            std::string ret;
            ret.reserve(s.size());
            for (const char ch : s) {
                switch (ch) {
                    case '{': case '}': case '|': case '<': case '>':
                        ret += "\\\\";
                        break;
                }
                ret += ch;
            }
            return ret;
        }
    }
    return s;
}



// 文字列を受け取り、末尾のスペースを削除する
namespace almo::detail {
inline std::string_view rtrim_view(std::string_view text) {
    return text.substr(0, text.find_last_not_of(" \n\r\t") + 1);
}
}  // namespace almo::detail

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
std::string remove_listdef(std::string s) {
    if (s.size() >= 2 && s[0] == '-' && s[1] == ' ') {
        return s.substr(2);
    }
    return s;
}
