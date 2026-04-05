//
// Created by Oleksandr Shumihin on 26/03/2026.
//

#pragma once
#include <algorithm>
#include <unordered_set>

#include "core/app/models.h"
#include "core/storage/storage.h"
#include "core/utils/time_format.h"


inline void create_task(AppState &app_state, const CreateTaskInfo &task_info) {
    using namespace std::chrono;

    const auto ts = duration_cast<std::chrono::milliseconds>(WallClock::now().time_since_epoch()).count();


    TaskEntity &t_e = app_state.tasks.emplace_back(TaskEntity{
        .task =
        {
            0,
            ts,
            ts,
            task_info.title,
            task_info.description.value_or(""),
        },
        .sessions = {}
    });
    db::insert_task(app_state.db, t_e.task);
}

inline void update_task(AppState &app_state, int64_t task_id, const CreateTaskInfo &task_info) {
    using namespace std::chrono;

    const auto it = std::ranges::find_if(app_state.tasks, [task_id](const TaskEntity &task) {
        return task.task.id == task_id;
    });

    if (it != app_state.tasks.end()) {
        it->task.title = task_info.title;
        if (task_info.description.has_value()) {
            it->task.description = task_info.description.value();
        }
        it->task.updated_at = get_now();

        db::update_task(app_state.db, it->task);
    }
}

inline void update_task_status(AppState &app_state, int64_t task_id, const TaskStatus status) {
    const auto it = std::ranges::find_if(app_state.tasks, [task_id](const TaskEntity &task) {
        return task.task.id == task_id;
    });

    db::update_task_status(app_state.db, task_id, status);

    if (it != app_state.tasks.end()) {
        it->task.status = status;
    }
}

inline void delete_task(AppState &app_state, int64_t task_id) {
    const auto it = std::ranges::find_if(app_state.tasks, [task_id](const TaskEntity &task) {
        return task.task.id == task_id;
    });

    // Stop active session before removing the task
    if (it != app_state.tasks.end()) {
        if (app_state.active_session && app_state.active_session.value().task_id == task_id) {
            app_state.active_session.reset();
        }
        app_state.tasks.erase(it);
    }

    db::delete_task(app_state.db, task_id);
}

inline void start_session(AppState &app_state, const int64_t task_id) {
    SessionModel session = {
        0,
        task_id,
        get_now(),
        std::nullopt,
    };

    // updates session.id after insert
    db::insert_session(app_state.db, session);

    const auto it = std::ranges::find_if(app_state.tasks, [task_id](const TaskEntity &t_e) {
        return t_e.task.id == task_id;
    });

    app_state.active_session = {task_id, session.id, session.start_at, &app_state};

    if (it != app_state.tasks.end()) {
        it->sessions.emplace_back(session);
    }
}

inline void stop_session(AppState &app_state) {
    if (app_state.active_session) {
        app_state.active_session.reset();
    }
}

inline void delete_task_sessions(AppState &app_state, int64_t task_id, const std::vector<int64_t> &session_ids) {
    db::delete_sessions(app_state.db, session_ids);

    if (app_state.tasks.empty()) {
        return;
    }

    const auto it = std::ranges::find_if(app_state.tasks, [task_id](const TaskEntity &t) {
        return t.task.id == task_id;
    });

    if (it != app_state.tasks.end()) {
        const std::unordered_set<int64_t> ids{session_ids.begin(), session_ids.end()};

        // If the active session is deleted -> stop it
        if (app_state.active_session && ids.contains(app_state.active_session.value().session_id)) {
            app_state.active_session.reset();
        }

        std::erase_if(it->sessions, [&](const SessionModel &s) {
            return ids.contains(s.id);
        });
    }
}
