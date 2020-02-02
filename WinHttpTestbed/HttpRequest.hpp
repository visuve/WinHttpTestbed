#pragma once

#include "URL.hpp"
#include <string>

namespace Http::Request
{
	std::string Get(const Url& url);
	std::string Post(const Url& url, const std::string& data);
};