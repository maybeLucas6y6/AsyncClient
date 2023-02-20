#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ASIO_STANDALONE
#include <asio.hpp>
#include <thread>
#include "MutexQueue.hpp"
#include "Message.hpp"
#include "ExampleEnum.hpp"

class Client {
public:
	bool isConnected;
	MutexQueue<Message<ExampleEnum>> messages;
private:
	std::jthread writingThread;
	asio::io_context writingContext;
	std::jthread processingThread;
	asio::io_context processingContext;
	asio::ip::tcp::socket socket;
	asio::ip::basic_resolver_results<asio::ip::tcp> endpoints;
	Message<ExampleEnum> message;
public:
	Client(const char* address, const char* port);
	~Client();
	asio::awaitable<void> Connect();
	asio::awaitable<void> ReadHeader();
	asio::awaitable<void> ReadBody();
	asio::awaitable<void> WriteHeader();
	asio::awaitable<void> WriteBody();
	void RegisterMessage(Message<ExampleEnum> msg);
	void ProcessMessage(Message<ExampleEnum> msg);
};