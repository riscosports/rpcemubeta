#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "hostfs_internal.h"
#include "rpcemu.h"

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

/* Extended attributes store RISC OS filetype, or load/exec.
 * Uses RISC OS on Linux's format: three 32‑bit words (load, exec, attributes).
 * NOTE: Not portable as in host endian, but matches ROonL. */
#define XATTR_NAME "user.RISC_OS.LoadExec"

typedef struct {
	uint32_t load;
	uint32_t exec;
	uint32_t attribs;
} XattrInfo;

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
	uint32_t low, high;

	assert(host_pathname != NULL);
	assert(object_info != NULL);
    
  // Ignore DS_Store files.
  if (strcasestr(host_pathname, ".DS_Store") != NULL)
  {
      object_info->type = OBJECT_TYPE_NOT_FOUND;
      return;
  }
    
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

	low  = (uint32_t) ((info.st_mtime & 255) * 100);
	high = (uint32_t) ((info.st_mtime / 256) * 100 + (low >> 8) + 0x336e996a);

	/* If the file has filetype and timestamp, additional values will need to be filled in later */
	object_info->load = (high >> 24);
	object_info->exec = (low & 0xff) | (high << 8);

	object_info->length = info.st_size;
}

int
hostfs_supports_xattr_platform()
{
	return config.xattrsenabled;
}

int
hostfs_read_object_xattr_platform(const char *host_pathname, risc_os_object_info *object_info)
{
	struct stat info;
	XattrInfo attrInfo;
	ssize_t len;

	assert(host_pathname != NULL);
	assert(object_info != NULL);

	if (stat(host_pathname, &info) || !S_ISREG(info.st_mode)) {
		return 0;
	}

	/* Read the RISC OS load/exec/attributes from the extended attribute. */
	len = getxattr(host_pathname, XATTR_NAME, &attrInfo, sizeof(attrInfo), 0,
				   XATTR_NOFOLLOW);
	if (len < 8)
		return 0; /* need at least load & exec */

	if (len < 12)
		attrInfo.attribs = DEFAULT_ATTRIBUTES; /* pretend it got this. */

	/* hostfs_read_object_info_platform(), above, always sets the time in
	 * object_info's load&exec. If xattrs are for a filetype/time, combine
	 * them, otherwise overwrite with fill load/exec. */
	if ((attrInfo.load & 0xfff00000u) == 0xfff00000u) {
		object_info->load |= attrInfo.load & 0xffffff00u;
	} else {
		object_info->load = attrInfo.load;
		object_info->exec = attrInfo.exec;
	}

	object_info->attribs = attrInfo.attribs;
	return 1;
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

void
hostfs_object_set_xattr_platform(const char *host_path,
                                 uint32_t load,
                                 uint32_t exec,
                                 uint32_t attribs)
{
	XattrInfo attrInfo;
	attrInfo.load = load;
	attrInfo.exec = exec;
	attrInfo.attribs = attribs;

	/* For filetype/timestamp RISC OS files, strip out the time. */
	if ((load & 0xfff00000u) == 0xfff00000u) {
		attrInfo.load = load & 0xffffff00u;
		attrInfo.exec = 0;
	}

	if (setxattr(host_path,
				 XATTR_NAME,
				 &attrInfo,
				 sizeof(attrInfo),
				 0,
				 XATTR_NOFOLLOW) != 0)
	{
		fprintf(stderr,
				"hostfs_object_set_loadexec_platform() could not set xattr on '%s': %s %d\n",
				host_path, strerror(errno), errno);
	}
}
