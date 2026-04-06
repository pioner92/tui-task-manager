//
// Created by Oleksandr Shumihin on 25/3/26.
//

#pragma once

#include "sqlite3.h"
#include <chrono>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>

#include "../storage/models.h"
#include "../utils/time_format.h"

namespace db {
int finalize_session(sqlite3* db, int64_t id, int64_t end_at);
}

struct TaskEntity {
    TaskModel task;
    std::deque<SessionModel> sessions;
};

struct CreateTaskInfo {
    std::string title;
    std::optional<std::string> description;
};

struct TimesheetItem {
    int64_t day_ts;
    int64_t duration_total_sec;
    const TaskEntity* task_entity;
};

struct Timesheet {
    using DayKey = int64_t;
    using DurationSec = int64_t;
    using DaysDurations = std::unordered_map<DayKey, DurationSec>;

    [[nodiscard]] DurationSec get_day_duration(const DayKey day_key) const {
        if (days_duration_.contains(day_key)) {
            return days_duration_.at(day_key);
        }
        return 0;
    };

    [[nodiscard]] const std::vector<TimesheetItem>& get_items() const {
        return items_;
    };

private:
    friend Timesheet to_timesheet(const std::deque<TaskEntity>& task_entities);

    DaysDurations days_duration_;
    std::vector<TimesheetItem> items_;
};


struct AppState;

struct ActiveSessionState {
    ActiveSessionState(const int64_t task_id, const int64_t session_id, const int64_t started_at, AppState* app_state) :
        task_id(task_id), session_id(session_id), started_at(started_at), app_state(app_state) {}

    int64_t task_id = 0;
    int64_t session_id = 0;
    int64_t started_at = 0;

    int64_t duration_sec = 0;

    ~ActiveSessionState();

    void refresh_duration() {
        duration_sec = (get_now() - started_at) / 1000;
    }

private:
    AppState* app_state = nullptr;
};

enum class ModalType {
    NONE,
    CREATE_TASK,
    EDIT_TASK,
    DELETE_TASK,
    EDIT_SESSIONS,
};

struct UIState {
    bool show_modal = false;
    ModalType modal_type = ModalType::NONE;
    int selected_task_index = 0;
};

struct AppState {
    std::deque<TaskEntity> tasks; // source of truth
    Timesheet timesheet;
    std::optional<ActiveSessionState> active_session;
    UIState ui;
    sqlite3* db;
};

inline ActiveSessionState::~ActiveSessionState() {
    if (app_state == nullptr || app_state->db == nullptr) {
        return;
    }

    const int64_t end_at =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();


    for (auto& t_e: app_state->tasks) {
        if (t_e.task.id == task_id) {
            for (auto& s: t_e.sessions) {
                if (s.id == session_id) {
                    s.end_at = end_at;
                }
            }
        }
    }

    db::finalize_session(app_state->db, session_id, end_at);
}
