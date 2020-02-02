#pragma once

namespace Http
{
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
		Url(const std::wstring& url);
		Url(const wchar_t* uri);
		template <typename size_t N>
		Url(const wchar_t(&uri)[N]) :
			Url(std::wstring(uri, N))
		{
		}

		operator std::wstring() const;
	};
}