#pragma once

#include "HTTPServer.h"

class RESTEndpoint {


public: 
	virtual http::response<http::string_body> ProcessRequest(http::request<http::string_body> const& req,
		std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData) = 0;
};