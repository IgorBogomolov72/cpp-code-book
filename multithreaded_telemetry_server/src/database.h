#pragma once

#include "device.h"
#include <mutex>
#include <condition_variable>
#include <optional>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <queue>
#include <atomic>

class DataBase {
public:
    DataBase();
    ~DataBase();

    void InsertReadingDataDevice(DeviceState read_data_device);

private:
    sqlite3* db_;
    std::mutex queue_mutex_;
    std::queue<DeviceState> queue_;
    std::condition_variable cv_;
    std::thread worker_;
    std::atomic<bool> stop_;
    int rc_;                      // Сохраняем результат открытия базы
    char* messaggeError_;

    void WorkerThread();
    void CreateTable();
    void Insert(const DeviceState&& r);
    void CheckDbError();
};
