#pragma once
#include <async/stream/decl.h>
#include <utility>
#include <future>
#include <algorithm>
#include <functional>
#include <memory>
#include <condition_variable>

#define TPL template <class T>
#define constructor stream
#define self async::stream<T>
#define self_t typename self
#define LOCK self_t::lock_guard _{this->mutex};
#define IF_CLOSED_THROW if(this->closed.load())\
  throw self_t::exception(self::ERR_STREAM_CLOSED);

TPL
self::constructor(const self_t::stream_type& other) : constructor() {
	*this = other;
}

TPL
self::constructor(self_t::stream_type&& other) noexcept : constructor() {
	*this = std::forward<decltype(other)>(other);
}

TPL
self/*_t::stream_type*/& self::operator=(const self_t::stream_type& other){
	this->closed.store(other.closed.load());
	this->listeners = other.listeners;
	this->closeListeners = other.closeListeners;
	return *this;
}

TPL
template <class... Args>
constexpr self_t::stream_type self::from(Args&&... args){
	return self_t::stream_type{
		std::forward<Args&&>(args)...
	};
}

TPL
constexpr self_t::stream_type self::make(){
	return self::from();
}

TPL
self& self::operator=(self_t::stream_type&& other) noexcept{
	this->closed.store(other.closed.load());
	this->listeners = std::move(other.listeners);
	this->closeListeners = std::move(other.closeListeners);
	return *this;
}

TPL
self& self::addListener(self_t::listener_type listener){
	LOCK
	this->listeners.push_back(listener);
	return *this;
}

TPL
self_t::stream_type& self::onValue(self_t::listener_type listener){
	return this->addListener(listener);
}

TPL
self_t::stream_type& self::emit(const self_t::value_type& value){
	IF_CLOSED_THROW

	std::async(std::launch::async, [=]{
		LOCK
		for(const auto& listener : this->listeners)
			listener(value);
	});

	return *this;
}

TPL
self_t::stream_type& self::operator<<(const self_t::value_type& value){
	return this->emit(value);
}

TPL
template <class... Args>
self_t::stream_type& self::emit(Args&&... args){
	IF_CLOSED_THROW

	std::async(std::launch::async, [=]{
		LOCK
		self_t::value_type value{args...};
		for(const auto& listener : this->listeners)
			listener(value);
	});

	return *this;
}

TPL
self_t::stream_type& self::onClose(self_t::close_listener_type listener){
	LOCK
	this->closeListeners.push_back(listener);
	return *this;
}

TPL
self_t::stream_type& self::close(){
	if(!this->closed.load()){
		this->closed.store(true);
		std::async(std::launch::async, [=]{
			LOCK
			for(auto f : this->closeListeners)
				f();
		});
	}

	return *this;
}

TPL
void self::wait() const{
	if(this->closed.load())
		return;

	std::condition_variable cv;
	this->onClose([&]{ cv.notify_one(); });
	std::unique_lock<std::mutex> lock{this->mutex};
	cv.wait(lock, [&]{ return this->closed.load(); });
}

TPL
self_t::stream_type& self::pipe(stream_type* stream){
	return this->onValue([=](const value_type& value){
		stream->emit(value);
	});
}

TPL
template <class Predicate>
self_t::shared_stream self::filter(Predicate predicate){
	shared_stream filtered{new stream_type()};

	this->onValue([=](const value_type& value){
		if(predicate(value))
			filtered->emit(value);
	});

	this->onClose([=]{
		filtered->close();
	});

	return filtered;
}

TPL
template <class U, class Mapper>
std::shared_ptr<async::stream<U>> self::map(Mapper mapper){
	std::shared_ptr<async::stream<U>> mapped{new async::stream<U>{}};

	this->onValue([=](const value_type& value){
		mapped->emit(
			mapper(value)
		);
	});

	this->onClose([=]{
		mapped->close();
	});

	return mapped;
}

TPL
template <class U, class Mapper>
std::shared_ptr<async::stream<U>> self::mapTo(Mapper mapper){
	return this->map<U>(mapper);
}

TPL
self_t::stream_type& self::peek(self_t::listener_type listener){
	return this->onValue(listener);
}

TPL
void self::forEach(self_t::listener_type listener){
	this->peek(listener);
}

TPL
template <class Reducer, class Accumulator>
Accumulator self::reduce(Reducer reducer, Accumulator start){
	Accumulator acc = start;
	this->onValue([&](const value_type& value){
		acc = reducer(acc, value);
	});
	this->wait();
	return acc;
}

TPL
template <class Predicate>
bool self::anyMatch(Predicate predicate){
	return this->reduce([&](bool acc, const value_type& value){
		return acc || predicate(value);
	}, false);
}

TPL
template <class Predicate>
bool self::allMatch(Predicate predicate){
	return this->reduce([&](bool acc, const value_type& value){
		return acc && predicate(value);
	}, true);
}

TPL
template <class Predicate>
bool self::noneMatch(Predicate predicate){
	return !this->anyMatch(predicate);
}


#undef TPL
#undef constructor
#undef self
#undef self_t
#undef LOCK
#undef IF_CLOSED_THROW
