#include "PCH.hpp"
#include "Timing.hpp"

namespace Http
{
	class HttpHandle
	{
	public:
		HttpHandle() = default;
		HttpHandle(HINTERNET handle) :
			m_handle(handle)
		{
		}

		~HttpHandle()
		{
			Close();
		}

		bool IsValid() const
		{
			return m_handle != nullptr;
		}

		HttpHandle& operator = (HINTERNET handle)
		{
			Exchange(handle);
			return *this;
		}

		operator HINTERNET() const
		{
			return m_handle;
		}

	private:
		void Close()
		{
			if (m_handle)
			{
				WinHttpCloseHandle(m_handle);
			}
		}

		void Exchange(HINTERNET handle)
		{
			if (m_handle != handle)
			{
				Close();
				m_handle = handle;
			}
		}

		HINTERNET m_handle = nullptr;
	};

	namespace Regex
	{
		// Derived from: https://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform answer from Mr. Jones
		constexpr wchar_t Scheme[] = L"((http[s]?)://)?";  // match http or https before the ://
		constexpr wchar_t User[] = L"(([^@/:\\s]+)@)?";  // match anything other than @ / : or whitespace before the ending @
		constexpr wchar_t Host[] = L"([^@/:\\s]+)";      // mandatory. match anything other than @ / : or whitespace
		constexpr wchar_t Port[] = L"(:([0-9]{1,5}))?";  // after the : match 1 to 5 digits
		constexpr wchar_t Path[] = L"(/[^:#?\\s]*)?";    // after the / match anything other than : # ? or whitespace
		constexpr wchar_t Query[] = L"(\\?(([^?;&#=]+=[^?;&#=]+)([;|&]([^?;&#=]+=[^?;&#=]+))*))?"; // after the ? match any number of x=y pairs, seperated by & or ;
		constexpr wchar_t Fragment[] = L"(#([^#\\s]*))?";    // after the # match anything other than # or whitespace

		const std::wregex UrlRegex(std::wstring(L"^") + Scheme + User + Host + Port + Path + Query + Fragment + L"$");
	}

	class Url
	{
	public:
		std::wstring Scheme;
		std::wstring User;
		std::wstring Host;
		std::wstring Port;
		std::wstring Path;
		std::wstring Query;
		std::wstring Fragment;

		Url() = default;

		Url(const std::wstring& url)
		{
			std::wcmatch result;

			if (std::regex_match(url.c_str(), result, Regex::UrlRegex))
			{
				Scheme.assign(result[2].first, result[2].second);
				User.assign(result[4].first, result[4].second);
				Host.assign(result[5].first, result[5].second);
				Port.assign(result[7].first, result[7].second);
				Path.assign(result[8].first, result[8].second);
				Query.assign(result[10].first, result[10].second);
				Fragment.assign(result[15].first, result[15].second);
			}
			else
			{
				throw std::invalid_argument("Failed to parse URL");
			}
		}

		Url(const wchar_t* uri) :
			Url(std::wstring(uri))
		{
		}

		template <typename size_t N>
		Url(const wchar_t(&uri)[N]) :
			Url(std::wstring(uri, N))
		{
		}
	};

	std::wostream& operator << (std::wostream& os, const Url& uri)
	{
		if (!uri.Scheme.empty())
		{
			os << uri.Scheme << "://";
		}

		os << uri.Host;

		if (!uri.Path.empty())
		{
			os << uri.Path;
		}

		return os;
	}

	std::string Get(const Url& url)
	{
		HttpHandle session, connection, request;

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

	std::string Post(const Url& url, const std::string& data)
	{
		HttpHandle session, connection, request;

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

void Usage(const std::wstring& exe)
{
	std::wcerr << L"Usage:" << std::endl;
	std::wcerr << exe << L" GET <url> <path-to-save-output>" << std::endl;
	std::wcerr << L"OR" << std::endl;
	std::wcerr << exe << L" POST <url> <path-to-post-contents>" << std::endl;
}

int wmain(int argc, const wchar_t* argv[])
{
	if (argc != 4)
	{
		Usage(argv[0]);
		return ERROR_BAD_ARGUMENTS;
	}

	const Timing::Timer t;

	try
	{
		const Http::Url url(argv[2]);
		std::filesystem::path path(argv[3]);

		if (!path.has_parent_path())
		{
			// Override current working directory
			path = std::filesystem::path(argv[0]).parent_path() / path;
		}

		if (_wcsicmp(argv[1], L"GET") == 0)
		{
			const std::string response = Http::Get(url);

			std::ofstream file(path, std::ios::binary | std::ios::trunc);

			if (file && file << response)
			{
				std::cout << "Saved: " << path << std::endl;
			}
		}
		else if (_wcsicmp(argv[1], L"POST") == 0)
		{
			std::ifstream file(path, std::ios::binary);

			if (!file)
			{
				return ERROR_FILE_NOT_FOUND;
			}

			const std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			
			const std::string response = Http::Post(url, data);

			if (!response.empty())
			{
				std::cout << "Got response: " << response;
			}
		}
		else
		{
			Usage(argv[0]);
			return ERROR_BAD_ARGUMENTS;
		}

	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;

	}

	return ERROR_SUCCESS;
}