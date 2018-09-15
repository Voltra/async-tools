#pragma once
#include <async/task/fwd.h>
#include <async/stream/decl.h>
//#include <thread>
#include <future>
#include <functional>
#include <memory>
#include <atomic>
#include <exception>

/**
 * A task that uses a stream to interact with the world
 * @tparam T The type of data that flows in the stream associated to this task
 */
template <class T>
class async::task{
	public:
		using task_t = task; ///< @typedef task_t being the type of this task
		using stream_t = stream<T>; ///< @typedef stream_t being the type of stream associated to tasks
		using runner_t = std::future<void>/*std::thread*/; ///< @typedef runner_t being the type used to run the task
		using runner_ptr_t = std::unique_ptr<runner_t>; ///< @typedef runner_ptr_t being the ptr type to the task runner
		using value_t = typename stream_t::value_type; ///< @typedef value_t being the type of values streamed
		using handler_t = std::function<void(task_t&, stream_t&)>; ///< @typedef handler_t being the type of handler to provided to the task
		using shared_stream = typename stream_t::shared_stream; ///< @typedef shared_stream being the type of shared stream associated to this task

		/**
		 * An exception class used to stop a task
		 */
		class stopping_task final : public std::exception{
			protected:
				/**
				 * @property _what The error message
				 */
				const char* const _what;

			public:
				/**
				 * Default constructor (w/ default error message)
				 */
				constexpr stopping_task();

				/**
				 * Constructor to use for a custom error message
				 * @param cstr Why the task has been stopped
				 */
				constexpr stopping_task(const char* cstr);

				/**
				 * Retrieve the information on why this exception was thrown
				 * @return
				 */
				const char* what() const noexcept override;
		};

	protected:
		runner_ptr_t runner = nullptr; ///< @property thread being the task launcher
		shared_stream stream_ptr{new stream_t{}}; ///< @property stream_ptr being the pointer to the stream of this task
		handler_t handler; ///< @property handler being the handler for this task
		std::atomic_bool running{false}; ///< @property running being the flag determining whether or not the task has been completed

		/**
		 * Setter for the running flag
		 * @param value being the new value for the flag
		 */
		void set_running(bool value){ this->running.store(value); }

		/**
		 * Crafts the runner from its handler
		 * @tparam F The function type of the handler
		 * @param handler The handler used when the task is run
		 * @return the created runner as a pointer
		 */
		template <class F>
		runner_ptr_t make_runner(F handler);

	public:
		task() = delete;
		task(const task&) = delete;
		task& operator=(const task&) = delete;

		/**
		 * Move constructor
		 * @param other being the task to move from
		 */
		task(task&& other) noexcept = default;

		/**
		 * Move assignment
		 * @param other being the task to move from
		 * @return a reference to this task
		 */
		task& operator=(task&& other) noexcept = default;

		/**
		 * Construct a task from its handler
		 * @param handler being the function to invoke in order to execute the task
		 */
		task(handler_t handler);

		/**
		 * Determine whether or not this task is running
		 * @return TRUE if running, FALSE otherwise
		 */
		bool is_running() const{ return this->running.load(); }

		/**
		 * Starts the execution of this task
		 * @return a reference to this task
		 *
		 * @pre This task is not running, this task has not been stopped
		 * @post This task is running, this task can be stopped
		 */
		task& run();

		/**
		 * Stops this task mid-execution without a specific error message
		 * @return a reference to this task
		 * @throws async::task<T>::stopping_task
		 * @warning closes the associated stream
		 *
		 * @pre This task is running
		 * @post This task is not running, this task has been stopped
		 */
		task& stop();

		/**
		 * Stops this task mid-execution with the specified error message
		 * @param cstr - The error message used to stop the stream associated to this task
		 * @return a reference to this task
		 * @throws async::task<T>::stopping_task
		 * @warning closes the associated stream
		 *
		 * @pre This task is running
		 * @post This task is not running, this task has been stopped
		 */
		task& stop(const char* cstr);

		/**
		 * Waits for the completion (or failure) of this task
		 * @return a reference to this task
		 *
		 * @pre This task is running
		 * @post This task is not running
		 */
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

	protected:
		/**
		 * Modify the internals of this task to be in a stopped state
		 * @warning closes the associated stream
		 *
		 * @pre This task is running
		 * @post This task state has been set to a stopped state
		 */
		void stop_internals();
};