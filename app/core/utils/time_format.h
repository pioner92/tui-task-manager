//
// Created by Oleksandr Shumihin on 27/03/2026.
//

#pragma once


#include <iomanip>
#include <sstream>
#include <string>

using SteadyClock = std::chrono::steady_clock;
using WallClock = std::chrono::system_clock;

inline constexpr int64_t MS_PER_DAY = 86'400'000;

inline std::string format_sec_to_hhmm(const long long seconds) {
    const long long hours = seconds / 3600;
    const long long minutes = (seconds % 3600) / 60;

    std::array<char, 32> buf{};
    char* ptr = buf.data();
    char* end = buf.data() + buf.size();

    if (hours > 0) {
        auto [p1, ec] = std::to_chars(ptr, end, hours);
        ptr = p1;
        *ptr++ = 'h';
        *ptr++ = ' ';

        if (minutes < 10) {
            *ptr++ = '0';
        }

        auto [p2, ec2] = std::to_chars(ptr, end, minutes);
        ptr = p2;
        *ptr++ = 'm';
        return std::string{buf.data(), ptr};
    }

    if (minutes > 0) {
        auto [p1, ec] = std::to_chars(ptr, end, minutes);
        ptr = p1;
        *ptr++ = 'm';
        return std::string{buf.data(), ptr};
    }

    return "0m";
}

inline std::string format_sec_to_hhmmss(const long long seconds) {
    const long long hours = seconds / 3600;
    const long long minutes = (seconds % 3600) / 60;
    const long long remainder = seconds % 60;

    std::array<char, 32> buf{};
    char* ptr = buf.data();
    char* end = buf.data() + buf.size();

    if (hours > 0) {
        auto [p1, ec] = std::to_chars(ptr, end, hours);
        ptr = p1;
        *ptr++ = 'h';
        *ptr++ = ' ';

        if (minutes < 10) {
            *ptr++ = '0';
        }

        auto [p2, ec2] = std::to_chars(ptr, end, minutes);
        ptr = p2;
        *ptr++ = 'm';
        *ptr++ = ' ';

        if (remainder < 10) {
            *ptr++ = '0';
        }

        auto [p3, ec3] = std::to_chars(ptr, end, remainder);
        ptr = p3;
        *ptr++ = 's';
        return std::string{buf.data(), ptr};
    }

    if (minutes > 0) {
        auto [p1, ec] = std::to_chars(ptr, end, minutes);
        ptr = p1;
        *ptr++ = 'm';
        *ptr++ = ' ';

        if (remainder < 10) {
            *ptr++ = '0';
        }

        auto [p2, ec2] = std::to_chars(ptr, end, remainder);
        ptr = p2;
        *ptr++ = 's';
        return std::string{buf.data(), ptr};
    }

    if (remainder > 0) {
        auto [p1, ec] = std::to_chars(ptr, end, remainder);
        ptr = p1;
        *ptr++ = 's';
        return std::string{buf.data(), ptr};
    }

    return "0s";
}

inline long long get_now() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(WallClock::now().time_since_epoch()).count();
}

inline std::string format_ms_to_hhmm_local(const long long ms) {
    const auto tp = WallClock::time_point{std::chrono::milliseconds{ms}};
    const std::time_t tt = WallClock::to_time_t(tp);

    const std::tm local_tm = *std::localtime(&tt);

    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M", &local_tm);
    return buf;
}

// Mar 26, 2026, at 20:56
inline std::string format_ts(const int64_t ms) {
    const std::time_t tt = ms / 1000;

    std::tm tm{};
    localtime_r(&tt, &tm); // local time

    std::ostringstream oss;
    oss << std::put_time(&tm, "%b %d, %Y, at %H:%M");

    return oss.str();
}

// dd/mm/yyyy hh:mm
inline std::string format_ts2(const int64_t ms) {
    const std::time_t tt = ms / 1000;

    std::tm tm{};
    localtime_r(&tt, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y %H:%M");

    return oss.str();
}

inline int64_t to_day_key(const int64_t ms) {
    return ms / MS_PER_DAY;
}

// Fri, Mar 20
inline std::string format_day_from_utc_day_key(const int64_t day_key) {
    std::chrono::system_clock::time_point tp{std::chrono::milliseconds(day_key * MS_PER_DAY)};

    return std::format("{:%a, %b %d}", tp);
}
