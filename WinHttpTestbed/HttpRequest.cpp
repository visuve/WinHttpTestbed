#include "PCH.hpp"
#include "HttpRequest.hpp"
#include "HttpHandle.hpp"

namespace Http
{
	Request::Request(const Url& url, std::wstring_view method)
	{
		_session = WinHttpOpen(
			L"WinHTTP Testbed/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);

		if (!_session.IsValid())
		{
			std::cerr << "WinHttpOpen failed with: " << GetLastError() << std::endl;
			return;
		}

		_connection = WinHttpConnect(
			_session,
			url.Host.data(),
			INTERNET_DEFAULT_HTTPS_PORT,
			0);

		if (!_connection.IsValid())
		{
			std::cerr << "WinHttpConnect failed with: " << GetLastError() << std::endl;
			return;
		}

		_request = WinHttpOpenRequest(
			_connection,
			method.data(),
			url.Path.data(),
			nullptr,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

		if (!_request.IsValid())
		{
			std::cerr << "WinHttpOpenRequest failed with: " << GetLastError() << std::endl;
			return;
		}
	}

	std::string Request::Response() const
	{
		if (!WinHttpReceiveResponse(_request, nullptr))
		{
			std::cerr << "WinHttpReceiveResponse failed with: " << GetLastError() << std::endl;
			return {};
		}

		DWORD bufferSize = 0;
		DWORD bytesRead = 0;
		DWORD bytesTotal = 0;
		std::string response;

		do
		{
			bufferSize = 0;

			if (!WinHttpQueryDataAvailable(_request, &bufferSize))
			{
				std::cerr << "WinHttpQueryDataAvailable failed with: " << GetLastError() << std::endl;
				break;
			}

			if (!bufferSize)
			{
				break;
			}

			std::string buffer(bufferSize, '\0');

			if (!WinHttpReadData(_request, &buffer.front(), bufferSize, &bytesRead))
			{
				std::cerr << "WinHttpReadData failed with: " << GetLastError() << std::endl;
				break;
			}

			if (bufferSize != bytesRead)
			{
				buffer.resize(bytesRead);
			}

			bytesTotal += bytesRead;
			response.append(buffer); // Suboptimal to have two buffers, don't care for now...
		}
		while (bufferSize > 0);

		return response;
	}

	GetRequest::GetRequest(const Url& url) :
		Request(url, L"GET")
	{
	}

	bool GetRequest::Execute() const
	{
		if (!_request.IsValid())
		{
			return false;
		}

		if (!WinHttpSendRequest(
			_request,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0))
		{
			std::cerr << "WinHttpSendRequest failed with: " << GetLastError() << std::endl;
			return false;
		}

		return true;
	}

	PostRequest::PostRequest(const Url& url, std::string_view payload) :
		Request(url, L"POST"),
		_payload(payload)
	{
	}

	bool PostRequest::Execute() const
	{
		if (!_request.IsValid())
		{
			return false;
		}

		wchar_t headers[] = L"Content-Type: text/html; charset=UTF-8";
		DWORD headerLength = static_cast<DWORD>(std::size(headers));
		DWORD payloadSize = static_cast<DWORD>(_payload.size());

		if (!WinHttpSendRequest(
			_request,
			headers,
			headerLength,
			const_cast<char*>(_payload.data()),
			payloadSize,
			payloadSize,
			0))
		{
			std::cerr << "WinHttpSendRequest failed with: " << GetLastError() << std::endl;
			return false;
		}

		return true;
	}
}