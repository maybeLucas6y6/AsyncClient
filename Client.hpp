#pragma once

#include <thread>
#include <asio.hpp>
#include "MutexQueue.hpp"
#include "Message.hpp"
#include "ExampleEnum.hpp"

class Client {
private:
	std::jthread writingThread;
	asio::io_context writingContext;
	std::jthread processingThread;
	asio::io_context processingContext;
	asio::ip::tcp::socket socket;
	asio::ip::basic_resolver_results<asio::ip::tcp> endpoints;
	MutexQueue<Message<ExampleEnum>> messages;
public:
	Client(const char* address, const char* port);
	~Client();
	asio::awaitable<void> Connect();
	asio::awaitable<void> ReadHeader();
	asio::awaitable<void> ReadBody(Message<ExampleEnum> msg);
	asio::awaitable<void> WriteHeader();
	asio::awaitable<void> WriteBody(Message<ExampleEnum> msg);
	void RegisterMessage(Message<ExampleEnum> msg);
	void ProcessMessage(std::vector<uint8_t> data);
};