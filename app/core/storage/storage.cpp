//
// Created by Oleksandr Shumihin on 23/3/26.
//

#include "storage.h"

#include <deque>
#include <sqlite3.h>
#include <stdexcept>

namespace db {
void exec_or_throw(sqlite3* db, const char* sql) {
    char* err_msg = nullptr;

    const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string msg = err_msg ? err_msg : "unknown sqlite error";
        sqlite3_free(err_msg);
        throw std::runtime_error("SQLite error: " + msg);
    }
}

sqlite3* open_db(const char* path) {
    sqlite3* db = nullptr;

    const int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        std::string msg = db ? sqlite3_errmsg(db) : "failed to open database";
        if (db) {
            sqlite3_close(db);
        }
        throw std::runtime_error("Failed to open database: " + msg);
    }

    try {
        exec_or_throw(db, "PRAGMA foreign_keys = ON;");
        exec_or_throw(db, "PRAGMA journal_mode = WAL;");
        exec_or_throw(db, "PRAGMA synchronous = NORMAL;");
        exec_or_throw(db, "PRAGMA temp_store = MEMORY;");
        exec_or_throw(db, "PRAGMA busy_timeout = 5000;");

        exec_or_throw(db, "BEGIN;");

        exec_or_throw(db, R"sql(
            CREATE TABLE IF NOT EXISTS tasks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                created_at INTEGER NOT NULL,
                updated_at INTEGER NOT NULL,
                title TEXT NOT NULL,
                description TEXT NOT NULL DEFAULT '',
                status INTEGER NOT NULL CHECK (status IN (0, 1, 2))
            );
        )sql");

        exec_or_throw(db, R"sql(
            CREATE TABLE IF NOT EXISTS sessions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                task_id INTEGER NOT NULL,
                start_at INTEGER NOT NULL,
                end_at INTEGER,
                FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE,
                CHECK (end_at IS NULL OR end_at >= start_at)
            );
        )sql");

        exec_or_throw(db, R"sql(
            CREATE INDEX IF NOT EXISTS idx_tasks_status
            ON tasks(status);
        )sql");

        exec_or_throw(db, R"sql(
            CREATE INDEX IF NOT EXISTS idx_tasks_created_at
            ON tasks(created_at);
        )sql");

        exec_or_throw(db, R"sql(
            CREATE INDEX IF NOT EXISTS idx_tasks_updated_at
            ON tasks(updated_at);
        )sql");

        exec_or_throw(db, R"sql(
            CREATE INDEX IF NOT EXISTS idx_sessions_task_id
            ON sessions(task_id);
        )sql");

        exec_or_throw(db, R"sql(
            CREATE INDEX IF NOT EXISTS idx_sessions_start_at
            ON sessions(start_at);
        )sql");

        exec_or_throw(db, R"sql(
            CREATE INDEX IF NOT EXISTS idx_sessions_task_id_start_at
            ON sessions(task_id, start_at);
        )sql");

        exec_or_throw(db, "COMMIT;");

        return db;
    } catch (...) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        throw;
    }
}


void close_db(sqlite3* db) {
    if (db) {
        sqlite3_close(db);
    }
}


Database::Database(const char* path) : db_(open_db(path)) {};

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

Database::Database(Database&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        if (db_) {
            sqlite3_close(db_);
        }
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

sqlite3* Database::raw() const {
    return db_;
}

int64_t insert_task(sqlite3* db, TaskModel& task) {
    static constexpr const char* sql = R"sql(
        INSERT INTO tasks (
            created_at,
            updated_at,
            title,
            description,
            status
        ) VALUES (?, ?, ?, ?, ?);
    )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare insert_task failed");
    }

    try {
        if (sqlite3_bind_int64(stmt, 1, task.created_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind created_at failed");
        }

        if (sqlite3_bind_int64(stmt, 2, task.updated_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind updated_at failed");
        }

        if (sqlite3_bind_text(stmt, 3, task.title.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            throw_sqlite_error(db, "bind title failed");
        }

        if (sqlite3_bind_text(stmt, 4, task.description.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            throw_sqlite_error(db, "bind description failed");
        }

        if (sqlite3_bind_int(stmt, 5, static_cast<int>(task.status)) != SQLITE_OK) {
            throw_sqlite_error(db, "bind status failed");
        }

        const int step_rc = sqlite3_step(stmt);
        if (step_rc != SQLITE_DONE) {
            throw_sqlite_error(db, "step insert_task failed");
        }

        const int64_t inserted_id = sqlite3_last_insert_rowid(db);

        sqlite3_finalize(stmt);

        task.id = inserted_id;
        return inserted_id;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

int update_task(sqlite3* db, const TaskModel& task) {
    static constexpr const char* sql = R"sql(
        UPDATE tasks
        SET
            created_at = ?,
            updated_at = ?,
            title = ?,
            description = ?,
            status = ?
        WHERE id = ?;
    )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare update_task failed");
    }

    try {
        if (sqlite3_bind_int64(stmt, 1, task.created_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind created_at failed");
        }

        if (sqlite3_bind_int64(stmt, 2, task.updated_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind updated_at failed");
        }

        if (sqlite3_bind_text(stmt, 3, task.title.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            throw_sqlite_error(db, "bind title failed");
        }

        if (sqlite3_bind_text(stmt, 4, task.description.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            throw_sqlite_error(db, "bind description failed");
        }

        if (sqlite3_bind_int(stmt, 5, static_cast<int>(task.status)) != SQLITE_OK) {
            throw_sqlite_error(db, "bind status failed");
        }

        if (sqlite3_bind_int64(stmt, 6, task.id) != SQLITE_OK) {
            throw_sqlite_error(db, "bind id failed");
        }

        const int step_rc = sqlite3_step(stmt);
        if (step_rc != SQLITE_DONE) {
            throw_sqlite_error(db, "step update_task failed");
        }

        const int changed = sqlite3_changes(db);

        sqlite3_finalize(stmt);
        return changed;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

int update_task_status(sqlite3* db, int64_t id, const TaskStatus& status) {
    static constexpr const char* sql = R"sql(
            UPDATE tasks
            SET status = ?
            WHERE id = ?;
        )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare update_task failed");
    }

    try {
        if (sqlite3_bind_int(stmt, 1, static_cast<int>(status)) != SQLITE_OK) {
            throw_sqlite_error(db, "bind status failed");
        }

        if (sqlite3_bind_int64(stmt, 2, id) != SQLITE_OK) {
            throw_sqlite_error(db, "bind id failed");
        }

        const int step_rc = sqlite3_step(stmt);
        if (step_rc != SQLITE_DONE) {
            throw_sqlite_error(db, "step update_task failed");
        }

        const int changed = sqlite3_changes(db);

        sqlite3_finalize(stmt);
        return changed;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

void delete_task(sqlite3* db, const int64_t id) {
    static constexpr const char* sql = R"sql(
            DELETE FROM tasks
            WHERE id = ?;
        )sql";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw_sqlite_error(db, "prepare update_session failed");
    }

    try {
        sqlite3_bind_int64(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            throw_sqlite_error(db, "delete failed");
        }

        sqlite3_finalize(stmt);
    } catch (...) {
        sqlite3_finalize(stmt);
    }
}

int64_t insert_session(sqlite3* db, SessionModel& session) {
    static constexpr const char* sql = R"sql(
            INSERT INTO sessions (
                task_id,
                start_at,
                end_at
            ) VALUES (?, ?, ?);
        )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare insert_task failed");
    }

    try {
        if (sqlite3_bind_int64(stmt, 1, session.task_id) != SQLITE_OK) {
            throw_sqlite_error(db, "bind task_id failed");
        }
        //
        if (sqlite3_bind_int64(stmt, 2, session.start_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind start_at failed");
        }
        if (session.end_at.has_value()) {
            if (sqlite3_bind_int64(stmt, 3, *session.end_at) != SQLITE_OK) {
                throw_sqlite_error(db, "bind end_at failed");
            }
        } else {
            if (sqlite3_bind_null(stmt, 3) != SQLITE_OK) {
                throw_sqlite_error(db, "bind null end_at failed");
            }
        }

        const int step_rc = sqlite3_step(stmt);
        if (step_rc != SQLITE_DONE) {
            throw_sqlite_error(db, "step insert_task failed");
        }

        const int64_t inserted_id = sqlite3_last_insert_rowid(db);

        sqlite3_finalize(stmt);
        session.id = inserted_id;
        return inserted_id;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

int update_session(sqlite3* db, const SessionModel& session) {
    static constexpr const char* sql = R"sql(
            UPDATE sessions
            SET
                task_id = ?,
                start_at = ?,
                end_at = ?
            WHERE id = ?;
        )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare update_session failed");
    }

    try {
        if (sqlite3_bind_int64(stmt, 1, session.task_id) != SQLITE_OK) {
            throw_sqlite_error(db, "bind task_id failed");
        }

        if (sqlite3_bind_int64(stmt, 2, session.start_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind start_at failed");
        }

        if (session.end_at.has_value()) {
            if (sqlite3_bind_int64(stmt, 3, *session.end_at) != SQLITE_OK) {
                throw_sqlite_error(db, "bind end_at failed");
            }
        } else {
            if (sqlite3_bind_null(stmt, 3) != SQLITE_OK) {
                throw_sqlite_error(db, "bind null end_at failed");
            }
        }

        if (sqlite3_bind_int64(stmt, 4, session.id) != SQLITE_OK) {
            throw_sqlite_error(db, "bind id failed");
        }

        const int step_rc = sqlite3_step(stmt);
        if (step_rc != SQLITE_DONE) {
            throw_sqlite_error(db, "step update_session failed");
        }

        const int changed = sqlite3_changes(db);

        sqlite3_finalize(stmt);
        return changed;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

int finalize_session(sqlite3* db, int64_t id, int64_t end_at) {
    static constexpr const char* sql = R"sql(
            UPDATE sessions
            SET end_at = ?
            WHERE id = ?;
        )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare update_session failed");
    }

    try {
        if (sqlite3_bind_int64(stmt, 1, end_at) != SQLITE_OK) {
            throw_sqlite_error(db, "bind end_at failed");
        }

        if (sqlite3_bind_int64(stmt, 2, id) != SQLITE_OK) {
            throw_sqlite_error(db, "bind id failed");
        }

        const int step_rc = sqlite3_step(stmt);
        if (step_rc != SQLITE_DONE) {
            throw_sqlite_error(db, "step update_session failed");
        }

        const int changed = sqlite3_changes(db);

        sqlite3_finalize(stmt);
        return changed;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

void delete_sessions(sqlite3* db, const std::vector<int64_t>& ids) {
    if (ids.empty())
        return;

    std::string sql = "DELETE FROM sessions WHERE id IN (";

    for (size_t i = 0; i < ids.size(); ++i) {
        sql += (i == 0 ? "?" : ",?");
    }
    sql += ");";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw_sqlite_error(db, "prepare delete_sessions failed");
    }

    try {
        for (size_t i = 0; i < ids.size(); ++i) {
            if (sqlite3_bind_int64(stmt, static_cast<int>(i + 1), ids[i]) != SQLITE_OK) {
                throw_sqlite_error(db, "bind id failed");
            }
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            throw_sqlite_error(db, "delete_sessions step failed");
        }

        sqlite3_finalize(stmt);
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

std::vector<TaskModel> get_all_tasks(sqlite3* db) {
    static constexpr const char* sql = R"sql(
            SELECT
                id,
                created_at,
                updated_at,
                title,
                description,
                status
            FROM tasks
            ORDER BY created_at DESC;
        )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare get_all_tasks failed");
    }

    std::vector<TaskModel> result;
    result.reserve(30);

    try {
        while (true) {
            const int step_rc = sqlite3_step(stmt);

            if (step_rc == SQLITE_ROW) {
                TaskModel task;

                task.id = sqlite3_column_int64(stmt, 0);
                task.created_at = sqlite3_column_int64(stmt, 1);
                task.updated_at = sqlite3_column_int64(stmt, 2);

                const unsigned char* title = sqlite3_column_text(stmt, 3);
                const unsigned char* description = sqlite3_column_text(stmt, 4);

                task.title = title ? reinterpret_cast<const char*>(title) : "";
                task.description = description ? reinterpret_cast<const char*>(description) : "";

                task.status = static_cast<TaskStatus>(sqlite3_column_int(stmt, 5));

                result.push_back(std::move(task));
            } else if (step_rc == SQLITE_DONE) {
                break;
            } else {
                throw_sqlite_error(db, "step get_all_tasks failed");
            }
        }

        sqlite3_finalize(stmt);
        return result;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}

std::deque<TaskEntity> get_all_tasks_with_sessions(sqlite3* db) {
    static constexpr const char* sql = R"sql(
            SELECT
                t.id,
                t.created_at,
                t.updated_at,
                t.title,
                t.description,
                t.status,
                s.id,
                s.task_id,
                s.start_at,
                s.end_at
            FROM tasks t
            LEFT JOIN sessions s ON s.task_id = t.id
            ORDER BY t.id, s.start_at;
        )sql";

    sqlite3_stmt* stmt = nullptr;

    const int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw_sqlite_error(db, "prepare get_all_tasks failed");
    }

    std::deque<TaskEntity> result;


    try {
        int64_t last_task_id = -1;
        while (true) {
            const int step_rc = sqlite3_step(stmt);

            if (step_rc == SQLITE_ROW) {
                TaskModel task;

                task.id = sqlite3_column_int64(stmt, 0);
                task.created_at = sqlite3_column_int64(stmt, 1);
                task.updated_at = sqlite3_column_int64(stmt, 2);

                const unsigned char* title = sqlite3_column_text(stmt, 3);
                const unsigned char* description = sqlite3_column_text(stmt, 4);

                task.title = title ? reinterpret_cast<const char*>(title) : "";
                task.description = description ? reinterpret_cast<const char*>(description) : "";

                task.status = static_cast<TaskStatus>(sqlite3_column_int(stmt, 5));

                int session_type = sqlite3_column_type(stmt, 6);

                if (last_task_id != task.id) {
                    last_task_id = task.id;
                    result.emplace_back(TaskEntity{task, {}});
                }
                if (session_type != SQLITE_NULL) {
                    SessionModel session;
                    session.id = sqlite3_column_int64(stmt, 6);
                    session.task_id = sqlite3_column_int64(stmt, 7);
                    session.start_at = sqlite3_column_int64(stmt, 8);
                    if (sqlite3_column_type(stmt, 9) != SQLITE_NULL) {
                        session.end_at = sqlite3_column_int64(stmt, 9);
                    } else {
                        session.end_at = std::nullopt;
                    }
                    result.back().sessions.push_back(session);
                };
            } else if (step_rc == SQLITE_DONE) {
                break;
            } else {
                throw_sqlite_error(db, "step get_all_tasks failed");
            }
        }

        sqlite3_finalize(stmt);
        return result;
    } catch (...) {
        sqlite3_finalize(stmt);
        throw;
    }
}
} // namespace db
