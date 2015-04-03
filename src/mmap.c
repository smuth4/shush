/*
** Copyright (C) 2003, 2004 Christophe Kalt
**
** This file is part of shush,
** see the LICENSE file for details on your rights.
*/

#include "os.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mmap.h"

static char const rcsid[] =
    "@(#)$Id: mmap.c 1404 2008-03-08 23:25:46Z kalt $";

static struct stat sb;

/*
** mapfile
**	Given a file name, mmap() the corresponding (regular) file
*/
int mapfile(char *name, int fd, void **mm, size_t * len)
{
    *mm = NULL;

    if (fd < 0) {
	fd = open(name, O_RDONLY, 0);
	if (fd < 0) {
	    if (errno == ENOENT)
		return 1;
	    else {
		error("open(%s) failed: %s", name, ERRSTR);
		return -1;
	    }
	}
	fd *= -1;
    }

    if (fstat(abs(fd), &sb) == -1) {
	/* Can this fail at all? */
	error("fstat(%s) failed: %s", name, ERRSTR);
	return -1;
    }
    *len = sb.st_size;

    if (*len == 0) {
	if (fd < 0)
	    close(abs(fd));
	return 0;
    }

    *mm = mmap(NULL, *len, PROT_READ | PROT_WRITE, MAP_PRIVATE
#if defined(MAP_FILE)
	       | MAP_FILE
#endif
#if defined(MAP_COPY)
	       | MAP_COPY
#endif
	       , abs(fd), 0);

    if (*mm == MAP_FAILED) {
	/* Can this fail at all? */
	error("mmap(%s) failed: %s", name, ERRSTR);
	if (fd < 0)
	    close(abs(fd));
	return -1;
    }

    if (fd < 0)
	close(abs(fd));

    return 0;
}

/*
** mapstat
**	returns 'struct stat' of the previously mapped file.
*/
struct stat *mapstat(void)
{
    return &sb;
}

/*
** unmapfile
**	munmap() a previously mapped file
*/
int unmapfile(char *name, void *mm, size_t len)
{
    assert(mm != NULL);
    assert(len > 0);

    if (munmap(mm, len) == -1) {
	error("munmap(%s) failed: %s", name, ERRSTR);
	return -1;
    } else
	return 0;
}
