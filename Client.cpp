#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Client.hpp"

Client::Client(const char* address, const char* port) :
	socket(writingContext),
	endpoints(asio::ip::tcp::resolver(writingContext).resolve(address, port)),
	writingThread([&] { writingContext.run(); }),
	processingThread([&] { processingContext.run(); })
{
	message.body.resize(32); // this should be changed
	asio::co_spawn(writingContext, Connect(), asio::detached);
	asio::co_spawn(processingContext, ReadHeader(), asio::detached);
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
	while (true) {
		messages.wait();
		while (!messages.empty()) {
			auto [error, n] = co_await asio::async_write(socket, asio::buffer(&messages.front().header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
			if (error) {
				break;
			}
			else {
				if (messages.front().header.bodySize > 0) {
					co_await WriteBody();
				}
				else {
					messages.pop();
				}
			}
		}
	}
}
asio::awaitable<void> Client::WriteBody() {
	auto [error, n] = co_await asio::async_write(socket, asio::buffer(messages.front().body.data(), messages.front().header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		std::cerr << error.message();
	}
	else {
		
	}
	messages.pop();
}
asio::awaitable<void> Client::ReadHeader() {
	while (!socket.is_open()) {
		std::cout << "Connecting...\n";
	}
	Sleep(2000); // solve this
	while (true) {
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(&message.header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			std::cerr << error.message() << "\n";
			break;
		}
		else {
			if (message.header.bodySize > 0) {
				message.body.resize(message.header.bodySize); // this should be changed
				co_await ReadBody();
			}
			else {
				//ProcessMessage(msg.body);
			}
		}
	}
}
asio::awaitable<void> Client::ReadBody() {
	auto [error, n] = co_await asio::async_read(socket, asio::buffer(message.body.data(), message.header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error || n == 0) {
		std::cerr << error.message();
	}
	else {
		//ProcessMessage(msg.body);
	}
}
void Client::ProcessMessage(std::vector<uint8_t> data) {
	
}
void Client::RegisterMessage(Message<ExampleEnum> msg) {
	messages.push(msg);
}