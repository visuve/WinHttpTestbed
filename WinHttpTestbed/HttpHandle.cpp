#include "PCH.hpp"
#include "HttpHandle.hpp"

namespace Http
{
	Handle::Handle(HINTERNET handle) :
		_handle(handle)
	{
	}

	Handle::~Handle()
	{
		Close();
	}

	bool Handle::IsValid() const
	{
		return _handle != nullptr;
	}

	Handle& Handle::operator = (HINTERNET handle)
	{
		if (_handle != handle)
		{
			Close();
			_handle = handle;
		}

		return *this;
	}

	Handle::operator HINTERNET() const
	{
		return _handle;
	}

	void Handle::Close()
	{
		if (_handle)
		{
			WinHttpCloseHandle(_handle);
		}
	}
}