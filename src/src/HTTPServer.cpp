#include <HTTPServer.h>
#include <random>
#include <nlohmann/json.hpp>
#include <SynchronizedData.h>
#include <RootEndpoint.h>
#include <BitmapEndpoint.h>
	

http::response<http::string_body> HTTPSession::handle_request(http::request<http::string_body> const& req,
	std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData)
{
	auto req_path = std::string(req.target());

	http::response<http::string_body> res; //TODO

	if (m_requestMapping.contains(req_path))
	{
		RESTEndpoint& processor = m_requestMapping.at(req_path);

		res = processor.ProcessRequest(req, synchronizedData);
	}

	return res;
}

HTTPSession::HTTPSession(tcp::socket socket, std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData) :
	m_socket(std::move(socket)), m_synchronizedInData(synchronizedData),
	
	m_requestMapping {
		{"/", m_rootEndpoint},
		{"/bitmap", m_bitmapEndpoint },
	}
	{}

void HTTPSession::do_read() {
	auto self(shared_from_this());

	http::async_read(m_socket, m_buffer, m_req, 
		[this, self](beast::error_code ec, std::size_t) {
		if (ec == http::error::end_of_stream) {
			beast::error_code ignored;
			m_socket.shutdown(tcp::socket::shutdown_send, ignored);
			return;
		}

		if (!ec) {
			do_write(handle_request(m_req, m_synchronizedInData));
		}
	});
}

void HTTPSession::do_write(http::response <http::string_body> res)
{
	auto self(shared_from_this());
	auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
	http::async_write(m_socket, *sp, [this, self, sp](beast::error_code ec, std::size_t) {
		if (ec) {
			return;
		}

		if (m_req.keep_alive()) {
			m_req = {};
			m_buffer.consume(m_buffer.size());
			do_read();
			return;
		}

		beast::error_code ignored;
		m_socket.shutdown(tcp::socket::shutdown_send, ignored);
	});
}

HTTPListener::HTTPListener(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData)
	: m_ioc(ioc), m_acceptor((ioc)), m_synchronizedInData(synchronizedData)
{
	beast::error_code ec;

	m_acceptor.open(endpoint.protocol(), ec);
	if (ec) {
		std::cerr << "Error: could not open " << ec.message() << std::endl;
		return;
	}

	m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);

	if (ec) {
		std::cerr << "Set option error: " << ec.message() << std::endl;
		return;
	}

	m_acceptor.bind(endpoint, ec);

	if (ec) {
		std::cerr << "Bind error: " << ec.message() << std::endl;
		return;
	}

	//start listening
	m_acceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec) {
		std::cerr << "Listen error: " << ec.message() << std::endl;
		return;
	}

}

void HTTPListener::do_accept()
{
	auto self = shared_from_this();
	m_acceptor.async_accept(asio::make_strand(m_ioc),
		[self, this](beast::error_code ec, tcp::socket socket) {
		if (!ec) {
			std::make_shared<HTTPSession>(std::move(socket), m_synchronizedInData)->run();
		}
		self->do_accept();
	});
}
