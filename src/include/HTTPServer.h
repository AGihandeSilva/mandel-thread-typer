#pragma once

#include <ApiDependencies.h>

#include "SynchronizedData.h"
#include <RESTEndpoint.h>
#include <RootEndpoint.h>
#include <BitmapEndpoint.h>
#include <map>

using RequestMapping = std::map < std::string, RESTEndpoint&>;

class HTTPSession : public std::enable_shared_from_this<HTTPSession>{
	tcp::socket m_socket;
	beast::flat_buffer m_buffer;
	http::request<http::string_body> m_req;
	std::shared_ptr<SynchronizedData<http::request<http::string_body>>> m_synchronizedInData;

public:
	explicit HTTPSession(tcp::socket socket,
		std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData);

	inline void run() { do_read(); }

private:

	http::response<http::string_body> handle_request(http::request<http::string_body> const& req,
		std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData);

	void do_read();
	void do_write(http::response <http::string_body > res);

	RootEndpoint m_rootEndpoint;
	BitmapEndpoint m_bitmapEndpoint;

	RequestMapping m_requestMapping;
};

class HTTPListener : public std::enable_shared_from_this<HTTPListener> {
	asio::io_context& m_ioc;
	tcp::acceptor m_acceptor;
	std::shared_ptr<SynchronizedData<http::request<http::string_body>>> m_synchronizedInData;


public:
	HTTPListener(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData);

	inline void run() { do_accept(); }

private:
	void do_accept();

};
