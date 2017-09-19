/*************************************************************************//**
 *****************************************************************************
 * @file   stdio.h
 * @brief  
 * @author Forrest Y. Yu
 * @date   2008
 *****************************************************************************
 *****************************************************************************/

/* the assert macro */
#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

/* EXTERN */
#define	EXTERN	extern	/* EXTERN is defined as extern except in global.c */

/* string */
#define	STR_DEFAULT_LEN	1024

#define	O_CREAT		1
#define	O_RDWR		2

#define T_FOLDER	1
#define T_FILE		0

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#define	MAX_PATH	128

/*--------*/
/* 库函数 */
/*--------*/

#ifdef ENABLE_DISK_LOG
#define SYSLOG syslog
#endif

/* lib/open.c */
PUBLIC	int	open		(const char *pathname, int flags);

/* lib/close.c */
PUBLIC	int	close		(int fd);

/* lib/changeType.c */
PUBLIC	int	changeType	(int fd, int type);

/* lib/showPro.c */
PUBLIC  int	showPro		(int fd);

/* lib/fs_check.c */
PUBLIC  int	fs_check	(int fd);

/* lib/move.c */
PUBLIC	int	move		(int fd, int target_fd);

/* lib/getPath.c */
PUBLIC	int	getPath		(int fd, char *pathname);

/* lib/search.c */
PUBLIC	int	search		(const char *filename, char *pathname);

/* lib/getPath.c */
PUBLIC	int	rename 		(int fd, const char *filename);

/* lib/read.c */
PUBLIC int	read		(int fd, void *buf, int count);

/* lib/write.c */
PUBLIC int	write		(int fd, const void *buf, int count);

/* lib/unlink.c */
PUBLIC	int	unlink		(const char *pathname);

/* lib/getpid.c */
PUBLIC int	getpid		();

/* lib/syslog.c */
PUBLIC	int	syslog		(const char *fmt, ...);

