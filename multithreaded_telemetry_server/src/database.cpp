#include "database.h"

#include <iostream>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <sqlite3.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

DataBase::DataBase()
    : stop_(false) {
    rc_ = sqlite3_open("example.db", &db_);
    CheckDbError();
    CreateTable();
    worker_ = std::thread(&DataBase::WorkerThread, this);
}

DataBase::~DataBase() {
    stop_ = true;
    cv_.notify_one();
    worker_.join();
    sqlite3_close(db_);
}

void DataBase::InsertReadingDataDevice(DeviceState read_data_device) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.push(read_data_device);
    }
    cv_.notify_one();
}

void DataBase::WorkerThread() {
    while (!stop_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stop_ ;});
        if (stop_ && queue_.empty()) {
            break;
        }
        DeviceState r = std::move(queue_.front());
        queue_.pop();
        lock.unlock();

        Insert(std::move(r));
    }
}

void DataBase::CreateTable(){
    std::string sql = "CREATE TABLE IF NOT EXISTS sensor_data("
                      "ID INT PRIMARY KEY, "
                      "DEVICE_ID    CHAR(50)    NOT NULL, "
                      "TIMESTAMP    INTEGER    NOT NULL, "
                      "TEMPERATURE  REAL    NOT NULL, "
                      "HUMIDITY     REAL    NOT NULL, "
                      "PRESSURE     REAL    NOT NULL);";
    rc_ = sqlite3_exec( // int                                          Возврат
        db_,            // sqlite3*,                                    Открытая база данных
        sql.c_str(),    // const char *sql,                             SQL-запрос для выполнения
        NULL,           // int (*callback)(void*,int,char**,char**),    Функция обратного вызова 
        0,              // void *,                                      1-й аргумент для функции обратного вызова
        &messaggeError_ // char **errmsg                                Здесь записывается сообщение об ошибке
    );

    CheckDbError();
}
void DataBase::Insert(const DeviceState&& r) {
    std::time_t tt = std::chrono::system_clock::to_time_t(r.last_update_); 
    std::string sql = "INSERT INTO sensor_data (device_id, timestamp, temperature, humidity, pressure) VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, r.device_id_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, tt);
    sqlite3_bind_double(stmt, 3, r.temperature_);
    sqlite3_bind_double(stmt, 4, r.humidity_);
    sqlite3_bind_double(stmt, 5, r.pressure_);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void DataBase::CheckDbError() {
    if (rc_) {
        // Show an error message
        std::cerr << "DB Error: " << sqlite3_errmsg(db_) << std::endl;

        sqlite3_close(db_);
    }
}
