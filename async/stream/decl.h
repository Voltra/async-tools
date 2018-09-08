#pragma once
#include "fwd.h"
#include <type_traits>
#include <atomic>
#include <functional>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <memory>
//#include <mingw.mutex.h>

/**
 * A class that represents an asynchronous data flow/stream
 * @tparam T - The type of data that flows in this stream
 */
template <class T>
class async::stream{
	public:
		using value_type = typename std::decay<T>::type;
		using mutex_type = std::mutex;
		using lock_guard = std::lock_guard<mutex_type>;
		using stream_type = async::stream<T>;
		using done_flag = std::atomic_bool;

		using listener_type = std::function<void(const value_type&)>;
		using listener_storage_type = std::vector<listener_type>;

		using exception = std::runtime_error;
		using shared_stream = std::shared_ptr<stream_type>;

		using close_listener_type = std::function<void()>;
		using close_listener_storage_type = std::vector<close_listener_type>;

	protected:
		mutex_type mutex{};
		done_flag closed{false};
		listener_storage_type listeners{};
		close_listener_storage_type closeListeners{};

	public:
		stream() = default;
		stream(const stream_type&);
		stream(stream_type&&) noexcept;
		~stream(){ this->close(); }

		stream& operator=(const stream_type&);
		stream& operator=(stream_type&&) noexcept;

	public:
		stream_type& addListener(listener_type);
		stream_type& onValue(listener_type);

		stream_type& emit(const value_type&);
		stream_type& operator<<(const value_type&);

		/**
		 * Interoperability with pointers to streams
		 * @return a pointer to this stream
		 */
		stream_type* operator->(){ return this; }

		template <class... Args>
		stream_type& emit(Args&&...);

		stream_type& onClose(close_listener_type);
		stream_type& close();

		void wait() const;

	public:
		stream_type& pipe(stream_type*);

		template <class Predicate>
		shared_stream filter(Predicate);

		template <class U, class Mapper>
		std::shared_ptr<stream<U>> map(Mapper);

		template <class U, class Mapper>
		std::shared_ptr<stream<U>> mapTo(Mapper);

		stream_type& peek(listener_type);

		void forEach(listener_type);

		template <class Reducer, class Accumulator>
		Accumulator reduce(Reducer, Accumulator);

		template <class Predicate>
		bool anyMatch(Predicate);

		template <class Predicate>
		bool allMatch(Predicate);

		template <class Predicate>
		bool noneMatch(Predicate);

	public:
		static constexpr const char* const ERR_STREAM_CLOSED = "Cannot emit a new value, the stream has already been closed";

		template <class... Args>
		static constexpr stream_type from(Args&&...);

		static constexpr stream_type make();
};