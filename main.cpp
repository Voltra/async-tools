#include <iostream>
#include <fstream>
#include <async/async.hpp>
#include <functional>
#include "utils.h"

namespace console{
	namespace wrap = surrounded;

	namespace w{
		namespace wrap = surrounded;
	}
}
namespace cli = console;
using namespace std::placeholders;

template <class Str=std::string>
void streamFile(const char* path, async::task<Str>& task, async::stream<Str>& stream){
	std::ifstream file{path};

	if(!file.is_open())
		task.stop("Could not open file");

	while(file.good()){
		Str tmp;
		std::getline(file, tmp);
		stream << tmp;
	}

	if(!file.eof())
		task.stop("Stopped reading before EOF");
}

void testLoremIpsum(){
	/*async::task<std::string> task([](async::task<std::string>& task, async::stream<std::string>& stream){
		streamFile("../test.txt", task, stream);
	});*/
	async::task<std::string> task(std::bind(
		streamFile<>,
		"../test.txt",
		_1,
		_2
	));

	task->stream()
	->filter(is_not_whitespace)
	->forEach(cli::wrap::log("$[> ", " <]$\n"));

	task->run()->wait();
}

void testNumber(){
	async::task<std::string> task(std::bind(
		streamFile<>,
		"../num.txt",
		_1,
		_2
	));

	task->stream()
	->filter(is_not_whitespace)
	->filter(is_int<>)
	->mapTo<int>(to_int<>)
	->forEach(cli::wrap::log<int>("$> ", " <$"));

	task->run()->wait();
}

int main() {
//	testLoremIpsum();
	testNumber();
	return 0;
}