//
//  ThreadPool.hpp
//
//  Created by Roald Christesen on 22.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Core/ThreadPool.hpp"
#include "String/String.hpp"


namespace Grain {


    /**
     *  @brief Constructs a ThreadPool with a specified number of worker threads.
     *
     *  @param thread_count The number of threads to create in the thread pool.
     */
    ThreadPool::ThreadPool(size_t thread_count) : m_stop_flag(false) {
        m_completed_count.store(0);
        for (size_t i = 0; i < thread_count; ++i) {
            m_workers.emplace_back(&ThreadPool::workerThread, this);
        }
    }


    /**
     *  @brief Destroys the ThreadPool and stops all running threads.
     *
     *  Ensures that all threads complete their current tasks before shutting down.
     */
    ThreadPool::~ThreadPool() {
        stop(true);
    }


    /**
     *  @brief Enqueues a task into the thread pool for execution.
     *
     *  This function adds a task to the internal task queue along with a task
     *  identifier. It ensures thread-safe access to the queue and notifies one of
     *  the worker threads that a new task is available.
     *
     *  @param task The task to be enqueued. It must conform to the ThreadPoolTask
     *              type, typically a callable object or a functor.
     *  @param task_id An integer identifier for the task, used for tracking or
     *                 logging purposes.
     *
     *  @note This function is thread-safe. It uses a mutex to synchronize access to
     *        the queue and a condition variable to notify a worker thread.
     */
    void ThreadPool::enqueueTask(const ThreadPoolTask& task, int32_t task_id) {
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_task_queue.emplace(task, task_id);
        }
        m_condition.notify_one();
    }


    /**
     *  @brief Stops all threads in the thread pool.
     *
     *  Signals all threads to stop and waits for them to complete their current
     *  tasks.
     */
    void ThreadPool::stop(bool immediate_flag) {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop_flag = true;
        m_stop_immediate_flag = immediate_flag;
        m_condition.notify_all();

        lock.unlock();

        for (std::thread& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        m_completed_count.store(0);
    }


    /**
     *  @brief Waits until the specified number of tasks have been completed.
     *
     *  This function periodically checks the number of completed tasks and waits
     *  until it reaches the specified target count.
     *
     *  @param task_count The total number of tasks to wait for before continuing.
     *
     *  @note This function does not use condition variables for waiting,
     *        and instead relies on periodic polling with a fixed delay.
     *        It is marked noexcept and assumes that `completed_count` is
     *        being updated atomically or under proper synchronization.
     */
    void ThreadPool::waitForCompletion(int32_t task_count) noexcept {
        while (task_count > static_cast<int32_t>(m_completed_count)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_completion_sleep_ms));
        }
    }


    /**
     *  @brief Retrieves the progress or result associated with a specific task.
     *
     *  This function looks up the result of a task identified by `task_id` in the
     *  internal results map. If found, the result string is copied to the output
     *  parameter and the function returns true. Otherwise, it returns false.
     *
     *  @param task_id The identifier of the task whose result is being queried.
     *  @param out_result A reference to a string that will be set to the task's
     *                    result if found.
     *
     *  @return true if the task result was found and assigned to `result`,
     *          false otherwise.
     *
     *  @note This function is thread-safe. It uses a mutex to ensure synchronized
     *        access to the results map.
     */
    bool ThreadPool::progress(int32_t task_id, String& out_result) {
        std::lock_guard<std::mutex> lock(m_results_mutex);
        auto it = m_results.find(task_id);
        if (it != m_results.end()) {
            out_result = it->second.c_str();
            return true;
        }

        return false;
    }


    /**
     *  @brief The function executed by each worker thread.
     *
     *  Continuously fetches and executes tasks from the task queue until signaled
     *  to stop.
     */
    void ThreadPool::workerThread() {
        while (true) {
            std::pair<ThreadPoolTask, int> item;

            {
                std::unique_lock<std::mutex> lock(m_queue_mutex);

                m_condition.wait(lock, [this] {
                    return m_stop_flag || !m_task_queue.empty();
                });

                if (m_stop_flag && m_task_queue.empty()) {
                    return;  // clean exit
                }

                if (m_stop_immediate_flag) {
                    // Optionally skip remaining tasks
                    return;
                }

                item = std::move(m_task_queue.front());
                m_task_queue.pop();
            }

            try {
                item.first.execute();
            }
            catch (const std::exception& e) {
                // Log exception and continue
                std::cerr << "Exception in task execution: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknown exception in task execution" << std::endl;
            }

            {
                std::lock_guard<std::mutex> lock(m_results_mutex);
                m_results[item.second] = "Task " + std::to_string(item.second) + " completed.";
                m_completed_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

} // End of namespace Grain
