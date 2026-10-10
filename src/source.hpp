#pragma once

#include <cstddef>

namespace almo {

// UTF-8 byte offsets in the original source, [begin, end).
struct SourceRange {
    std::size_t begin = 0;
    std::size_t end = 0;
    bool operator==(const SourceRange&) const = default;
};

}  // namespace almo
