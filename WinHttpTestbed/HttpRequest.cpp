#include "PCH.hpp"
#include "HttpRequest.hpp"
#include "HttpHandle.hpp"

namespace Http
{
	std::string Request::Get(const Url& url)
	{
		Handle session, connection, request;

		session = WinHttpOpen(
			L"WinHTTP Testbed/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);

		if (!session.IsValid())
		{
			std::cerr << "WinHttpOpen failed with: " << GetLastError() << std::endl;
			return {};
		}

		connection = WinHttpConnect(
			session,
			url.Host.c_str(),
			INTERNET_DEFAULT_HTTPS_PORT,
			0);

		if (!connection.IsValid())
		{
			std::cerr << "WinHttpConnect failed with: " << GetLastError() << std::endl;
			return {};
		}

		request = WinHttpOpenRequest(
			connection,
			L"GET",
			url.Path.c_str(),
			nullptr,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

		if (!request.IsValid())
		{
			std::cerr << "WinHttpOpenRequest failed with: " << GetLastError() << std::endl;
			return {};
		}

		if (!WinHttpSendRequest(
			request,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0))
		{
			std::cerr << "WinHttpSendRequest failed with: " << GetLastError() << std::endl;
			return {};
		}

		if (!WinHttpReceiveResponse(request, nullptr))
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

			if (!WinHttpQueryDataAvailable(request, &bufferSize))
			{
				std::cerr << "WinHttpQueryDataAvailable failed with: " << GetLastError() << std::endl;
				break;
			}

			if (!bufferSize)
			{
				break;
			}

			std::string buffer(bufferSize, '\0');

			if (!WinHttpReadData(request, &buffer.front(), bufferSize, &bytesRead))
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
		} while (bufferSize > 0);

		return response;
	}

	std::string Http::Request::Post(const Url& url, const std::string& data)
	{
		Http::Handle session, connection, request;

		session = WinHttpOpen(
			L"WinHTTP Testbed/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);

		if (!session.IsValid())
		{
			std::cerr << "WinHttpOpen failed with: " << GetLastError() << std::endl;
			return {};
		}

		connection = WinHttpConnect(
			session,
			url.Host.c_str(),
			INTERNET_DEFAULT_HTTPS_PORT,
			0);

		if (!connection.IsValid())
		{
			std::cerr << "WinHttpConnect failed with: " << GetLastError() << std::endl;
			return {};
		}

		request = WinHttpOpenRequest(
			connection,
			L"POST",
			url.Path.c_str(),
			nullptr,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

		if (!request.IsValid())
		{
			std::cerr << "WinHttpOpenRequest failed with: " << GetLastError() << std::endl;
			return {};
		}

		constexpr wchar_t headers[] = L"Content-Type: text/html; charset=UTF-8\r\n";

		if (!WinHttpSendRequest(
			request,
			headers,
			static_cast<DWORD>(-1),
			const_cast<char*>(data.c_str()),
			static_cast<DWORD>(data.size()),
			static_cast<DWORD>(data.size()),
			0))
		{
			std::cerr << "WinHttpSendRequest failed with: " << GetLastError() << std::endl;
			return {};
		}

		if (!WinHttpReceiveResponse(request, nullptr))
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

			if (!WinHttpQueryDataAvailable(request, &bufferSize))
			{
				std::cerr << "WinHttpQueryDataAvailable failed with: " << GetLastError() << std::endl;
				break;
			}

			if (!bufferSize)
			{
				break;
			}

			std::string buffer(bufferSize, '\0');

			if (!WinHttpReadData(request, &buffer.front(), bufferSize, &bytesRead))
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
		} while (bufferSize > 0);

		return response;
	}
}