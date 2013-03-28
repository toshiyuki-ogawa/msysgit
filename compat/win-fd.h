#ifndef WIN_FD_H
#define WIN_FD_H
typedef struct _win_fd_status win_fd_status;

struct _win_fd_status
{
	int size;
	struct {
		int fd;
		int inherited;
	} status[1];
};
win_fd_status *win_fd_apply_inheritance_0(int inheritance, int size, int* fd);_

win_fd_status *win_fd_apply_inheritance_1(int inheritance, ...);

void win_fd_restore_inheritance(win_fd_status *fd_status);

void win_fd_free_status(win_fd_status *fd_status);
#endif
