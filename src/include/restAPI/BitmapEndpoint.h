#pragma once

#include <ApiDependencies.h>

#include <RESTEndpoint.h>
#include <SynchronizedData.h>
#include <QBuffer>
#include <QPixmap>


class BitmapEndpoint : public RESTEndpoint {


public:
	http::response<http::string_body> ProcessRequest(http::request<http::string_body> const& req,
		std::shared_ptr<SynchronizedData<http::request<http::string_body>>> synchronizedData) override
	{
		(void)req;
		(void)synchronizedData;

		if (synchronizedData && req.method() == http::verb::get)
		{
			http::response<http::string_body> res{ http::status::ok, req.version() };
			res.set(http::field::server, "Beast");
			res.keep_alive(req.keep_alive());

			//TODO do something sensible here, or remove this endpoint completely
			res.body() = "<img alt = \"1×1 red pixel\"src = \"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABAQMAAAAl21bKAAAAA1BMVEX/AAAZ4gk3AAAAAXRSTlPM0jRW/QAAAApJREFUeJxjYgAAAAYAAzY3fKgAAAAASUVORK5CYII=\" />";

			res.prepare_payload();
			return res;

		}

		return http::response<http::string_body>{};
	}
};
