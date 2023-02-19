#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Client.hpp"

Client::Client(const char* address, const char* port) :
	socket(writingContext),
	endpoints(asio::ip::tcp::resolver(writingContext).resolve(address, port)),
	writingThread([&] { writingContext.run(); })
	//processingThread([&] { processingContext.run(); })
{
	//message.body.resize(32); // this should be changed
	asio::co_spawn(writingContext, Connect(), asio::detached);
	//asio::co_spawn(processingContext, ReadHeader(), asio::detached);
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
				std::cout << "Sent header: " << n << " bytes\n";
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
		std::cout << "Sent " << n << " bytes: " << messages.front() << "\n";
		ExampleStruct s{ 0,0 };
		//message >> s;
		std::vector<uint8_t> v = messages.front().body;
		std::memcpy(&s, v.data(), sizeof(ExampleStruct));
		std::cout << "Contents: " << s.a << " " << s.b << "\n";
	}
	messages.pop();
}
asio::awaitable<void> Client::ReadHeader() {
	while (!socket.is_open()) {
		std::cout << "Connecting...\n";
	}
	while (true) {
		Sleep(2000);
		//Message<ExampleEnum> msg;
		//messagesIn.push(std::move(msg));
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(&message.header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			std::cerr << error.message() << "*\n";
			break;
		}
		else {
			if (message.header.bodySize > 0) {
				message.body.resize(message.header.bodySize); // this should be changed
				co_await ReadBody();
			}
			else {
				//ProcessMessage(msg.body);
				//messagesIn.pop();
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
		std::cout << message;
		/*for (auto& x : message.body) {
			std::cout << x;
		}*/
		ExampleStruct s{0,0};
		message >> s;
		std::cout << "-" << s.a << "-" << s.b << "-";
	}
	//messagesIn.pop();
}
void Client::ProcessMessage(std::vector<uint8_t> data) {
	for (auto& x : data) {
		std::cout << x;
	}
}
void Client::RegisterMessage(Message<ExampleEnum> msg) {
	messages.push(msg);

	//ExampleStruct s{ 0,0 };
	////message >> s;
	//std::vector<uint8_t> v = messages.front().body;
	//std::memcpy(&s, v.data(), sizeof(ExampleStruct));
	//std::cout << "Contents: " << s.a << " " << s.b << "\n";
}