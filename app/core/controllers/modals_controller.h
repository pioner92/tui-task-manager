//
// Created by Oleksandr Shumihin on 28/03/2026.
//

#pragma once
#include "core/app/models.h"
#include <functional>


struct CreateTaskModalState {
    std::string title;
    std::string description;

    void reset() {
        title.clear();
        description.clear();
    }
};

struct EditSessionsModalState {
    std::optional<const TaskEntity*> task_entity = std::nullopt;
    std::unordered_map<int64_t, bool> selected_ids;

    void reset() {
        selected_ids.clear();
        task_entity.reset();
    }
};

using Action = std::function<void()>;

class ModalsController {
public:
    mutable int64_t triggered_value = 0; // is used to trigger modals reset or update values
    mutable CreateTaskModalState modal_state;
    mutable EditSessionsModalState edit_sessions_state;

    explicit ModalsController(AppState& app_state) : app_state_(app_state) {}

    void open_create_task() const {
        update_triggered();
        app_state_.ui.show_modal = true;
        app_state_.ui.modal_type = ModalType::CREATE_TASK;
    }

    void open_edit_task() const {
        const auto& items = app_state_.timesheet.get_items();

        if (app_state_.ui.selected_task_index < items.size()) {
            update_triggered();
            const TaskModel& task = items[app_state_.ui.selected_task_index].task_entity->task;

            modal_state.title = task.title;
            modal_state.description = task.description;

            app_state_.ui.show_modal = true;
            app_state_.ui.modal_type = ModalType::EDIT_TASK;
        }
    }

    void open_delete_task() const {
        update_triggered();
        app_state_.ui.show_modal = true;
        app_state_.ui.modal_type = ModalType::DELETE_TASK;
    }

    void open_edit_sessions() const {
        const int selected_index = app_state_.ui.selected_task_index;
        const auto& items = app_state_.timesheet.get_items();

        if (items.empty() || selected_index >= items.size()) {
            return;
        }
        update_triggered();

        edit_sessions_state.task_entity = items[selected_index].task_entity;

        app_state_.ui.show_modal = true;
        app_state_.ui.modal_type = ModalType::EDIT_SESSIONS;
    }

    void close() const {
        update_triggered();
        app_state_.ui.show_modal = false;
        app_state_.ui.modal_type = ModalType::NONE;

        modal_state.reset();
        edit_sessions_state.reset();

        did_close_();
    }

    void register_did_close(Action&& did_close) const {
        did_close_ = std::move(did_close);
    }

private:
    AppState& app_state_;
    mutable Action did_close_;

    void update_triggered() const {
        triggered_value++;
    }
};
