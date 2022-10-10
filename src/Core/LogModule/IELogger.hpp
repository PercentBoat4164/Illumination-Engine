#pragma once

#include <memory>
#include <spdlog/logger.h>
#include <string>
#include <vulkan/vulkan.h>

namespace IE::Core {
/**
 * @brief A small logging tool that is a very thin wrapper for spdlog.
 * @class IELogger
 */
class Logger {
private:
    std::shared_ptr<spdlog::logger> m_logger;
    static bool                     m_init;

public:
    enum Level {
        ILLUMINATION_ENGINE_LOG_LEVEL_OFF             = spdlog::level::off,
        ILLUMINATION_ENGINE_LOG_LEVEL_TRACE           = spdlog::level::trace,
        ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG           = spdlog::level::debug,
        ILLUMINATION_ENGINE_LOG_LEVEL_INFO            = spdlog::level::info,
        ILLUMINATION_ENGINE_LOG_LEVEL_WARN            = spdlog::level::warn,
        ILLUMINATION_ENGINE_LOG_LEVEL_ERROR           = spdlog::level::err,
        ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL        = spdlog::level::critical,
        ILLUMINATION_ENGINE_LOG_LEVEL_MAX             = spdlog::level::n_levels,
        ILLUMINATION_ENGINE_LOG_LEVEL_0               = spdlog::level::off,
        ILLUMINATION_ENGINE_LOG_LEVEL_1               = spdlog::level::trace,
        ILLUMINATION_ENGINE_LOG_LEVEL_2               = spdlog::level::debug,
        ILLUMINATION_ENGINE_LOG_LEVEL_3               = spdlog::level::info,
        ILLUMINATION_ENGINE_LOG_LEVEL_4               = spdlog::level::warn,
        ILLUMINATION_ENGINE_LOG_LEVEL_5               = spdlog::level::err,
        ILLUMINATION_ENGINE_LOG_LEVEL_6               = spdlog::level::critical,
        ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_LOW    = spdlog::level::warn,
        ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_MEDIUM = spdlog::level::err,
        ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_HIGH   = spdlog::level::critical,
    };

    enum Flags {
        ILLUMINATION_ENGINE_LOG_TO_NONE   = 0x0,
        ILLUMINATION_ENGINE_LOG_TO_STDOUT = 0x1,
        ILLUMINATION_ENGINE_LOG_TO_FILE   = 0x2,
        ILLUMINATION_ENGINE_LOG_PRESERVE  = 0x4,
    };

    explicit Logger(
      const std::string &t_name,
      const std::string &t_path  = "",
      Flags              t_flags = IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_STDOUT
    );

    void log(
      const std::string      &t_msg,
      IE::Core::Logger::Level t_level = IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_TRACE
    ) const;

    void log(VkDebugUtilsMessageSeverityFlagBitsEXT t_level, const std::string &t_msg) const;

    void setLogLevel(Level t_level);

    static void init();
};

/**
 * @brief Basic private 'constructor' for the Flags enum.
 * @details This is used to create IE::Graphics::Image::Locations from the uint8_ts that are used to perform the
 * binary operators. Before converting the t_flags to an IE::Core::Logger::Flags, validity checks are
 *performed to ensure that the t_flags does not represent a physically impossible scenario.
 * @param t_flags The value that the new Location should take on.
 * @return A verified IE::Graphics::Image::Location.
 **/
constexpr IE::Core::Logger::Flags Flags(uint8_t t_flags) {
    if (t_flags & IE::Core::Logger::Flags::ILLUMINATION_ENGINE_LOG_TO_NONE && (t_flags & ~IE::Core::Logger::Flags::ILLUMINATION_ENGINE_LOG_TO_NONE))
        throw std::logic_error("ILLUMINATION_ENGINE_LOG_TO_NONE mix error!");
    return static_cast<IE::Core::Logger::Flags>(t_flags);
}

/**
 * @brief An implementation of the OR operator for the IE::Core::Logger::Flags enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first | t_second.
 */
constexpr IE::Core::Logger::Flags operator|(IE::Core::Logger::Flags t_first, IE::Core::Logger::Flags t_second) {
    return Flags(static_cast<uint8_t>(t_first) | static_cast<uint8_t>(t_second));
}

/**
 * @brief An implementation of the AND operator for the IE::Core::Logger::Flags enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first & t_second.
 */
constexpr IE::Core::Logger::Flags
operator&(IE::Core::Logger::Flags t_first, IE::Core::Logger::Flags t_second) noexcept {
    return Flags(static_cast<uint8_t>(t_first) & static_cast<uint8_t>(t_second));
}

/**
 * @brief An implementation of the XOR operator for the IE::Core::Logger::Flags enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first ^ t_second.
 */
constexpr IE::Core::Logger::Flags
operator^(IE::Core::Logger::Flags t_first, IE::Core::Logger::Flags t_second) noexcept {
    return Flags(static_cast<uint8_t>(t_first) ^ static_cast<uint8_t>(t_second));
}

/**
 * @brief An implementation of the NOT operator for the IE::Core::Logger::Flags enum.
 * @details This operator does not necessarily return a valid location. It may return a location that has both the
 * ILLUMINATION_ENGINE_LOG_TO_NONE and ILLUMINATION_ENGINE_LOG_TO_STDOUT bits set for example
 * @param t_first Operand.
 * @return ~t_first
 */
constexpr IE::Core::Logger::Flags operator~(IE::Core::Logger::Flags t_first) noexcept {
    return static_cast<IE::Core::Logger::Flags>(~static_cast<uint8_t>(t_first));
}
}  // namespace IE::Core