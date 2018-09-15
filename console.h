#pragma once
#include <iostream>
#include <functional>

namespace console{
	template <class T=std::string>
	decltype(std::cout)& log(T value){
		return std::cout << value << '\n';
	}

	template <class T=std::string>
	decltype(std::cerr)& error(T value){
		return std::cerr << value << '\n';
	}

	namespace prefixed{
		template <class T=std::string>
		std::function<decltype(std::cout)&(const T&)> log(const char* prefix){
			decltype(std::cout)* cout = &std::cout;
			return [=](const T& value) -> decltype(std::cout)&{
				return *cout << prefix << value << '\n';
			};
		}

		template <class T=std::string>
		std::function<decltype(std::cerr)&(const T&)> error(const char* prefix){
			decltype(std::cerr)* cerr = &std::cerr;
			return [=](const T& value) -> decltype(std::cerr)&{
				return *cerr << prefix << value << '\n';
			};
		}
	}

	namespace surrounded{
		template <class T=std::string>
		std::function<decltype(std::cout)&(const T&)> log(const char* prefix, const char* suffix){
			decltype(std::cout)* cout = &std::cout;
			return [=](const T& value) -> decltype(std::cout)&{
				return *cout << prefix << value << suffix << '\n';
			};
		}

		template <class T=std::string>
		std::function<decltype(std::cerr)&(const T&)> error(const char* prefix, const char* suffix){
			decltype(std::cerr)* cerr = &std::cerr;
			return [=](const T& value) -> decltype(std::cerr)&{
				return *cerr << prefix << value << suffix <<  '\n';
			};
		}
	}
};

namespace console{
	namespace w{
		template <class T=std::wstring>
		decltype(std::wcout)& log(T value){
			return std::wcout << value << '\n';
		}

		template <class T=std::wstring>
		decltype(std::wcerr)& error(T value){
			return std::wcerr << value << '\n';
		}

		namespace prefixed{
			template <class T=std::wstring>
			std::function<decltype(std::wcout)&(const T&)> log(const char* prefix){
				decltype(std::wcout)* wcout = &std::wcout;
				return [=](const T& value) -> decltype(std::wcout)&{
					return *wcout << prefix << value << '\n';
				};
			}

			template <class T=std::wstring>
			std::function<decltype(std::wcerr)&(const T&)> error(const char* prefix){
				decltype(std::wcerr)* wcerr = &std::wcerr;
				return [=](const T& value) -> decltype(std::wcerr)&{
					return *wcerr << prefix << value << '\n';
				};
			}
		}

		namespace surrounded{
			template <class T=std::wstring>
			std::function<decltype(std::wcout)&(const T&)> log(const char* prefix, const char* suffix){
				decltype(std::wcout)* wcout = &std::wcout;
				return [=](const T& value) -> decltype(std::wcout)&{
					return *wcout << prefix << value << suffix << '\n';
				};
			}

			template <class T=std::wstring>
			std::function<decltype(std::wcerr)&(const T&)> error(const char* prefix, const char* suffix){
				decltype(std::wcerr)* wcerr = &std::wcerr;
				return [=](const T& value) -> decltype(std::wcerr)&{
					return *wcerr << prefix << value << suffix <<  '\n';
				};
			}
		}
	}
};

namespace console{
	namespace wrap = surrounded;

	namespace w{
		namespace wrap = surrounded;
	}
}

namespace cli = console;