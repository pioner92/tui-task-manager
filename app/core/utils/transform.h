//
// Created by Oleksandr Shumihin on 26/03/2026.
//

#pragma once
#include <algorithm>

#include "./utils.h"
#include "time_format.h"
#include <unordered_map>
#include <vector>


struct TaskDayAgg {
    const TaskEntity* task_entity = nullptr;
    int64_t duration = 0;
};

inline Timesheet to_timesheet(const std::deque<TaskEntity>& task_entities) {
    using DayTS = int64_t;
    using TaskId = int64_t;

    Timesheet timesheet;

    std::unordered_map<DayTS, std::unordered_map<TaskId, TaskDayAgg>> session_groups;
    session_groups.reserve(task_entities.size());

    for (const auto& te: task_entities) {
        const TaskId task_id = te.task.id;
        if (te.sessions.empty()) {
            int64_t day_ts = to_day_key(te.task.created_at);
            auto& agg = session_groups[day_ts][task_id];
            agg.task_entity = &te;
            continue;
        }

        for (const auto& s: te.sessions) {
            const DayTS day_ts = to_day_key(s.start_at);
            auto& agg = session_groups[day_ts][task_id];
            agg.task_entity = &te;
            if (s.end_at) {
                agg.duration += get_session_duration_sec(s);
            }
        }
    }

    size_t total = 0;
    for (const auto& [day, tasks]: session_groups) {
        total += tasks.size();
    }

    timesheet.items_.reserve(total);
    timesheet.days_duration_.reserve(session_groups.size());

    for (const auto& [day_ts, t_d]: session_groups) {
        for (const auto& [_, agg]: t_d) {
            timesheet.items_.emplace_back(day_ts, agg.duration, agg.task_entity);
            timesheet.days_duration_[day_ts] += agg.duration;
        }
    }

    // Sort by day desc, then by created_at desc, then by task_id desc ( the most recent tasks first )
    std::ranges::sort(timesheet.items_, [](const TimesheetItem& a, const TimesheetItem& b) {
        if (a.day_ts != b.day_ts) {
            return a.day_ts > b.day_ts;
        }

        const auto a_created_at = a.task_entity->task.created_at;
        const auto b_created_at = b.task_entity->task.created_at;

        if (a_created_at != b_created_at) {
            return a_created_at > b_created_at;
        }

        return a.task_entity->task.id > b.task_entity->task.id;
    });

    return timesheet;
}
