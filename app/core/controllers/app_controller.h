//
// Created by Oleksandr Shumihin on 28/03/2026.
//
#pragma once
#include "core/app/models.h"
#include "core/handlers.h"
#include "core/utils/transform.h"

class AppController {
public:
    explicit AppController(AppState& app_state) : app_state_(app_state) {};

    void refresh_timesheet() const {
        app_state_.timesheet = to_timesheet(app_state_.tasks);
        sort_sessions();
    }

    void create_new_task(const CreateTaskInfo& info) const {
        create_task(app_state_, info);
        refresh_timesheet();
    }

    void edit_selected_task(const CreateTaskInfo& info) const {
        if (const std::optional<int64_t> task_id = get_selected_task_id_from_timesheets(app_state_)) {
            update_task(app_state_, *task_id, info);
            refresh_timesheet();
        }
    }

    void delete_selected_task() const {
        if (const std::optional<int64_t> task_id = get_selected_task_id_from_timesheets(app_state_)) {
            delete_task(app_state_, *task_id);
            refresh_timesheet();

            auto& idx = app_state_.ui.selected_task_index;
            const int new_size = static_cast<int>(app_state_.timesheet.get_items().size());
            if (new_size == 0) {
                idx = 0;
            } else if (idx >= new_size) {
                idx = new_size - 1;
            }
        }
    }

    void toggle_session() const {
        if (app_state_.active_session) {
            stop_session(app_state_);
        } else {
            if (const std::optional<int64_t> task_id = get_selected_task_id_from_timesheets(app_state_)) {
                start_session(app_state_, *task_id);
                update_task_status(app_state_, *task_id, TaskStatus::IN_PROGRESS);
            }
        }
        refresh_timesheet();
    }

    void set_selected_task_status(const TaskStatus status) const {
        if (const std::optional<int64_t> task_id = get_selected_task_id_from_timesheets(app_state_)) {
            update_task_status(app_state_, *task_id, status);
        }
    }

    void delete_selected_sessions(const std::vector<int64_t>&& session_ids) const {
        if (const std::optional<int64_t> task_id = get_selected_task_id_from_timesheets(app_state_)) {
            delete_task_sessions(app_state_, *task_id, session_ids);
            refresh_timesheet();
        }
    }

private:
    AppState& app_state_;

    void reset_active_session() const {
        if (app_state_.active_session) {
            app_state_.active_session.reset();
        }
    }

    // Days DESC | sessions inside a group ASC
    void sort_sessions() const {
        for (auto& te: app_state_.tasks) {
            std::ranges::sort(te.sessions, [](const SessionModel& a, const SessionModel& b) {
                const auto day_a = to_day_key(a.start_at);
                const auto day_b = to_day_key(b.start_at);

                if (day_a != day_b) {
                    return day_a > day_b;
                }

                return a.start_at < b.start_at;
            });
        }
    }
};
