#pragma once

#include <spdlog/fmt/fmt.h>

#include "depthai/utility/Path.hpp"
#include "depthai-shared/datatype/DatatypeEnum.hpp"

namespace dai {
namespace utility {
static constexpr char path_convert_err[] = "<Unicode path not convertible>";
}
}  // namespace dai

template <>
struct fmt::formatter<dai::Path> : formatter<std::string> {
    // https://fmt.dev/latest/api.html#formatting-user-defined-types
    // https://fmt.dev/latest/syntax.html#format-specification-mini-language
    template <typename FormatContext>
    auto format(const dai::Path& p, FormatContext& ctx) {
        std::string output;
        try {
            output = p.string();
        } catch(const std::exception&) {
            output = dai::utility::path_convert_err;
        }
        return formatter<std::string>::format(output, ctx);
    }
};

#ifdef MOVERSE_DEPTHAI_CORE
#include "magic_enum.hpp"

template <>
struct fmt::formatter<dai::DatatypeEnum> : fmt::formatter<std::string> {
    auto format(const dai::DatatypeEnum& type, fmt::format_context& ctx) -> decltype(ctx.out()) {
        std::string output;
        try {
            output = magic_enum::enum_name(type);
        } catch(const std::exception&) {
            output = dai::utility::path_convert_err;
        }
        return formatter<std::string>::format(output, ctx);
    }
};
#endif
