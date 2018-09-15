#pragma once
#include <async/task/decl.h>
#include <async/stream/stream.hpp>

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

TPL
self& self::run(){
	if(this->is_running())
		return *this;

	this->runner = this->make_runner([&]{
		try{
			self& task = *this;
			self_t::stream_t& stream = *(this->stream_ptr);
			this->handler(task, stream);
		}catch(const self_t::stopping_task& e){
			this->stop_internals();
			#ifdef ASYNC_TASK_DEBUG
			std::cerr << e.what() << '\n';
			#endif
		}catch(...){
			this->stop_internals();
			throw; //aka rethrow the last exception thrown
		}
	});

	this->set_running(true);
	return *this;
}

TPL
self& self::stop(){
	if(!this->is_running())
		return *this;

	this->stop_internals();
	throw stopping_task{};
}

TPL
self& self::stop(const char* cstr){
	if(!this->is_running())
		return *this;

	this->stop_internals();
	throw stopping_task{cstr};
}

TPL
void self::stop_internals(){
	this->set_running(false);
	this->stream_ptr->close();
}

TPL
self& self::wait(){
	if(!this->is_running())
		return *this;

	if(this->runner)
		this->runner->get();

	return *this;
}

TPL
template <class F>
self_t::runner_ptr_t self::make_runner(F handler){
	auto handled = std::async(
		std::launch::async,
		handler
	);

	return self_t::runner_ptr_t{
		new self_t::runner_t(std::move(handled))
	};
}

TPL
constexpr self::stopping_task::stopping_task() : _what{"stopping task"} {
}

TPL
constexpr self::stopping_task::stopping_task(const char* cstr) : _what{cstr} {
}

TPL
const char* self::stopping_task::what() const noexcept { return this->_what; }

#undef constructor
#undef self
#undef self_t
#undef TPL