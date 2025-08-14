//
//  ThreadPool.hpp
//
//  Created by Roald Christesen on 22.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.08.2025
//

#ifndef GrainThreadPool_hpp
#define GrainThreadPool_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <unordered_map>


namespace Grain {

    /**
     *  @brief Represents a task to be executed by the ThreadPool.
     *
     *  Encapsulates a callable function or lambda to be executed by a worker thread.
     */
    struct ThreadPoolTask {

        ThreadPoolTask() {}

        /**
         *  @brief The function to be executed by the task.
         *  This can be any callable object (e.g., lambda, function pointer, or
         *  std::function).
         */
        std::function<void()> work;

        /**
         *  @brief Constructs a Task with a given callable function.
         *  @param func The function or lambda to execute as the task.
         */
        ThreadPoolTask(std::function<void()> func) : work(std::move(func)) {}

        /**
         *  @brief Executes the encapsulated task function.
         *  If the task function is valid, it will be executed. Otherwise, this
         *  method does nothing.
         */
        void execute() {
            if (work) {
                work();
            }
        }
    };


    /**
     *  @brief A thread pool implementation for managing and executing tasks.
     *
     *  The ThreadPool class manages a fixed number of worker threads to execute a
     *  queue of tasks. Tasks are enqueued and executed by the first available
     *  thread. The pool supports dynamic task addition and graceful shutdown.
     */
    class ThreadPool : public Object {
    public:
        explicit ThreadPool(size_t thread_count);
        ~ThreadPool();

        void enqueueTask(const ThreadPoolTask& task, int32_t task_id);
        void stop(bool immediate_flag = false);

        bool progress(int32_t task_id, String& out_result);
        [[nodiscard]] size_t completedCount() const { return m_completed_count; }
        [[nodiscard]] bool isImmediateStopMode() const { return m_stop_immediate_flag; }

        void incrementCompletedCount() {
            m_completed_count.fetch_add(1, std::memory_order_relaxed);
            std::cout << "m_completed_count: " << m_completed_count << std::endl;
        }

        void waitForCompletion(int32_t task_count) noexcept;

    private:
        void workerThread();

    protected:
        std::vector<std::thread> m_workers;             ///< Threads in the pool
        std::queue<std::pair<ThreadPoolTask, int>> m_task_queue;  ///< Task queue
        std::unordered_map<int, std::string> m_results; ///< Map task ID to its result
        std::mutex m_queue_mutex;                       ///< Mutex for task queue
        std::mutex m_results_mutex;
        std::condition_variable m_condition;            ///< Condition variable for synchronization
        std::atomic<bool> m_stop_flag;                  ///< Flag to stop threads
        std::atomic<bool> m_stop_immediate_flag;        ///< Flag to stop threads immediately
        std::atomic<size_t> m_completed_count = 0;
        int32_t m_completion_sleep_ms = 50;             ///< idle time in waitForCompletion()

    public:
        std::mutex m_log_mutex;
    };


} // End of namespace Grain

#endif // GrainThreadPool_hpp
