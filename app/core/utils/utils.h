//
// Created by Oleksandr Shumihin on 23/3/26.
//

#pragma once
#include "core/app/models.h"
#include "core/storage/models.h"
#include <numeric>


inline const char* STATUS_TEXT_TABLE[] = {"TODO", "IN PROGRESS", "DONE"};
inline const char* STATUS_TEXT_SHORT_TABLE[] = {"[T]", "[I]", "[D]"};

inline int64_t duration_from_sessions_ms(const std::vector<SessionModel>& sessions) {
    return std::accumulate(sessions.begin(), sessions.end(), int64_t{0}, [](const int64_t acc, const SessionModel& s) {
        return s.end_at ? acc + (*s.end_at - s.start_at) : acc;
    });
}

inline int64_t duration_from_sessions_sec(const std::vector<SessionModel>& sessions) {
    return std::accumulate(sessions.begin(),
                           sessions.end(),
                           int64_t{0},
                           [](const int64_t acc, const SessionModel& s) {
                               return s.end_at ? acc + (*s.end_at - s.start_at) : acc;
                           }) /
        1000;
}

inline std::optional<int64_t> get_selected_task_id_from_timesheets(const AppState& app_state) {
    const auto& items = app_state.timesheet.get_items();

    if (const int selected_task_index = app_state.ui.selected_task_index; selected_task_index < items.size()) {
        return items[selected_task_index].task_entity->task.id;
    }
    return std::nullopt;
}

inline std::optional<TaskEntity*> get_selected_task_entity_from_timesheets(const AppState& app_state) {
    const auto& items = app_state.timesheet.get_items();
    if (const int selected_task_index = app_state.ui.selected_task_index; selected_task_index < items.size()) {
        const TaskEntity* task_entity = items[selected_task_index].task_entity;
        if (task_entity != nullptr) {
            return const_cast<TaskEntity*>(task_entity);
        }
        return std::nullopt;
    }
    return std::nullopt;
}


inline int64_t get_session_duration_sec(const SessionModel& session) {
    if (session.end_at) {
        return (*session.end_at - session.start_at) / 1000;
    }
    return 0;
}


inline bool copy_to_clipboard_mac(const std::string& text) {
    FILE* pipe = popen("pbcopy", "w");
    if (!pipe) {
        return false;
    }

    const size_t written = fwrite(text.data(), 1, text.size(), pipe);
    const int rc = pclose(pipe);

    return written == text.size() && rc == 0;
}
