#pragma once
#include <async/stream/decl.h>

namespace async{
	/**
	 * A helper function that creates a stream from the arguments that will be passed to the constructor
	 * @tparam T The type of data that will flow into the stream
	 * @tparam Args The types of the arguments
	 * @param args being the arguments to pass to the constructor
	 * @return the desired stream
	 */
	template <class T, class... Args>
	stream<T> make_stream(Args&&... args);
}