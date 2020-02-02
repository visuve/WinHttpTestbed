#include "PCH.hpp"
#include "URL.hpp"

namespace Http
{
	// Derived from: https://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform answer from Fabiano Tarlao
	const std::wregex UrlRegex(L"(http|https):\\/\\/(?:([^@ ]*)@)?([^:?# ]+)(?::(\\d+))?([^?# ]*)(?:\\?([^# ]*))?(?:#([^ ]*))?");

	Url::Url(const std::wstring& url)
	{
		std::wcmatch result;

		if (std::regex_match(url.c_str(), result, UrlRegex))
		{
			Scheme.assign(result[1].first, result[1].second);
			User.assign(result[2].first, result[2].second);
			Host.assign(result[3].first, result[3].second);
			Port.assign(result[4].first, result[4].second);
			Path.assign(result[5].first, result[5].second);
			Query.assign(result[6].first, result[6].second);
			Fragment.assign(result[7].first, result[7].second);
		}
		else
		{
			throw std::invalid_argument("Failed to parse URL");
		}

		const std::wstring parsed(*this);

		if (url != parsed)
		{
			throw std::invalid_argument("Failed to parse URL");
		}
	}

	Url::Url(const wchar_t* uri) :
		Url(std::wstring(uri))
	{
	}

	Url::operator std::wstring() const
	{
		std::wstring url;

		if (!Scheme.empty())
		{
			url.append(Scheme);
			url.append(L"://");
		}

		if (!User.empty())
		{
			url.append(User);
			url.push_back(L'@');
		}

		url.append(Host);

		if (!Port.empty())
		{
			url.push_back(L':');
			url.append(Port);
		}

		if (!Path.empty())
		{
			url.append(Path);
		}

		if (!Query.empty())
		{
			url.push_back(L'?');
			url.append(Query);
		}

		if (!Fragment.empty())
		{
			url.push_back(L'#');
			url.append(Fragment);
		}

		return url;
	}
}