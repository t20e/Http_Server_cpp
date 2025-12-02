#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>


class ThreadPool {
	private:
		std::vector<std::thread> workers_; // Store worker threads
		std::queue<std::function<void()>> tasks_; // Queue of tasks to be done
		std::mutex queue_mutex_; // Mutex to synchronize access to shared data.
		std::condition_variable condition_; // To single changes in the state of the tasks queue
		bool stop_;

	public:
		/**
         * @brief Construct launches the thread workers.
         * 
         * @param num_workers How many threads to start up.
         */
		ThreadPool(size_t num_workers);

		/**
         * @brief Destructor: Joins all threads.
         * 
         */
		~ThreadPool();

		/**
         * @brief Add a new task to the thread pool queue.
         * 
         * @tparam F Is the function type.
         * @tparam Args The arguments for the function.
         * @param args 
         */
		template<class F, class... Args> // Note: Templates must be declared in header files!
		void enqueue(F &&f, Args &&...args)
		{
			{
				std::unique_lock<std::mutex> lock(queue_mutex_); // Lock the queue to safely add a task. Locking means the other workers for the time being wont be able to read or write to the tasks queue.

				// Don't allow new tasks to be added if the program is shutting down.
				if (stop_)
					throw std::runtime_error("Attempted to enqueue on stopped ThreadPool!");

				// Wrap the function and arguments into a generic void lambda
				tasks_.emplace([func = std::bind(std::forward<F>(f), std::forward<Args>(args)...)]() {
					func();
				});
			}

            // Wake up one worker, to perform the task.
			condition_.notify_one(); 
		}
};
