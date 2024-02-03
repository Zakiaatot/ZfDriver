#ifndef _UTILS_H_
#define _UTILS_H_

namespace Utils
{
	void AlertError(const wchar_t* msg);
	bool ReleaseResource(unsigned int uResourceId, const wchar_t* szResourceType, const wchar_t* szFileName);
}

#endif // !_UTILS_H_

