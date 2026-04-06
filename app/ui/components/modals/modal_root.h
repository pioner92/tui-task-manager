#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "core/app/models.h"
#include "core/controllers/app_controller.h"
#include "create_task_modal.h"
#include "delete_task_modal.h"
#include "edit_session_modal.h"
#include "edit_task_modal.h"

using namespace ftxui;


inline Component
ModalRoot(AppState& app_state, const AppController& controller, const ModalsController& modals_controller) {
    auto close = [&modals_controller] {
        modals_controller.close();
    };

    const auto modal_create_task = CreateTaskModal(
        modals_controller,
        [&controller, &modals_controller](const std::string& title, const std::string& description) {
            controller.create_new_task({title, description});
            modals_controller.close();
        },
        close);

    const auto modal_edit_task = EditTaskModal(
        modals_controller,
        [&controller, &modals_controller](const std::string& title, const std::string& description) {
            controller.edit_selected_task({title, description});
            modals_controller.close();
        },
        close);

    const auto modal_delete_task = DeleteTaskModal(
        modals_controller,
        [&controller, &modals_controller] {
            controller.delete_selected_task();
            modals_controller.close();
        },
        close);

    const auto modal_edit_sessions = EditSessionsModal(
        modals_controller,
        [&controller, &modals_controller] {
            if (const auto& selected_items_map = modals_controller.edit_sessions_state.selected_ids;
                !selected_items_map.empty()) {
                std::vector<int64_t> ids;
                ids.reserve(selected_items_map.size());

                for (auto& [id, state]: selected_items_map) {
                    if (state) {
                        ids.push_back(id);
                    }
                }
                controller.delete_selected_sessions(std::move(ids));
            }

            modals_controller.close();
        },
        close);


    auto container = Container::Stacked({
        modal_create_task,
        modal_edit_task,
        modal_delete_task,
        modal_edit_sessions,
    });

    return Renderer(
        container,
        [container, &app_state, modal_create_task, modal_delete_task, modal_edit_task, modal_edit_sessions]() {
            Element modal;

            switch (app_state.ui.modal_type) {
                case ModalType::CREATE_TASK:
                    modal = modal_create_task->Render();
                    if (container->ActiveChild() != modal_create_task) {
                        container->SetActiveChild(modal_create_task);
                    }
                    break;
                case ModalType::EDIT_TASK:
                    modal = modal_edit_task->Render();
                    if (container->ActiveChild() != modal_edit_task) {
                        container->SetActiveChild(modal_edit_task);
                    }
                    break;
                case ModalType::DELETE_TASK:
                    modal = modal_delete_task->Render();
                    if (container->ActiveChild() != modal_delete_task) {
                        container->SetActiveChild(modal_delete_task);
                    }
                    break;

                case ModalType::EDIT_SESSIONS:
                    modal = modal_edit_sessions->Render();
                    if (container->ActiveChild() != modal_edit_sessions) {
                        container->SetActiveChild(modal_edit_sessions);
                    }
                    break;
                case ModalType::NONE:
                    return emptyElement();
            }

            return dbox({
                modal | center,
            });
        });
}
