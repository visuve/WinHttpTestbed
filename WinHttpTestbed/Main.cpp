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
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc < 2 || argc > 3)
	{
		std::cerr << "Usage <url> <path-to-save-output (optional)>" << std::endl;
		return ERROR_BAD_ARGUMENTS;
	}

	{
		const Timing::Timer t;

		const Http::Url url(argv[1]);
		std::wcout << url << std::endl;

		const std::string content = Http::Get(url);

		if (content.empty())
		{
			return ERROR_NO_DATA;
		}

		if (argc != 3)
		{
			std::cout << content << std::endl;
		}
		else
		{
			std::filesystem::path path(argv[2]);

			if (!path.has_parent_path())
			{
				// Override current working directory
				path = std::filesystem::path(argv[0]).parent_path() / path;
			}

			std::ofstream file(path, std::ios::binary | std::ios::trunc);

			if (file && file << content)
			{
				std::cout << "Saved: " << path << std::endl;
			}
		}
	}

	return ERROR_SUCCESS;
}