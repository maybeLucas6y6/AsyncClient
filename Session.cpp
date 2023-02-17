#pragma once

#include <iostream>
#include <asio/experimental/as_tuple.hpp>
#include "Session.hpp"

Session::Session(asio::ip::tcp::socket s, asio::io_context& c, MutexQueue<std::string>& mq) :
	socket(std::move(s)),
	q(mq)
{
	//asio::co_spawn(c, Write(), asio::detached);
	//asio::co_spawn(c, Read(), asio::detached);
	asio::co_spawn(c, Transfer(), asio::detached);
	std::cout << "Started session\n";
}
Session::~Session() {
	socket.close();
	std::cout << "Stopped session\n";
}
asio::awaitable<void> Session::Write() {
	while (true) {
		auto [error, n] = co_await asio::async_write(socket, asio::buffer("123\n"), asio::experimental::as_tuple(asio::use_awaitable));
		if (error) {
			break;
		}
	}
}
asio::awaitable<void> Session::Read() {
	while (true) {
		char data[5] = { 0 };
		auto [error, n] = co_await asio::async_read(socket, asio::buffer(data, 5), asio::experimental::as_tuple(asio::use_awaitable));
		if (error || n == 0) {
			break;
		}
		else {
			//std::cout << data;
			q.push(data);
		}
	}
}
asio::awaitable<void> Session::Transfer() {
	while (true) {
		char data[512] = { 0 };
		strcpy_s(data, "67890");
		auto [e1, r1] = co_await asio::async_write(socket, asio::buffer(data, 5), asio::experimental::as_tuple(asio::use_awaitable));
		if (e1 || r1 == 0) {
			break;
		}
		else {
			std::cout << "Sent " << r1 << " bytes\n";
		}
		auto [e2, r2] = co_await asio::async_read(socket, asio::buffer(data, 5), asio::experimental::as_tuple(asio::use_awaitable));
		if (e2 || r2 == 0) {
			std::cout << e2.message();
			break;
		}
		else {
			//messages.push(data);
		}
	}
	std::cout << "broke\n";
}