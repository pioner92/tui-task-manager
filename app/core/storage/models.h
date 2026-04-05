//
// Created by Oleksandr Shumihin on 23/3/26.
//

#pragma once
#include <optional>
#include <string>

enum class TaskStatus : int {
    TODO = 0,
    IN_PROGRESS = 1,
    DONE = 2,
};

struct SessionModel {
    int64_t id;
    int64_t task_id;
    int64_t start_at;
    std::optional<int64_t> end_at;
};

struct TaskModel {
    int64_t id;
    int64_t created_at;
    int64_t updated_at;
    std::string title;
    std::string description;
    TaskStatus status;
};
