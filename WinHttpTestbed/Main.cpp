#include "PCH.hpp"
#include "Timing.hpp"
#include "URL.hpp"
#include "HttpRequest.hpp"

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
			Http::GetRequest request(url);

			if (!request.Execute())
			{
				std::cerr << "Failed to execute get request" << std::endl;
				return ERROR_READ_FAULT;
			}
				
			std::string response = request.Response();

			if (response.empty())
			{
				std::cerr << "Got no reponse" << std::endl;
				return WSANO_DATA;
			}

			std::ofstream file(path, std::ios::binary | std::ios::trunc);

			if (file && file << response)
			{
				std::cout << "Saved response in: " << path << std::endl;
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

			Http::PostRequest request(url, data);

			if (!request.Execute())
			{
				std::cerr << "Failed to execute post request" << std::endl;
				return ERROR_WRITE_FAULT;
			}

			std::string response = request.Response();

			if (response.empty())
			{
				std::cerr << "Got no response" << std::endl;
				return WSANO_DATA;
			}
			else
			{
				std::cout << "Got response:" << response << std::endl;
			}
		}
		else
		{
			Usage(argv[0]);
			return ERROR_BAD_ARGUMENTS;
		}
	}
	catch (const std::exception & e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}

	return ERROR_SUCCESS;
}