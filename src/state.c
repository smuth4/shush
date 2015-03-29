/*
** Copyright (C) 2005, 2006 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <fcntl.h>
#if defined(HAVE_PTHREAD_H)
# include <pthread.h>
#endif

#include "state.h"
#include "cf.h"
#include "debug.h"
#include "gethostname.h"
#include "units.h"
#include "version.h"

static char const rcsid[] = "@(#)$Id: state.c 1404 2008-03-08 23:25:46Z kalt $";

static char statepath[PATH_MAX];
static int  fd = -1;

struct
{
    time_t last;	/* last time state was saved */
    time_t start;	/* shush start time */
    time_t delay; 	/* random delay */
    time_t run;		/* command run start time */
    time_t timeout;	/* first timeout action time */
    time_t end;		/* command end time */
    int locked;		/* lock: (1) used (2) ignored (3) aborted */
    int killed;		/* whether command was sent signals */
    /* need exit status info? */
    char *msg;		/* human readable message */
} state;

#if defined(HAVE_PTHREAD)
static void *
state_save(void *null)
{
    char str[1024], lock;
    int len;

    fd = open(statepath, O_RDWR|O_CREAT|O_EXCL, 0644);
    if (fd == -1)
      {
        if (errno != ENOENT || cf_getstr(CF_STATEDIR) != NULL)
            error("open(%s) failed: %s", statepath, ERRSTR);
        pthread_exit(NULL);
      }

    while (1)
      {
        if (time(NULL) - state.last < 30)
          {
            sleep(1);
            continue;
          }

        if (lseek(fd, SEEK_SET, 0) != 0)
          {
            error("lseek(%s, SEEK_SET, 0) failed, state lost: %s",
                  statepath, ERRSTR);
            close(fd);
            fd = -1;
            pthread_exit(NULL);
          }

        if (state.end == 0 && state.msg == NULL)
            /* Better update twice than miss an update */
            state.last = time(NULL);
        else
            /* final update */
            state.last = 0;

        if (state.locked == STATE_LCKOK)
            lock = 'L';
        else if (state.locked == STATE_LCKIGN)
            lock = 'I';
        else if (state.locked == STATE_LCKFAIL)
            lock = 'F';
        else
            lock = '-';

        len = snprintf(str, 1024, "%lu %lu %lu %lu %lu %lu %c %u %.80s\n",
                       state.last, state.start, state.delay, state.run,
                       state.timeout, state.end, lock, state.killed,state.msg);

        if (write(fd, str, len) != len)
          {
            error("write(%s) failed, state lost: %s", statepath, ERRSTR);
            close(fd);
            fd = -1;
            pthread_exit(NULL);
          }

        if (ftruncate(fd, len) != 0)
          {
            error("ftruncate(%s, %d) failed, state lost: %s",
                  statepath, len, ERRSTR);
            close(fd);
            fd = -1;
            pthread_exit(NULL);
          }

        if (state.last == 0)
          {
            close(fd);
            fd = -1;
            pthread_exit(NULL);
          }
      }
}
#endif

void
state_init(char *cfdir, char *job)
{
#if defined(HAVE_PTHREAD_H)
    pthread_t save;
#endif
    char *sdir;
    int status;

    state.start = time(NULL);
    state.last = 0;
    state.delay = 0;
    state.run = 0;
    state.timeout = 0;
    state.end = 0;
    state.locked = 0;
    state.killed = 0;
    state.msg = "";

    sdir = cf_getstr(CF_STATEDIR);
    if (sdir == NULL)
        snprintf(statepath, PATH_MAX, "%s/.state/shtate-%lu-%s-%s-%u",
                 cfdir, state.start, job, get_hostname(0), getpid());
    else if (sdir[0] == '\0')
        return;
    else
        snprintf(statepath, PATH_MAX, "%s/shtate-%lu-%s-%s-%u",
                 sdir, state.start, job, get_hostname(0), getpid());

#if defined(HAVE_PTHREAD)
    status = pthread_create(&save, NULL, state_save, NULL);
    if (status != 0)
        error("pthread_create(state_save) failed: %s", strerror(status));
#endif
}

void
state_delay(time_t delay)
{
    assert( state.delay == 0 );
    state.delay = delay;
    state.last = 1;
}

void
state_run()
{
    assert( state.run == 0 );
    state.run = time(NULL);
    state.last = 1;
}

void
state_timeout()
{
    if (state.timeout == 0)
      {
        state.timeout = time(NULL);
        state.last = 1;
      }
}

void
state_locked(int status)
{
    state.locked = status;
    state.last = 1;
}

void
state_killed()
{
    state.killed += 1;
    state.last = 1;
}

void
state_close(char *msg)
{
    int max;

    if (msg == NULL)
        state.end = time(NULL);
    else
        state.msg = msg;
    state.last = 1;

    max = 15;
    while (max-- > 0 && fd >= 0)
        sleep(1);
}
