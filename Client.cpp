#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Client.hpp"

					  Client::Client(const char* address, const char* port) :
	socket(writingContext),
	endpoints(asio::ip::tcp::resolver(writingContext).resolve(address, port)),
	writingThread([&] { writingContext.run(); }),
	processingThread([&] { processingContext.run(); })
{
	asio::co_spawn(writingContext, Connect(), asio::detached);
	asio::co_spawn(processingContext, Write(), asio::detached);
}
					  Client::~Client() {
	writingContext.stop();
	socket.close();
	std::cout << "Disconnected\n";
}
asio::awaitable<void> Client::Connect() {
	auto [error, result] = co_await asio::async_connect(socket, endpoints, asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		std::cerr << error.message();
	}
	else {
		std::cout << "Connected\n";
		co_await Read();
	}
}
asio::awaitable<void> Client::Read() {
	while (!socket.is_open()) {
		std::cout << "Connecting...\n";
	}
	while (true) {
		char data[5] = { 0 };
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(data, 5), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			std::cerr << error.message();
			break;
		}
		else {
			ProcessMessage(data);
		}
	}
}
asio::awaitable<void> Client::Write() {
	while (!socket.is_open()) {
		std::cout << "Connecting...\n";
	}
	while (true) {
		messages.wait();
		while (!messages.empty()) {
			auto& msg = messages.front();
			auto [error, n] = co_await asio::async_write(socket, asio::buffer(msg, sizeof(msg)), asio::experimental::as_tuple(asio::use_awaitable));
			if (error) {
				break;
			}
			else {

			}
			messages.pop();
		}
	}
}
void				  Client::ProcessMessage(char data[]) {
	std::cout << data;
}
void				  Client::RegisterMessage(std::string msg) {
	messages.push(msg);
}