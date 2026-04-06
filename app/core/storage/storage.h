//
// Created by Oleksandr Shumihin on 23/3/26.
//

#pragma once
#include "../app/models.h"
#include "./models.h"
#include <deque>
#include <sqlite3.h>


namespace db {
sqlite3* open_db(const char* path);

inline void throw_sqlite_error(sqlite3* db, const char* prefix) {
    throw std::runtime_error(std::string(prefix) + ": " + sqlite3_errmsg(db));
}

void close_db(sqlite3* db);

class Database {
public:
    explicit Database(const char* path);

    ~Database();

    Database(const Database&) = delete;

    Database& operator=(const Database&) = delete;

    Database(Database&& other) noexcept;

    Database& operator=(Database&& other) noexcept;

    sqlite3* raw() const;

private:
    sqlite3* db_ = nullptr;
};

// == TASKS ==

int64_t insert_task(sqlite3* db, TaskModel& task);

int update_task(sqlite3* db, const TaskModel& task);

int update_task_status(sqlite3* db, int64_t id, const TaskStatus& status);

void delete_task(sqlite3* db, const int64_t id);

std::vector<TaskModel> get_all_tasks(sqlite3* db);

std::deque<TaskEntity> get_all_tasks_with_sessions(sqlite3* db);

// == TASK SESSIONS ==
int64_t insert_session(sqlite3* db, SessionModel& session);

int update_session(sqlite3* db, const SessionModel& session);

int finalize_session(sqlite3* db, int64_t id, int64_t end_at);
void delete_sessions(sqlite3* db, const std::vector<int64_t>& ids);
} // namespace db
