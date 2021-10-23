#pragma once

#include "HttpHandle.hpp"
#include "URL.hpp"

namespace Http
{
	class Request
	{
	public:
		Request(const Url& url, std::wstring_view method);
		virtual bool Execute() const = 0;
		std::string Response() const;
	protected:
		Handle _session;
		Handle _connection;
		Handle _request;
	};

	class GetRequest : public Request
	{
	public:
		GetRequest(const Url& url);
		bool Execute() const override;
	};

	class PostRequest : public Request
	{
	public:
		PostRequest(const Url& url, std::string_view payload);
		bool Execute() const override;
	private:
		std::string _payload;
	};
};
