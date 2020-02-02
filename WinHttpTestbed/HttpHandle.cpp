#include "PCH.hpp"
#include "HttpHandle.hpp"

namespace Http
{
	Handle::Handle(HINTERNET handle) :
		m_handle(handle)
	{
	}

	Handle::~Handle()
	{
		Close();
	}

	bool Handle::IsValid() const
	{
		return m_handle != nullptr;
	}

	Handle& Handle::operator = (HINTERNET handle)
	{
		if (m_handle != handle)
		{
			Close();
			m_handle = handle;
		}

		return *this;
	}

	Handle::operator HINTERNET() const
	{
		return m_handle;
	}

	void Handle::Close()
	{
		if (m_handle)
		{
			WinHttpCloseHandle(m_handle);
		}
	}
}