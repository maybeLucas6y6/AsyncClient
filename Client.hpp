#pragma once

#include <asio.hpp>
#include <asio/experimental/as_tuple.hpp>
#include <iostream>
#include <thread>
#include "Message.hpp"
#include "MutexQueue.hpp"

template<class T> class Client {
private:
	const char* address;
	const char* port;
	asio::io_context ctx;
	std::jthread thr;
	MutexQueue<Message<T>> incomingMessages;
private:
	asio::ip::tcp::socket socket;
	Message<T> message;
	MutexQueue<Message<T>> messagesRegistered;
	bool isConnected;
public:
	Client(const char* address_, const char* port_);
	~Client();
	bool IsConnected() const;
	void RegisterMessage(const Message<T>& msg);
private:
	asio::awaitable<void> Connect();
	asio::awaitable<void> WriteHeader();
	asio::awaitable<void> WriteBody();
	asio::awaitable<void> ReadHeader();
	asio::awaitable<void> ReadBody();
protected:
	void ProcessMessage(Message<T> msg);
};

template<class T> Client<T>::Client(const char* address_, const char* port_) :
	socket(ctx),
	address(address_),
	port(port_),
	thr([&]() { asio::co_spawn(ctx, Connect(), asio::detached); ctx.run(); })
{
	isConnected = false;
}
template<class T> Client<T>::~Client() {
	socket.cancel();
	socket.close();
	ctx.stop();
	std::cout << "Disconnected\n";
}
template<class T> bool Client<T>::IsConnected() const {
	return isConnected;
}
template<class T> void Client<T>::RegisterMessage(const Message<T>& msg) {
	bool free = messagesRegistered.empty();
	messagesRegistered.push(msg);
	if (free) {
		asio::co_spawn(ctx, WriteHeader(), asio::detached);
	}
}
template<class T> asio::awaitable<void> Client<T>::Connect() {
	auto r = asio::ip::tcp::resolver(ctx).resolve(asio::ip::tcp::resolver::query(address, port));
	auto [error, result] = co_await asio::async_connect(socket, r, asio::experimental::as_tuple(asio::use_awaitable));
	if (error) {
		// change stuff here regarding isConnected(timeout)
		std::cerr << error.message();
	}
	else {
		std::cout << "Connected\n";
		isConnected = true;
		co_await ReadHeader();
	}
}
template<class T> asio::awaitable<void> Client<T>::WriteHeader() {
	while (!messagesRegistered.empty()) {
		auto [error, n] = co_await asio::async_write(socket, asio::buffer(&messagesRegistered.front().header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			std::cerr << error.message() << "\n";
			isConnected = false;
			break;
		}
		else {
			if (messagesRegistered.front().header.bodySize > 0) {
				co_await WriteBody();
			}
			else {
				ProcessMessage(messagesRegistered.pop());
			}
		}
	}
}
template<class T> asio::awaitable<void> Client<T>::WriteBody() {
	auto [error, n] = co_await asio::async_write(socket, asio::buffer(messagesRegistered.front().body.data(), messagesRegistered.front().header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	if (error || n == 0) {
		std::cerr << error.message() << "\n";
		isConnected = false;
	}
	else {
		ProcessMessage(messagesRegistered.pop());
	}
}
template<class T> asio::awaitable<void> Client<T>::ReadHeader() {
	while (isConnected) {
		std::cout << "Waiting for header\n";
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(&message.header, sizeof(MessageHeader<ExampleEnum>)), asio::experimental::as_tuple(asio::use_awaitable));
		std::cout << "Received header: " << n << " bytes: " << message.header.bodySize << "\n";
		if (error || n == 0) {
			std::cerr << error.message() << "\n";
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
	std::cout << "Waiting for body\n";
	auto [error, n] = co_await asio::async_read(socket, asio::buffer(message.body.data(), message.header.bodySize), asio::experimental::as_tuple(asio::use_awaitable));
	std::cout << "Received body: " << n << " bytes: " << message.BodySize() << "\n";
	if (error || n == 0) {
		std::cerr << error.message() << "\n";
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