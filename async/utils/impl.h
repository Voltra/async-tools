#pragma once
#include "decl.h"
#include <utility>

template <class T, class... Args>
async::stream<T> async::make_stream(Args&&... args){
	return stream<T>::from(std::forward<Args>(args)...);
}