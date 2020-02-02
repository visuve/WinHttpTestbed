#pragma once

#include <winhttp.h>

namespace Http
{
	class Handle
	{
	public:
		Handle() = default;
		Handle(HINTERNET handle);
		~Handle();

		bool IsValid() const;
		Handle& operator = (HINTERNET handle);
		operator HINTERNET() const;

	private:
		void Close();

		HINTERNET m_handle = nullptr;
	};
}