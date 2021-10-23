#pragma once

#include "URL.hpp"

namespace Http
{
	std::string Get(const Url& url);
	std::string Post(const Url& url, const std::string& data);
};
