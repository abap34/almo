#pragma once

#include <string_view>

namespace almo::inline_match {

struct DelimitedMatch {
    std::size_t start = std::string_view::npos;
    std::string_view prefix;
    std::string_view content;
    std::string_view suffix;
};

struct LinkMatch {
    std::size_t start = std::string_view::npos;
    std::string_view prefix;
    std::string_view label;
    std::string_view target;
    std::string_view suffix;
};

inline bool find_delimited(std::string_view str, std::string_view open,
                           std::string_view close, DelimitedMatch& match) {
    for (std::size_t open_pos = str.find(open); open_pos != std::string_view::npos;
         open_pos = str.find(open, open_pos + 1)) {
        const std::size_t content_start = open_pos + open.size();
        const std::size_t close_pos = str.find(close, content_start);
        if (close_pos == std::string_view::npos) {
            continue;
        }

        match.start = open_pos;
        match.prefix = str.substr(0, open_pos);
        match.content = str.substr(content_start, close_pos - content_start);
        match.suffix = str.substr(close_pos + close.size());
        return true;
    }

    return false;
}

inline bool find_link(std::string_view str, std::string_view opener,
                      LinkMatch& match) {
    for (std::size_t open_pos = str.find(opener); open_pos != std::string_view::npos;
         open_pos = str.find(opener, open_pos + 1)) {
        const std::size_t label_start = open_pos + opener.size();
        const std::size_t separator_pos = str.find("](", label_start);
        if (separator_pos == std::string_view::npos) {
            continue;
        }

        const std::size_t close_pos = str.find(')', separator_pos + 2);
        if (close_pos == std::string_view::npos) {
            continue;
        }

        match.start = open_pos;
        match.prefix = str.substr(0, open_pos);
        match.label = str.substr(label_start, separator_pos - label_start);
        match.target =
            str.substr(separator_pos + 2, close_pos - (separator_pos + 2));
        match.suffix = str.substr(close_pos + 1);
        return true;
    }

    return false;
}

}  // namespace almo::inline_match
