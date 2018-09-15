#pragma once
#include <async/stream/fwd.h>
#include <type_traits>
#include <atomic>
#include <functional>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <memory>

/**
 * A class that represents an asynchronous data flow/stream
 * @tparam T The type of data that flows in this stream
 */
template <class T>
class async::stream{
	public:
		using value_type = typename std::decay<T>::type;///< @typedef value_type being the type of value flowing into this stream
		using mutex_type = std::mutex;///< @typedef mutex_type being the type of mutex used to lock the stream
		using lock_guard = std::lock_guard<mutex_type>;///< @typedef lock_guard being the type of lock guard used to lock the stream
		using stream_type = async::stream<T>;///< @typedef stream_type being the type of this stream
		using done_flag = std::atomic_bool;///< @typedef done_flag being the type of the a flag for a completable action

		using listener_type = std::function<void(const value_type&)>;///< @typedef listener_type being the type of listeners used to handle new data
		using listener_storage_type = std::vector<listener_type>;///< @typedef listener_storage_type being the type of the container used to store listeners

		using exception = std::runtime_error;///< @typedef exception being the type of exception thrown when an unexpected error occurs
		using shared_stream = std::shared_ptr<stream_type>;///< @typedef shared_stream being the type that designates a shared pointer to a stream

		using close_listener_type = std::function<void()>;///< @typedef close_listener_type being the type of listeners used when the stream is closed
		using close_listener_storage_type = std::vector<close_listener_type>;///< @typedef close_listener_storage_type being the type of the container used to store on close listeners

	protected:
		mutex_type mutex{};///< @property mutex being the mutex used to lock the stream
		done_flag closed{false};///< @property closed being the flag used to determine whether or not this stream is closed
		listener_storage_type listeners{};///< @property listeners being the container of value listeners
		close_listener_storage_type closeListeners{};///< @property closeListeners being the container of on close listeners

	public:
		/**
		 * Default constructor that initializes a stream to a valid state
		 */
		stream() = default;

		/**
		 * Copy constructor
		 * @param other being the stream to copy from
		 */
		stream(const stream_type& other);


		/**
		 * Move constructor
		 * @param other being the stream to move from
		 */
		stream(stream_type&& other) noexcept;

		/**
		 * Destructor
		 */
		~stream(){ this->close(); }

		/**
		 * Copy assignment
		 * @param other being the stream to copy from
		 * @return a reference to this stream
		 */
		stream& operator=(const stream_type& other);

		/**
		 * Move assignment
		 * @param other being the stream to move from
		 * @return a reference to this stream
		 */
		stream& operator=(stream_type&& other) noexcept;

	public:
		/**
		 * @defgroup addListener
		 * @{
		 * Listen to the data coming into the stream
		 * @param listener being he listener that will receive each new value
		 * @return a reference to this stream
		 */
		stream_type& addListener(listener_type);

		stream_type& onValue(listener_type);
		/** @} */

		/**
		 * @defgroup emitting
		 * @{
		 * Push a new value down the stream
		 * @param value being the value to push down the stream
		 * @return a reference to this stream
		 *
		 * @pre This stream is not closed
		 * @post The value has been emitted
		 */
		stream_type& emit(const value_type& value);
		stream_type& operator<<(const value_type& value);
		/**  @} */

		/**
		 * Interoperability with pointers to streams
		 * @return a pointer to this stream
		 */
		stream_type* operator->(){ return this; }

		/**
		 * Construct the next value to be pushed down the stream and pushes it
		 * @tparam Args - The types of the arguments used in order to create the new data
		 * @param args - The arguments used to construct the new value
		 * @return a reference to this stream
		 *
		 * @pre This stream is not closed
		 * @post The value has been emitted
		 */
		template <class... Args>
		stream_type& emit(Args&&... args);

		/**
		 * Add a callback to be executed when the stream is closed
		 * @param listener - The listener that will be executed once the stream is closed
		 * @return a reference to this stream
		 */
		stream_type& onClose(close_listener_type listener);

		/**
		 * Closes this stream
		 * @return a reference to this stream
		 *
		 * @pre The stream is not closed
		 * @post The stream is closed
		 */
		stream_type& close();

		/**
		 * Waits until the stream is closed
		 * @warning introduces a blocking call
		 *
		 * @post The stream is closed
		 */
		void wait() const;

	public:
		/**
		 * Pipes a stream to this stream (functions like "ls | grep" in bash)
		 * @param stream - The stream to pipe into this one
		 * @return a reference to this stream
		 */
		stream_type& pipe(stream_type* stream);

		/**
		 * Filter this stream according to the given predicate
		 * @tparam Predicate - Predicate :: (const value_type&) -> bool
		 * @param predicate - The predicate indicating whether it should be filtered away (false) or not (false)
		 * @return a shared_ptr to the filtered stream
		 */
		template <class Predicate>
		shared_stream filter(Predicate predicate);

		/**
		 * @defgroup mapping
		 * @{
		 * Maps this stream into a stream of another type using the mapper
		 * @tparam U - The type of data that will flow in the mapped stream
		 * @tparam Mapper - Mapper :: (const value_type&) -> U
		 * @param mapper - The mapper function used to map each incoming element
		 * @return a shared_ptr to the mapped stream
		 */
		template <class U, class Mapper>
		std::shared_ptr<stream<U>> map(Mapper mapper);

		template <class U, class Mapper>
		std::shared_ptr<stream<U>> mapTo(Mapper mapper);
		/** @} */

		/**
		 * Invoke a function on each element of this stream
		 * @param listener being the function to invoke on each element
		 * @return a reference to this stream
		 */
		stream_type& peek(listener_type listener);

		/**
		 * Invoke a function on each element of this stream
		 * @param listener being the function to invoke on each element
		 */
		void forEach(listener_type listener);

		/**
		 * Reduces the stream to a single value
		 * @tparam Reducer - Reducer :: (Accumulator, const value_type&) -> Accumulator
		 * @tparam Accumulator - The type of the final desired value
		 * @param reducer - The function used to reduce the stream to a single value
		 * @param start - The initial value of the accumulator used to reduce the stream to a single value
		 * @return the reduced value
		 *
		 * @warning This is a blocking call that waits until this stream is closed
		 * @post The stream is closed
		 */
		template <class Reducer, class Accumulator>
		Accumulator reduce(Reducer, Accumulator);

		/**
		 * Tests whether or not any element of this stream matches the given predicate
		 * @tparam Predicate - Predicate :: (const value_type&) -> bol
		 * @param predicate - The predicate to match against each element
		 * @return true if any matches, false if none
		 *
		 * @warning Uses async::stream<T>::reduce and therefore introduces a blocking call
		 * @post The stream is closed
		 */
		template <class Predicate>
		bool anyMatch(Predicate);

		/**
		 * Tests whether or not every element of this stream matches the given predicate
		 * @tparam Predicate - Predicate :: (const value_type&) -> bool
		 * @param predicate - The predicate to match
		 * @return true if every element matches, false otherwise
		 *
		 * @warning Uses async::stream<T>::reduce and therefore introduces a blocking call
		 * @post The stream is closed
		 */
		template <class Predicate>
		bool allMatch(Predicate);

		/**
		 * Tests whether or not none of this stream's element
		 * @tparam Predicate - Predicate :: (const value_type&) -> bool
		 * @param predicate - The predicate to test against
		 * @return true if none matches, false otherwise
		 *
		 * @warning Uses async::stream<T>::reduce and therefore introduces a blocking call
		 * @post The stream is closed
		 */
		template <class Predicate>
		bool noneMatch(Predicate);

	public:
		/**
		 * @property ERR_STREAM_CLOSED The error message used when attempting to push a new value onto a closed stream
		 */
		static constexpr const char* const ERR_STREAM_CLOSED = "Cannot emit a new value, the stream has already been closed";

		/**
		 * Creates a stream using the given arguments
		 * @tparam T The type of data that flows in this stream
		 * @tparam Args The argument's types
		 * @param args The arguments to use in order to construct the stream
		 * @return the created stream
		 */
		template <class... Args>
		static constexpr stream_type from(Args&&... args);

		/**
		 * Creates a stream without arguments
		 * @tparam T - The type of data that flows in this stream
		 * @return the created stream
		 */
		static constexpr stream_type make();
};
