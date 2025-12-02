#include <atomic>
#include <chrono>
#include <format>
#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "ThreadPool.h"

using std::cout;

std::mutex print_mutex;
std::atomic<int> completed_tasks{0};

/**
 * @brief Helper to make printing thread-safe, do texts don't scramble.
 * @param msg 
 */
void safe_print(const std::string &msg)
{
	std::lock_guard<std::mutex> lock(print_mutex);
	std::cout << msg << std::endl;
}

/**
 * @brief Simulate work by sleeping for 1 second.
 * @param id 
 * @param wait_time_second How long in seconds does the task take to complete.
 */
void heavy_task(int id, int wait_time_second)
{
	std::this_thread::sleep_for(std::chrono::seconds(wait_time_second));
	std::stringstream ss;
	ss << std::this_thread::get_id();

	safe_print(std::format("Task {} finished by Thread ID: {}", id, ss.str()));
	// increment counter
	completed_tasks++;
}


TEST(ThreadPoolTest, TestingHeavyTask)
{
	const int NUM_WORKERS = 4;
	const int NUM_TASKS = 8;
	const int task_time_sec = 1;

	cout << std::format("Creating Threadpool with {} workers.\n", NUM_WORKERS);
	ThreadPool pool(NUM_WORKERS);

	cout << std::format("Enqueuing {} tasks, each takes {} seconds.\n", NUM_TASKS, task_time_sec);

	auto start_time = std::chrono::high_resolution_clock::now();

	// Add tasks
	for (int i = 0; i < NUM_TASKS; ++i) {
		pool.enqueue([i] {
			heavy_task(i, task_time_sec);
		});
	}

	// Wait for tasks to finish.
	while (completed_tasks < NUM_TASKS) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_time = end_time - start_time;

	cout << std::format("Time elapsed: {} seconds.\n", elapsed_time);

	// Assert elapsed_time is (LESS THAN) < 3.0
	ASSERT_LT(elapsed_time.count(), 3.0);
	/*
    Logic: We have 4 threads/workers and 8 tasks, each task takes a thread 1 second to complete.
        In single thread this would take 8 seconds, i.e., only 1 thread open.
        In Multi-threaded should take ~2 seconds, 8/4 = 2.
    */
}
