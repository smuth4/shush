/*
** Copyright (C) 2002, 2003, 2004 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#if defined(HAVE_PATHS_H)
#include <paths.h>
#else
#define _PATH_DEVNULL "/dev/null"
#endif
#include <fcntl.h>
#include <libgen.h>
#include <sys/time.h>		/* FreeBSD wants this for the next one.. */
#include <sys/resource.h>
#include <sys/types.h>
#include <signal.h>
#include <stdarg.h>

#include "exec.h"

static char const rcsid[] =
    "@(#)$Id: exec.c 1404 2008-03-08 23:25:46Z kalt $";

static void fatal(int, char *, ...)
#if ( __GNUC__ == 2 && __GNUC_MINOR__ >= 5 ) || __GNUC__ >= 3
    __attribute__ ((__noreturn__))
#endif
    ;

/*
** fatal
**	Output the fatal error message and commit SIGTERM suicide to try and
**	make sure this doesn't go unnoticed.
*/
static void fatal(int fd, char *format, ...)
{
    char msg[1024];
    va_list va;

    va_start(va, format);
    vsprintf(msg, format, va);
    va_end(va);
    write(fd, msg, strlen(msg) + 1);
    kill(SIGKILL, getpid());
    sleep(5);			/* Just in case.. */
    abort();
}

/*
** exec
**	fork()/execl() wrapper.
*/
pid_t exec(int fd0, int fd1, int fd2, int newgrp, char *cmd[])
{
    struct rlimit fdlimit;
    pid_t child;

    assert(fd0 == -1 || fd0 > 2);
    assert(fd1 >= 1);
    assert(fd2 >= 2);
    assert(cmd != NULL);

    /*
     ** Get the maximum number of open files for this process so we can
     ** cleanup things properly in the child.  The only reason this is
     ** done before forking is to be able to whine if the call fails.
     */
    if (getrlimit(RLIMIT_NOFILE, &fdlimit) == -1) {
	error("getrlimit(RLIMIT_NOFILE): %s", ERRSTR);
	fdlimit.rlim_cur = 1024;
    }

    /* fork() */
    child = fork();

    if (child == -1) {
	/* Nope.. */
	error("fork(): %s", ERRSTR);
	return -1;
    }

    /* Let each process go its own way */
    if (child > 0) {
	if (fd0 >= 0)
	    close(fd0);
	return child;
    } else {
	int fd;
	struct sigaction sa;
	char *command;

	/* Close all open filedescriptors */
	fd = 0;
	while (fd <= fdlimit.rlim_cur) {
	    if (fd != fd0 && fd != fd1 && fd != fd2)
		close(fd);
	    fd += 1;
	}

	if (fd0 < 0) {
	    fd0 = open(_PATH_DEVNULL, O_RDONLY, 0);
	    if (fd0 == -1)
		fatal(fd2, "%s: open(%s): %s\n",
		      myname, _PATH_DEVNULL, ERRSTR);
	    if (fd0 != 0)
		fatal(fd2,
		      "%s: open(%s) returned %d (should have been 0!)\n",
		      myname, _PATH_DEVNULL, fd);
	} else if (dup(fd0) != 0)
	    fatal(fd2, "%s: dup(in) failed: %s\n", myname, ERRSTR);

	if (fd1 != 1 && dup(fd1) != 1)
	    fatal(fd2, "%s: dup(out) failed: %s\n", myname, ERRSTR);
	if (fd2 != 2 && dup(fd2) != 2)
	    fatal(fd2, "%s: dup(out) failed: %s\n", myname, ERRSTR);
	if (fd1 > 2 && close(fd1) == -1)
	    fatal(2, "%s: close(out) failed: %s\n", myname, ERRSTR);
	if (fd2 > 2 && fd2 != fd1 && close(fd2) == -1)
	    fatal(2, "%s: close(err) failed: %s\n", myname, ERRSTR);

	/* Reset SIGPIPE handler */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	if (sigaction(SIGPIPE, &sa, NULL) != 0)
	    fatal(2, "sigaction(SIGPIPE, SIG_DFL, NULL) failed: %s",
		  myname, ERRSTR);

	/* If requested, start a new process group */
	if (newgrp != 0 && setpgid(getpid(), getpid()) != 0)
	    fatal(2, "%s: setpgid() failed: %s\n", myname, ERRSTR);

	/* Finally, execl() */
	command = cmd[0];
	cmd[0] = basename(command);
	execv(command, cmd);

	/* Uhoh */
	fatal(2, "%s: execv(%s): %s\n", myname, command, ERRSTR);
    }
    /* NOTREACHED */
}
