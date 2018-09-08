#pragma once
#include "decl.h"
#include <utility>
#include <future>
#include <algorithm>
#include <functional>
#include <memory>
#include <condition_variable>
//#include <mingw.condition_variable.h>

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
self_t::stream_type& self::operator=(const self_t::stream_type& other){
	this->closed.store(other.closed.load());
	this->listeners = other.listeners;
	this->closeListeners = other.closeListeners;
	return *this;
}

/**
 * Creates a stream using the given arguments
 * @tparam T - The type of data that flows in this stream
 * @tparam Args - The argument's types
 * @param args - The arguments to use in order to construct the stream
 * @return the created stream
 */
TPL
template <class... Args>
constexpr self_t::stream_type self::from(Args&&... args){
	return self_t::stream_type{
		std::forward<Args&&>(args)...
	};
}

/**
 * Creates a stream without arguments
 * @tparam T - The type of data that flows in this stream
 * @return the created stream
 */
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


/**
 * Listen to the data coming into the stream
 * @tparam T - The type of data that flows in this stream
 * @param listener - The listener that will receive each new value
 * @return a reference to this stream
 */
TPL
self& self::addListener(self_t::listener_type listener){
	LOCK
	this->listeners.push_back(listener);
	return *this;
}

/**
 * An alias for async::stream::addListener
 * @see async::stream::addListener
 */
TPL
self_t::stream_type& self::onValue(self_t::listener_type listener){
	return this->addListener(listener);
}

/**
 * Push a new value down the stream
 * @tparam T - The type of data that flows in this stream
 * @param value - The value to push down the stream
 * @return a reference to this stream
 */
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

/**
 * The stream input operator version of async::stream::emit
 * @see async::stream::emit
 */
TPL
self_t::stream_type& self::operator<<(const self_t::value_type& value){
	return this->emit(value);
}

/**
 * Construct the next value to be pushed down the stream and pushes it
 * @tparam T - The type of data that flows in this stream
 * @tparam Args - The types of the arguments used in order to create the new data
 * @param args - The arguments used to construct the new value
 * @return a reference to this stream
 */
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

/**
 * Add a callback to be executed when the stream is closed
 * @tparam T - The type of data that flows in this stream
 * @param listener - The listener that will be executed once the stream is closed
 * @return a reference to this stream
 */
TPL
self_t::stream_type& self::onClose(self_t::close_listener_type listener){
	LOCK
	this->closeListeners.push_back(listener);
	return *this;
}

/**
 * Closes this stream
 * @tparam T - The type of data that flows in this stream
 * @return a reference to this stream
 */
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

/**
 * Waits until the stream is closed
 * @tparam T  - The type of data that flows in this stream
 */
TPL
void self::wait() const{
	if(this->closed.load())
		return;

	std::condition_variable cv;
	this->onClose([&]{ cv.notify_one(); });
	std::unique_lock<std::mutex> lock{this->mutex};
	cv.wait(lock, [&]{ return this->closed.load(); });
}

/**
 * Pipes a stream to this stream (functions like "ls | grep" in bash)
 * @tparam T - The type of data that flows in this stream and the given stream
 * @param stream - The stream to pipe into this one
 * @return a reference to this stream
 */
TPL
self_t::stream_type& self::pipe(stream_type* stream){
	return this->onValue([=](const value_type& value){
		stream->emit(value);
	});
}

/**
 * Filter this stream according to the given predicate
 * @tparam T - The type of data that flows in this stream
 * @tparam Predicate - Predicate :: (const value_type&) -> bool
 * @param predicate - The predicate indicating whether it should be filtered away (false) or not (false)
 * @return a shared_ptr to the filtered stream
 */
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


/**
 * Maps this stream into a stream of another type using the mapper
 * @tparam T - The type of data that flows in this stream
 * @tparam U - The type of data that will flow in the mapped stream
 * @tparam Mapper - Mapper :: (const value_type&) -> U
 * @param mapper - The mapper function used to map each incoming element
 * @return a shared_ptr to the mapped stream
 */
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

/**
 * Alias for async::stream::map
 * @see async::stream::map
 */
TPL
template <class U, class Mapper>
std::shared_ptr<async::stream<U>> self::mapTo(Mapper mapper){
	return this->map<U>(mapper);
}

/**
 * Alias for async::stream::onValue
 * @see async::stream::onValue
 */
TPL
self_t::stream_type& self::peek(self_t::listener_type listener){
	return this->onValue(listener);
}

/**
 * Alias for async::stream::peek
 * @see async::stream::forEach
 */
TPL
void self::forEach(self_t::listener_type listener){
	this->peek(listener);
}

/**
 * Reduces the stream to a single value
 * @tparam T - The type of data that flows in this stream
 * @tparam Reducer - Reducer :: (Accumulator, const value_type&) -> Accumulator
 * @tparam Accumulator - The type of the final desired value
 * @param reducer - The function used to reduce the stream to a single value
 * @param start - The initial value of the accumulator used to reduce the stream to a single value
 * @return the reduced value
 *
 * @warning This is a blocking call that waits until this stream is closed
 */
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

/**
 * Tests whether or not any element of this stream matches the given predicate
 * @tparam T - The type of data that flows in this stream
 * @tparam Predicate - Predicate :: (const value_type&) -> bol
 * @param predicate - The predicate to match against each element
 * @return true if any matches, false if none
 *
 * @warning Uses async::stream::reduce and therefore introduces a blocking call
 */
TPL
template <class Predicate>
bool self::anyMatch(Predicate predicate){
	return this->reduce([&](bool acc, const value_type& value){
		return acc || predicate(value);
	}, false);
}

/**
 * Tests whether or not every element of this stream matches the given predicate
 * @tparam T - The type of data that flows in this stream
 * @tparam Predicate - Predicate :: (const value_type&) -> bool
 * @param predicate - The predicate to match
 * @return true if every element matches, false otherwise
 */
TPL
template <class Predicate>
bool self::allMatch(Predicate predicate){
	return this->reduce([&](bool acc, const value_type& value){
		return acc && predicate(value);
	}, true);
}

/**
 * Tests whether or not none of this stream's element
 * @tparam T - The type of data that flows in this stream
 * @tparam Predicate - Predicate :: (const value_type&) -> bool
 * @param predicate - The predicate to test against
 * @return true if none matches, false otherwise
 */
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