#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Client.hpp"

Client::Client(const char* address, const char* port) :
	socket(writingContext),
	endpoints(asio::ip::tcp::resolver(writingContext).resolve(address, port)),
	writingThread([&] { writingContext.run(); })
	//processingThread([&] { processingContext.run(); })
{
	asio::co_spawn(writingContext, Connect(), asio::detached);
	//asio::co_spawn(processingContext, WriteHeader(), asio::detached);
}
Client::~Client() {
	writingContext.stop();
	processingContext.stop();
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
		co_await WriteHeader();
	}
}
asio::awaitable<void> Client::WriteHeader() {
	while (!socket.is_open()) {
		std::cout << "Connecting...\n";
	}
	while (true) {
		messages.wait();
		while (!messages.empty()) {
			auto msg = messages.front();
			messages.pop();
			auto [error, n] = co_await asio::async_write(socket, asio::buffer(&msg.header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
			if (error) {
				break;
			}
			else {
				std::cout << "Sent header: " << n << " bytes\n";
				if (msg.header.bodySize > 0) {
					co_await WriteBody(std::move(msg));
				}
			}
		}
	}
}
asio::awaitable<void> Client::WriteBody(Message<ExampleEnum> msg) {
	auto [error, n] = co_await asio::async_write(socket, asio::buffer(&msg.body, msg.header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		std::cerr << error.message();
	}
	else {
		
	}
}
asio::awaitable<void> Client::ReadHeader() {
	while (!socket.is_open()) {
		std::cout << "Connecting...\n";
	}
	while (true) {
		Message<ExampleEnum> msg;
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(&msg.header, sizeof(Message<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			std::cerr << error.message();
			break;
		}
		else {
			if (msg.header.bodySize > 0) {
				msg.body.resize(msg.header.bodySize); // this should be changed
				ReadBody(std::move(msg));
			}
			else {
				ProcessMessage(msg.body);
			}
		}
	}
}
asio::awaitable<void> Client::ReadBody(Message<ExampleEnum> msg) {
	auto [error, n] = co_await asio::async_read(socket, asio::buffer(&msg.body, msg.BodySize()), asio::experimental::as_tuple(asio::use_awaitable));
	if (error || n == 0) {
		std::cerr << error.message();
	}
	else {
		ProcessMessage(msg.body);
	}
}
void Client::ProcessMessage(std::vector<uint8_t> data) {
	for (auto& x : data) {
		std::cout << x;
	}
}
void Client::RegisterMessage(Message<ExampleEnum> msg) {
	messages.push(msg);
}