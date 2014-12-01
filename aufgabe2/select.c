#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

inr pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *eceptfds, const struct timespec *timeout, const sigset_t *sigmask)
