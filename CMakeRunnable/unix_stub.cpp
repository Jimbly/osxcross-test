#include "unix_stub.h"
#if defined(__unix) || defined(__APPLE__)

#include <assert.h>
#include <cerrno>

errno_t fopen_s(FILE **f, const char *name, const char *mode) {
	errno_t ret = 0;
	assert(f);
	*f = fopen(name, mode);
	/* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
	if (!*f)
		ret = errno;
	return ret;
}

#endif