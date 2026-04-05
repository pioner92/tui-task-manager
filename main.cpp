#include "ftxui/component/app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <thread>

#include "core/controllers/app_controller.h"
#include "core/controllers/modals_controller.h"
#include "core/storage/models.h"
#include "core/storage/storage.h"
#include "ui/components/modals/modal_root.h"
#include "ui/footer.h"
#include "ui/header.h"
#include "ui/task_info_block/task_info_block.h"
#include "ui/timesheet_list/timesheets_list.h"

using namespace ftxui;

// Update timer each 1 sec
std::jthread run_timer(ScreenInteractive& screen) {
    return std::jthread([&screen](std::stop_token stop_token) {
        using namespace std::chrono_literals;

        while (!stop_token.stop_requested()) {
            std::this_thread::sleep_for(1s);

            if (stop_token.stop_requested()) {
                break;
            }

            screen.PostEvent(Event::Custom);
        }
    });
}

int main() {
    using namespace std::chrono_literals;
    auto screen = App::Fullscreen();

    auto timer_thread = run_timer(screen);
    const db::Database database("tasks.db");

    AppState app_state = {
        .tasks = db::get_all_tasks_with_sessions(database.raw()),
        .active_session = {},
        .ui = {.selected_task_index = 0},
        .db = database.raw(),
    };

    const ModalsController modals_controller(app_state);
    const AppController controller(app_state);
    controller.refresh_timesheet();

    const auto list_timesheet = TimesheetList(app_state.timesheet, app_state);

    const auto info_block = TaskInfoBlock(app_state, modals_controller);
    const auto modal_root = ModalRoot(app_state, controller, modals_controller);

    const auto body = Container::Horizontal({
        list_timesheet,
        info_block,
    });

    modals_controller.register_did_close([body, list_timesheet]() {
        body->SetActiveChild(list_timesheet);
    });

    auto app = Renderer(body, [&]() {
        return dbox({
                   vbox({
                       hbox({
                           PaddingH(),
                           vbox({
                               Header(app_state),
                               separator() | color(theme::divider),
                               list_timesheet->Render(),
                           }) | bgcolor(theme::panel),
                           PaddingH() | bgcolor(theme::bg),
                           PaddingH(),
                           info_block->Render() | bgcolor(theme::panel),
                       }) | flex,
                       Footer(),
                   }) | flex,
                   modal_root->Render() | center,
               }) |
            bgcolor(theme::panel);
    });

    app = app | CatchEvent([&](const Event& event) {
              if (app_state.ui.show_modal) {
                  if (event == Event::Escape) {
                      modals_controller.close();
                      return true;
                  }
                  return modal_root->OnEvent(event);
              }
              if (event == Event::Character('a')) {
                  modals_controller.open_create_task();
                  return true;
              }
              if (event == Event::Character('e')) {
                  modals_controller.open_edit_task();
                  return true;
              }
              if (event == Event::Character('q')) {
                  screen.ExitLoopClosure()();
                  return true;
              }
              if (event == Event::Character(' ')) {
                  controller.toggle_session();
                  return true;
              }
              if (event == Event::Character('d')) {
                  modals_controller.open_delete_task();
                  return true;
              }
              if (event == Event::Character('1')) {
                  controller.set_selected_task_status(TaskStatus::TODO);
                  return true;
              }
              if (event == Event::Character('2')) {
                  controller.set_selected_task_status(TaskStatus::IN_PROGRESS);
                  return true;
              }
              if (event == Event::Character('3')) {
                  controller.set_selected_task_status(TaskStatus::DONE);
                  return true;
              }
              return false;
          });

    screen.Loop(app);

    return 0;
}
