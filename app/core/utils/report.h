#pragma once
// #include "../app/models.h"


#include "core/app/models.h"
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>


inline std::string read_file_mmap(const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1)
        return {};

    struct stat st{};
    if (fstat(fd, &st) == -1) {
        close(fd);
        return {};
    }

    void* mapped = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        return {};
    }

    std::string result(static_cast<char*>(mapped), st.st_size);

    munmap(mapped, st.st_size);
    close(fd);

    return result;
}

inline void render_template(std::string_view templ,
                            std::string& out,
                            std::function<std::optional<std::string>(const std::string_view)>&& resolve) {
    size_t pos = 0;

    while (pos < templ.size()) {
        const size_t start = templ.find("{{", pos);
        if (start == std::string::npos) {
            out.append(templ.data() + pos, templ.size() - pos);
            break;
        }

        if (start > pos) {
            out.append(templ.data() + pos, start - pos);
        }

        const size_t close = templ.find("}}", start + 2);
        if (close == std::string::npos) {
            out.append(templ.data() + start, templ.size() - start);
            break;
        }

        const size_t end = close + 2;
        const std::string_view pattern{templ.data() + start, end - start};

        std::optional<std::string> replacement = resolve(pattern);
        if (replacement) {
            out.append(replacement.value().data(), replacement.value().size());
        }

        pos = end;
    }
}

// TODO: Add template support
inline std::string make_report(const AppState& app_state) {

    std::string res;

    res.reserve(1024);
    int64_t prev_day_key = 0;

    for (const auto& item: app_state.timesheet.get_items()) {
        if (prev_day_key != item.day_ts) {
            if (prev_day_key != 0) {
                res.append("  ");
            }
            res.append("\n");
            res.append(format_day_from_utc_day_key(item.day_ts));
            res.append(" | ");
            res.append(format_sec_to_hhmm(app_state.timesheet.get_day_duration(item.day_ts)));
            res.append("\n");
        }
        res.append(" - " + item.task_entity->task.title + "\n");
        prev_day_key = item.day_ts;
    }

    return res;
}

inline void render_task(const std::string& templ, std::string& out, const TimesheetItem& item) {
    render_template(templ, out, [&](const std::string_view patt) -> std::optional<std::string> {
        if (patt == "{{task}}") {
            return item.task_entity->task.title;
        }
        if (patt == "{{task_duration}}") {
            return format_sec_to_hhmm(item.duration_total_sec);
        }
        return std::nullopt;
    });
}

inline std::string make_report2(const AppState& app_state) {
    const std::string report_template = read_file_mmap("report_template.md");
    const std::string task_template = read_file_mmap("task_template.md");

    std::string res;

    std::map<int64_t, std::vector<const TimesheetItem*>> groups;

    for (const auto& item: app_state.timesheet.get_items()) {
        groups[item.day_ts].push_back(&item);
    }

    res.reserve(1024);

    for (const auto& [day_key, items]: groups) {
        render_template(report_template, res, [&](const std::string_view pattern) -> std::optional<std::string> {
            if (pattern == "{{day}}") {
                return format_day_from_utc_day_key(day_key);
            }
            if (pattern == "{{day_duration}}") {
                return format_sec_to_hhmm(app_state.timesheet.get_day_duration(day_key));
            }

            if (pattern == "{{tasks}}") {
                std::string tasks;
                for (const auto& t: items) {
                    render_task(task_template, tasks, *t);
                    tasks.append("\n");
                }
                return tasks;
            }
            return std::nullopt;
        });
    }

    std::cout << "" << res << '\n';

    return res;
}
