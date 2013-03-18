#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H
#include "run-command.h"

#ifdef WIN32
#define fd_to_socket(fd) ((SOCKET)_get_osfhandle(fd))
#else
#define fd_to_socket(fd) (fd)
#endif
#ifndef WIN32

extern int is_socket(int fd);

extern void set_socket_to_time_wait(int fd, int fd_is_out);

extern int start_command_socket(struct child_process * cld, int socket);
#endif
