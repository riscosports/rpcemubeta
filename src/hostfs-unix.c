#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <utime.h>
#include <sys/stat.h>

#include "hostfs_internal.h"

/**
 * Convert a time_t to the equivalent RISC OS time.
 *
 * @param      t    Time as time_t
 * @param[out] load Pointer to uint32_t, set to high 8 bits of RISC OS time
 * @param[out] exec Pointer to uint32_t, set to low 32 bits of RISC OS time
 *
 * Code adapted from fs/adfs/inode.c from Linux licensed under GPL2.
 * Copyright (C) 1997-1999 Russell King
 */
static void
hostfs_time_t_to_risc_os_time(time_t t, uint32_t *load, uint32_t *exec)
{
	uint32_t low, high;

	low  = (uint32_t) ((t & 255) * 100);
	high = (uint32_t) ((t / 256) * 100 + (low >> 8) + 0x336e996a);

	*load = (high >> 24);
	*exec = (low & 0xff) | (high << 8);
}

/**
 * Convert ADFS time-stamped Load-Exec addresses to the equivalent time_t.
 *
 * @param load RISC OS load address (assumed to be time-stamped)
 * @param exec RISC OS exec address (assumed to be time-stamped)
 * @return Time converted to time_t format
 *
 * Code adapted from fs/adfs/inode.c from Linux licensed under GPL2.
 * Copyright (C) 1997-1999 Russell King
 */
static time_t
hostfs_adfs2host_time(uint32_t load, uint32_t exec)
{
	uint32_t high = load << 24;
	uint32_t low  = exec;

	high |= low >> 8;
	low &= 0xff;

	if (high < 0x3363996a) {
		/* Too early */
		return 0;
	} else if (high >= 0x656e9969) {
		/* Too late */
		return 0x7ffffffd;
	}

	high -= 0x336e996a;
	return (((high % 100) << 8) + low) / 100 + (high / 100 << 8);
}

/**
 * Extract the modification time from a 'struct stat'.
 * Convert this time to an equivalent RISC OS time, and store in the
 * Load-Exec addresses of a 'risc_os_object_info'.
 *
 * @param      s           Pointer to a 'struct stat'
 * @param[out] object_info Pointer to object info in which Load-Exec are filled in
 */
static void
hostfs_struct_stat_to_risc_os_time(const struct stat *s, risc_os_object_info *object_info)
{
	uint32_t load, exec;

	hostfs_time_t_to_risc_os_time(s->st_mtime, &load, &exec);
	object_info->load = load;
	object_info->exec = exec;
}

/**
 * Read information about an object.
 *
 * @param host_pathname Full Host path to object
 * @param object_info   Return object info (filled-in)
 */
void
hostfs_read_object_info_platform(const char *host_pathname,
                                 risc_os_object_info *object_info)
{
	struct stat info;

	assert(host_pathname != NULL);
	assert(object_info != NULL);

	if (stat(host_pathname, &info)) {
		/* Error reading info about the object */

		switch (errno) {
		case ENOENT: /* Object not found */
		case ENOTDIR: /* A path component is not a directory */
			object_info->type = OBJECT_TYPE_NOT_FOUND;
			break;

		default:
			/* Other error */
			fprintf(stderr,
			        "hostfs_read_object_info_platform() could not stat() \'%s\': %s %d\n",
			        host_pathname, strerror(errno), errno);
			object_info->type = OBJECT_TYPE_NOT_FOUND;
			break;
		}

		return;
	}

	/* We were able to read about the object */
	if (S_ISREG(info.st_mode)) {
		object_info->type = OBJECT_TYPE_FILE;
	} else if (S_ISDIR(info.st_mode)) {
		object_info->type = OBJECT_TYPE_DIRECTORY;
	} else {
		/* Treat types other than file or directory as not found */
		object_info->type = OBJECT_TYPE_NOT_FOUND;
		return;
	}

	/* If the file has filetype and timestamp, additional values will need to be filled in later */
	hostfs_struct_stat_to_risc_os_time(&info, object_info);

	object_info->length = info.st_size;
}

/**
 * Apply the timestamp to the supplied host object
 *
 * @param host_path Full path to object (file or dir) in host format
 * @param load      RISC OS load address (must contain time-stamp)
 * @param exec      RISC OS exec address (must contain time-stamp)
 */
void
hostfs_object_set_timestamp_platform(const char *host_path, uint32_t load, uint32_t exec)
{
	struct utimbuf t;

	t.actime = t.modtime = hostfs_adfs2host_time(load, exec);
	utime(host_path, &t);
	/* TODO handle error in utime() */
}
