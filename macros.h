#include <fcntl.h>

#define SET_NONBLOCKING(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)