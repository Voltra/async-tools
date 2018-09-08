# async-tools

## What is it?

async-tools is a tiny C++11 library that aims toward making asynchrony easy in C++. It provides a variety of tools that will help you achieve that.



It provides a lot of classes and helper functions under the namespace `async`

## Tools

### stream

`async::stream<T>` is a one-way non-owning data flow : It takes the data and simply passes it around without keeping it. The design is based around the principle of sensors next to a river, the sensor grabs the newest data and makes it available for processing but doesn't hold it or block it : the data just passes in front of it.



It shares similar traits with [Java8's Stream API](https://docs.oracle.com/javase/8/docs/api/java/util/stream/package-summary.html) even though it serves a totally different purpose : Java's Stream API is mainly used to ease collection manipulation operations while `async::stream<T>` should be used to have a uniform way to asynchronously manipulate a (supposedly) non-finite one-way data flow.

Obviously you could use `async::stream<T>` to manipulate a container of `T`s but that would probably be a bit more expensive than using a library that is specifically made for this single purpose (such as [range-v3](https://github.com/ericniebler/range-v3)).



Like Java's `Stream`, `async::stream<T>` provides a way to manipulate the data flow easily (eg. `async::stream<T>::forEach`, `async::stream<T>::filter`, `async::stream<T>::mapTo<U>`).



### task

At first sight, streams might seem tedious to use since you need to setup listeners before the work actually starts (if you don't want to miss anything). This is why I provide the `async::task<T>`, it is a rather simple wrapper around `std::thread` that has a `async::stream<T>` ready for you to use. You can then `async::task<T>::run` (and `async::task<T>::stop`) the task (eg. reading a file line by line and processing each line individually).



## Example

```c++
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <functional>
#include <async/async.hpp>

using namespace std;
using namespace std::placeholders;
#define ASYNC_TASK_DEBUG

bool is_whitespace(const string& str){
    static regex emptystr{"^\\s*$"};
    return regex_search(str, emptystr);
}

bool is_not_whitespace(const std::string& str){
    return !is_whitespace(str);
}

template <class Str=string>
void streamFile(const char* path, async::task<Str>& task, async::stream<Str>& stream){
	ifstream file{path};
	
	if(!file.is_open())
		task->stop("Could not open file");
		
        while(file.good()){
        	Str buffer;
        	std::getline(file, buffer);
        	stream << buffer; //only a line at a time is in memory
        }
        
        if(!file.eof())
        	task->stop("Stopped reading file before EOF");
}

template <class T=string>
void print_it(const T& t){
    cout << "$> " << t << " <$";
}

int main(){
	const/*expr*/ char* PATH = "kittens.txt";
	
	async::task<string> task{bind(
		streamFile<>,
		PATH, _1, _2
	)};
	
	task->stream()
	->filter(is_not_whitespace)
    ->forEach(print_it<>);
    
    task->run()->wait();
}
```



