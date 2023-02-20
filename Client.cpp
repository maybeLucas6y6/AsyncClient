#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Client.hpp"

template<class T> Client<T>::Client(const char* address, const char* port) :
	socket(writingContext),
	endpoints(asio::ip::tcp::resolver(writingContext).resolve(address, port)),
	writingThread([&] { writingContext.run(); }),
	processingThread([&] { processingContext.run(); })
{
	isConnected = false;
	asio::co_spawn(writingContext, Connect(), asio::detached);
	asio::co_spawn(processingContext, ReadHeader(), asio::detached);
}
template<class T> Client<T>::~Client() {
	writingContext.stop();
	processingContext.stop();
	socket.cancel();
	socket.close();
	isConnected = false;
	std::cout << "Disconnected\n";
}
template<class T> bool Client<T>::IsConnected() const {
	return isConnected;
}
template<class T> void Client<T>::RegisterMessage(const Message<T>& msg) {
	messages.push(msg);
}
template<class T> asio::awaitable<void> Client<T>::Connect() {
	auto [error, result] = co_await asio::async_connect(socket, endpoints, asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		// change stuff here regarding isConnected(timeout)
		std::cerr << error.message();
	}
	else {
		std::cout << "Connected\n";
		isConnected = true;
		co_await WriteHeader();
	}
}
template<class T> asio::awaitable<void> Client<T>::WriteHeader() {
	while (isConnected) {
		messages.wait();
		while (!messages.empty()) {
			auto [error, n] = co_await asio::async_write(socket, asio::buffer(&messages.front().header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
			if (error) {
				std::cerr << error.message() << "\n";
				socket.cancel();
				socket.close();
				isConnected = false;
				break;
			}
			else {
				if (messages.front().header.bodySize > 0) {
					co_await WriteBody();
				}
				else {
					ProcessMessage(messages.pop());
				}
			}
		}
	}
}
template<class T> asio::awaitable<void> Client<T>::WriteBody() {
	auto [error, n] = co_await asio::async_write(socket, asio::buffer(messages.front().body.data(), messages.front().header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		std::cerr << error.message() << "\n";
		socket.cancel();
		socket.close();
		isConnected = false;
	}
	else {
		ProcessMessage(messages.pop());
	}
}
template<class T> asio::awaitable<void> Client<T>::ReadHeader() {
	while (!isConnected) {
		std::cout << "Connecting...\n";
	}
	while (isConnected) {
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(&message.header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			std::cerr << error.message() << "\n";
			socket.cancel();
			socket.close();
			isConnected = false;
			break;
		}
		else {
			if (message.header.bodySize > 0) {
				message.body.resize(message.header.bodySize);
				co_await ReadBody();
			}
			else {
				ProcessMessage(message);
			}
		}
	}
}
template<class T> asio::awaitable<void> Client<T>::ReadBody() {
	auto [error, n] = co_await asio::async_read(socket, asio::buffer(message.body.data(), message.header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error || n == 0) {
		std::cerr << error.message() << "\n";
		socket.cancel();
		socket.close();
		isConnected = false;
	}
	else {
		ProcessMessage(message);
	}
}
template<class T> void Client<T>::ProcessMessage(Message<T> msg) {
	ExampleStruct s;
	msg >> s;
	std::cout << "Processed: ";
	std::cout << s.a << " " << s.b;
	std::cout << "\n";
}