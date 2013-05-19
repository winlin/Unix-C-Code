UNIX/LINUX下面，除了网卡不被看成文件以外，其他的都看成是文件。
所以，对于一个目录来说它也会具有struct stat信息。
====================================================
access - check real user's permissions for a file
#include <unistd.h>
int access(const char *pathname, int mode);

The mode specifies the accessibility check(s) to be performed, and is either the value F_OK, or a mask  consisting
of  the bitwise OR of one or more of R_OK, W_OK, and X_OK.  F_OK tests for the existence of the file.  R_OK, W_OK,
and X_OK test whether the file exists and grants read, write, and execute permissions, respectively.

====================================================
打开一个目录：
#include <sys/types.h>
#include <dirent.h>

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
RETURN VALUE
The opendir() and fdopendir() functions return a pointer to the directory stream.  On error, NULL is returned,
and errno is set appropriately.
在文件/usr/include/dirent.h里面可以找到：
	typedef struct __dirstream DIR; 知道DIR也是一个结构体。
====================================================
读一个目录中的文件：
#include <dirent.h>
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
On Linux, the dirent structure is defined as follows:
包含的结构体：	
	struct dirent {
		ino_t          d_ino;       /* inode number */
		off_t          d_off;       /* offset to the next dirent */
		unsigned short d_reclen;    /* length of this record */
		unsigned char  d_type;      /* type of file; not supported
                                              by all file system types */
		char           d_name[256]; /* filename */
	};
The data returned by readdir() may be overwritten by subsequent calls to readdir() for the same directory stream.
=====================================================
获得一个文件的状态信息
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);
其中，stat()和lstat()除了对链接文件处理不同外，其余的都相同。
All of these system calls return a stat structure, which contains the following fields:
	struct stat {
		dev_t     st_dev;     /* ID of device containing file */
		ino_t     st_ino;     /* inode number */
		mode_t    st_mode;    /* protection */
		nlink_t   st_nlink;   /* number of hard links */
		uid_t     st_uid;     /* user ID of owner */
		gid_t     st_gid;     /* group ID of owner */
		dev_t     st_rdev;    /* device ID (if special file) */
		off_t     st_size;    /* total size, in bytes */
		blksize_t st_blksize; /* blocksize for file system I/O */
		blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
		time_t    st_atime;   /* time of last access */
		time_t    st_mtime;   /* time of last modification */
		time_t    st_ctime;   /* time of last status change */
	};
