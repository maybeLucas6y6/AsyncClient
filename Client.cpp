#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Client.hpp"

Client::Client(const char* address, const char* port) :
	socket(writingContext),
	endpoints(asio::ip::tcp::resolver(writingContext).resolve(address, port)),
	writingThread([&] { writingContext.run(); }),
	processingThread([&] { processingContext.run(); })
{
	isConnected = false;
	asio::co_spawn(writingContext, Connect(), asio::detached);
	asio::co_spawn(processingContext, ReadHeader(), asio::detached);
}
Client::~Client() {
	writingContext.stop();
	processingContext.stop();
	socket.cancel();
	socket.close();
	isConnected = false;
	std::cout << "Disconnected\n";
}
asio::awaitable<void> Client::Connect() {
	auto [error, result] = co_await asio::async_connect(socket, endpoints, asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		std::cerr << error.message();
	}
	else {
		std::cout << "Connected\n";
		isConnected = true;
		co_await WriteHeader();
	}
}
asio::awaitable<void> Client::WriteHeader() {
	while (isConnected) {
		messages.wait();
		while (!messages.empty()) {
			auto [error, n] = co_await asio::async_write(socket, asio::buffer(&messages.front().header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
			if (error) {
				std::cerr << error.message() << "1\n";
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
					std::cout << "Sent: ";
					ProcessMessage(messages.pop());
					std::cout << "\n";
				}
			}
		}
	}
}
asio::awaitable<void> Client::WriteBody() {
	auto [error, n] = co_await asio::async_write(socket, asio::buffer(messages.front().body.data(), messages.front().header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		std::cerr << error.message() << "2\n";
		socket.cancel();
		socket.close();
		isConnected = false;
	}
	else {
		std::cout << "Sent: ";
		ProcessMessage(messages.pop());
		std::cout << "\n";
	}
}
asio::awaitable<void> Client::ReadHeader() {
	while (!isConnected) {
		std::cout << "Connecting...\n";
	}
	while (isConnected) {
		std::cout << "Started reading header...\n";
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(&message.header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		std::cout << "Read " << n << " bytes\n";
		if (error || n == 0) {
			std::cerr << error.message() << "3\n";
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
				std::cout << "Received: ";
				ProcessMessage(message);
				std::cout << "\n";
			}
		}
	}
}
asio::awaitable<void> Client::ReadBody() {
	std::cout << "Started reading body...\n";
	auto [error, n] = co_await asio::async_read(socket, asio::buffer(message.body.data(), message.header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error || n == 0) {
		std::cerr << error.message() << "4\n";
		socket.cancel();
		socket.close();
		isConnected = false;
	}
	else {
		std::cout << "Received: ";
		ProcessMessage(message);
		std::cout << "\n";
	}
}
void Client::ProcessMessage(Message<ExampleEnum> msg) {
	ExampleStruct s;
	msg >> s;
	std::cout << s.a << " " << s.b;
}
void Client::RegisterMessage(const Message<ExampleEnum>& msg) {
	messages.push(msg);
}