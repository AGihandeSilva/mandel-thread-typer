#pragma once

#include <ApiDependencies.h>

#include <RESTEndpoint.h>

#include <random>

#include <nlohmann/json.hpp>
#include <SynchronizedData.h>


class RootEndpoint : public RESTEndpoint {


public:
	http::response<http::string_body> ProcessRequest(http::request<http::string_body> const& req,
		std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData) override {
		if (req.method() == http::verb::get)
		{
			http::response<http::string_body> res{ http::status::ok, req.version() };
			res.set(http::field::server, "Beast");
			res.set(http::field::content_type, "text/plain");
			res.keep_alive(req.keep_alive());

			std::random_device rd;
			std::default_random_engine engine{ rd() };
			std::uniform_int_distribution ints{};

			auto req_body = req.body();
			auto req_path = std::string(req.target());
			auto req_method = std::string(http::to_string(req.method()));

			nlohmann::json body = nlohmann::json::parse(req_body, nullptr, false);

			const std::string bodyString = body.is_discarded() ? "(None)" : body.dump(0);

			nlohmann::json result;

			if (!req_path.empty())
			{
				result["body"] = req_body;
				result["path"] = req_path;
				result["method"] = req_method;



				if (synchronizedData)
				{
					synchronizedData->PushMessage(req);
				}

				res.body() = result.dump(0);
				res.prepare_payload();
				return res;
			}
		}
	http::response<http::string_body> res{ http::status::method_not_allowed, req.version() };
	res.set(http::field::server, "Beast");
	res.keep_alive(req.keep_alive());
	res.prepare_payload();
	return res;
}
};