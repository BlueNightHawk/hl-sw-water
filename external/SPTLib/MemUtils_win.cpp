#include "PlatformHeaders.h"
#include <string>
#include <functional>
#include <future>
#include "MemUtils.hpp"
#include <Psapi.h>

namespace MemUtils
{
	bool GetModuleInfo(void* moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		if (!moduleHandle)
			return false;

		MODULEINFO Info;
		GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(moduleHandle), &Info, sizeof(Info));

		if (moduleBase)
			*moduleBase = Info.lpBaseOfDll;

		if (moduleSize)
			*moduleSize = (size_t)Info.SizeOfImage;

		return true;
	}

	bool GetModuleInfo(const std::wstring& moduleName, void** moduleHandle, void** moduleBase, size_t* moduleSize)
	{
		HMODULE Handle = GetModuleHandleW(moduleName.c_str());
		auto ret = GetModuleInfo(Handle, moduleBase, moduleSize);

		if (ret && moduleHandle)
			*moduleHandle = Handle;

		return ret;
	}
}
