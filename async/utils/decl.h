#pragma once
#include <async/stream/decl.h>

namespace async{
	template <class T, class... Args>
	stream<T> make_stream(Args&&... args);
}