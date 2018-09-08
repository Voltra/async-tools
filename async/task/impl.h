#pragma once
#include "decl.h"
#include <async/stream/impl.h>

#ifdef ASYNC_TASK_DEBUG
#include <iostream>
#endif


#define constructor task
#define self async::task<T>
#define self_t typename self
#define TPL template <class T>

TPL
self::constructor(self_t::handler_t handler) : handler{handler}{
}

/**
 * Starts the execution of this task
 * @tparam T - The type of data that flows in the stream associated to this task
 * @return a reference to this task
 */
TPL
self& self::run(){
	if(this->is_running())
		return *this;

	this->thread = self_t::thread_ptr_t{new self_t::thread_t([&]{
		try{
			self& task = *this;
			self_t::stream_t& stream = *(this->stream_ptr);
			this->handler(task, stream);
		}catch(const self_t::stopping_task& e){
			#ifdef ASYNC_TASK_DEBUG
			std::cerr << e.what() << '\n';
			#endif
		}
	})};
	this->set_running(true);

	return *this;
}

/**
 * Stops this task mid-execution without a specific error message
 * @tparam T - The type of data that flows in the stream associated to this task
 * @return a reference to this task
 */
TPL
self& self::stop() const{
	throw stopping_task{};
}

/**
 * Stops this task mid-execution with the specified error message
 * @tparam T - The type of data that flows in this stream
 * @param cstr - The error message used to stop the stream associated to this task
 * @return a reference to this task
 * @throws async::task::stopping_task
 */
TPL
self& self::stop(const char* cstr) const{
	throw stopping_task{cstr};
}

/**
 * Waits for the completion of this task
 * @tparam T - The type of data that flows in the stream associated to this task
 * @return a reference to this task
 */
TPL
self& self::wait(){
	if(!this->is_running())
		return *this;

	if(this->thread)
		this->thread->join();

	return *this;
}

/**
 * An exception class used to stop a task
 * @tparam T - The type of data that flows in the stream associated to this stream
 */
TPL
class self::stopping_task final : public std::exception{
	protected:
		const char* _what;
	public:
		constexpr stopping_task() : _what{"stopping task"} {
		}

		constexpr stopping_task(const char* cstr) : _what{cstr} {
		}

		const char* what() const noexcept override{ return this->_what; }
};

#undef constructor
#undef self
#undef self_t
#undef TPL