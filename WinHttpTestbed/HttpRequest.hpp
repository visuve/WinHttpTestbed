#pragma once

#include "URL.hpp"

namespace Http
{
	class Request
	{
	public:
		Request(const Url& url, std::wstring_view method);
		virtual ~Request();
		virtual bool Execute() const = 0;
		std::string Response() const;
	protected:
		HINTERNET _session = nullptr;
		HINTERNET _connection = nullptr;
		HINTERNET _request = nullptr;
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
