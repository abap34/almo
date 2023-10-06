#include <string>

std::string join(std::vector<std::string> parts){
    std::string result = "";
    for (std::string part : parts){
        result += part;
    }
    return result;
}

// パースが失敗した時のエラー
class ParseError : public std::exception {
    std::string message;
public:
    ParseError(std::string message) : message(message) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};


// 構文が間違っている時のエラー
class SyntaxError : public std::exception {
    std::string message;
    int error_pos;
public:
    SyntaxError(std::string message, int error_pos) : message(message), error_pos(error_pos) {}
    const char* what() const noexcept override {
        return "構文エラー  " + std::to_string(error_pos) + "文字目付近: " + message.c_str(); 
    }
};
