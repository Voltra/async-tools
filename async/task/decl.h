#pragma once
#include "fwd.h"
#include <async/stream/decl.h>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include <exception>

/**
 * A task that uses a stream to interact with the world
 * @tparam T - The type of data that flows in the stream associated to this task
 */
template <class T>
class async::task{
	public:
		using task_t = task<T>;
		using stream_t = stream<T>;
		using thread_t = std::thread;
		using thread_ptr_t = std::unique_ptr<thread_t>;
		using value_t = typename stream_t::value_type;
		using handler_t = std::function<void(task_t&, stream_t&)>;
		using shared_stream = typename stream_t::shared_stream;

		class stopping_task;

	protected:
		thread_ptr_t thread = nullptr;
		shared_stream stream_ptr{new stream_t{}};
		handler_t handler;
		std::atomic_bool running{false};

		void set_running(bool value){ this->running.store(value); }

	public:
		task() = delete;
		task(const task&) = delete;
		task& operator=(const task&) = delete;

		task(task&&) = default;
		task& operator=(task&&) = default;

		task(handler_t handler);

		bool is_running() const{ return this->running.load(); }
		task& run();
		task& stop() const;
		task& stop(const char*) const;
		task& wait();

		/**
		 * Get a shared_ptr to the stream associated to this task
		 * @return a shared_ptr to the stream associated to this task
		 */
		shared_stream stream() const{ return this->stream_ptr; }

		/**
		 * Interoperability with pointers to tasks
		 * @return a pointer to this task
		 */
		task* operator->(){ return this; }
};