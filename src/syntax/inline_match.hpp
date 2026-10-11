#pragma once

#include "../interfaces/syntax.hpp"

namespace almo::detail {

inline bool is_escaped_delimiter(std::string_view source, std::size_t position) {
    const auto end = position;
    while (position > 0 && source[position - 1] == '\\') --position;
    return (end - position) % 2 != 0;
}

inline std::string unescape_inline_text(std::string_view source) {
    auto escape = source.find('\\');
    if (escape == source.npos) return std::string(source);
    std::string result;
    result.reserve(source.size());
    std::size_t cursor = 0;
    do {
        result.append(source.substr(cursor, escape - cursor));
        cursor = escape + 1;
        if (cursor < source.size() && (source[cursor] == '\\' || source[cursor] == '$'))
            result += source[cursor++];
        else
            result += '\\';
        escape = source.find('\\', cursor);
    } while (escape != source.npos);
    result.append(source.substr(cursor));
    return result;
}

// These patterns have literal delimiters and non-greedy captures. Like the
// original ECMAScript regexes, a capture must not cross CR or LF.
inline std::optional<InlineMatch> find_delimited_inline(
    std::string_view source, std::string_view open, std::string_view close) {
    auto begin = source.find(open);
    while (begin != source.npos) {
        const auto content = begin + open.size();
        const auto end = source.find(close, content);
        if (end == source.npos) break;
        const auto line_end = source.substr(content, end - content).find_last_of("\r\n");
        if (line_end == source.npos)
            return InlineMatch{{begin, end + close.size()},
                               {std::string(source.substr(content, end - content))}};
        // All openers before the last line break would cross that break too.
        begin = source.find(open, content + line_end + 1);
    }
    return std::nullopt;
}

inline std::optional<InlineMatch> find_link_inline(
    std::string_view source, std::string_view open) {
    auto begin = source.find(open);
    while (begin != source.npos) {
        const auto label = begin + open.size();
        const auto separator = source.find("](", label);
        if (separator == source.npos) break;
        const auto target = separator + 2;
        const auto end = source.find(')', target);
        if (end == source.npos) break;
        const auto line_end = source.substr(label, end - label).find_last_of("\r\n");
        if (line_end == source.npos)
            return InlineMatch{{begin, end + 1},
                               {std::string(source.substr(label, separator - label)),
                                std::string(source.substr(target, end - target))}};
        begin = source.find(open, label + line_end + 1);
    }
    return std::nullopt;
}

}  // namespace almo::detail
