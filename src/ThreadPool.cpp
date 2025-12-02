#include <condition_variable>
#include <format>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "./ThreadPool.h"
#include "utils/Logger.h"


ThreadPool::ThreadPool(size_t num_workers)
	: stop_(false)
{
	Logger::getInstance().log(LogLevel::INFO, std::format("Initializing thread pool with {} workers", num_workers));

	for (size_t i = 0; i < num_workers; ++i) {
		workers_.emplace_back(( // emplace_back creates a std::thread object and puts it in a vector.
			[this] {
				while (true) { // The worker entires an infinite loop.
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex_); // The queue is a shared resource. Only one thread can access it at a time.

						// Wait until there is a new task, or we are terminating the program
						this->condition_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });

						// If there are no tasks, and we are terminating the program then kill thread.
						if (this->stop_ && this->tasks_.empty()) {
							return;
						}

						// Move the task from the queue and add to the worker's current task
						task = std::move(this->tasks_.front());
						this->tasks_.pop(); //remove that task from queue.
					}

					//Execute the tasks
					task();
				}
			}));
	}
}

ThreadPool::~ThreadPool()
{
	{
		// Lock the queue to safely change 'stop_'
		std::unique_lock<std::mutex> lock(queue_mutex_);
		stop_ = true;
	}

	// Notify all threads that we are terminating
	condition_.notify_all();

	// Wait for every worker to finish their current task, and then join them all.
	for (std::thread &worker: workers_)
		worker.join();
}
