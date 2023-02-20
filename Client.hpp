#pragma once

#include <asio.hpp>
#include <thread>
#include "ExampleEnum.hpp"
#include "Message.hpp"
#include "MutexQueue.hpp"

class Client {
private:
	std::jthread writingThread;
	asio::io_context writingContext;
	std::jthread processingThread;
	asio::io_context processingContext;
	asio::ip::tcp::socket socket;
	asio::ip::basic_resolver_results<asio::ip::tcp> endpoints;
	Message<ExampleEnum> message;
	MutexQueue<Message<ExampleEnum>> messages; // rename this
	bool isConnected;
public:
	Client(const char* address, const char* port);
	~Client();
	bool IsConnected() const;
	void RegisterMessage(const Message<ExampleEnum>& msg);
private:
	asio::awaitable<void> Connect();
	asio::awaitable<void> WriteHeader();
	asio::awaitable<void> WriteBody();
	asio::awaitable<void> ReadHeader();
	asio::awaitable<void> ReadBody();
protected:
	void ProcessMessage(Message<ExampleEnum> msg);
};