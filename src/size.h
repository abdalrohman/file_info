#ifndef SIZE_H_
#define SIZE_H_

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h> /* for close */

#define UNIT_B "B"

#define UNIT_KB "KB"
#define UNIT_MB "MB"
#define UNIT_GB "GB"
#define UNIT_TB "TB"
#define UNIT_PB "PB"
#define UNIT_EB "EB"

#define UNIT_KiB "KiB"
#define UNIT_MiB "MiB"
#define UNIT_GiB "GiB"
#define UNIT_TiB "TiB"
#define UNIT_PiB "PiB"
#define UNIT_EiB "EiB"

const int64_t SIZE_B = 1000; // 1000

const int64_t SIZE_KB = 1e3;  // 1e3f
const int64_t SIZE_MB = 1e6;  // 1e6f
const int64_t SIZE_GB = 1e9;  // 1e9f
const int64_t SIZE_TB = 1e12; // 1e12f
const int64_t SIZE_PB = 1e15; // 1e15f
const int64_t SIZE_EB = 1e18; // 1e18f

const int64_t SIZE_KiB = 1024;                //1024
const int64_t SIZE_MiB = 1048576;             // 1048576
const int64_t SIZE_GiB = 1073741824;          // 1073741824
const int64_t SIZE_TiB = 1099511627776;       // 1099511627776
const int64_t SIZE_PiB = 1125899906842624;    // 1125899906842624
const int64_t SIZE_EiB = 1152921504606846976; // 1152921504606846976

// https://en.wikipedia.org/wiki/Dereference_operator
#define ArrayLength(x) (sizeof(x) / sizeof(*(x)))

// static const char *units_1024[] = {UNIT_EiB, UNIT_PiB, UNIT_TiB, UNIT_GiB, UNIT_MiB, UNIT_KiB, UNIT_B};
// static const int64_t size_1024 = 1024L * 1024L * 1024L * 1024L * 1024L * 1024L;

static const char *units_1000[] = {UNIT_EB, UNIT_PB, UNIT_TB, UNIT_GB, UNIT_MB, UNIT_KB, UNIT_B};
static const int64_t size_1000 = 1000L * 1000L * 1000L * 1000L * 1000L * 1000L;

int64_t stat_size(struct stat *s)
{
    return s->st_blocks * 512;
}

int64_t dir_size(int dfd)
{
    struct dirent *dirp;
    struct stat buff;
    DIR *dir = fdopendir(dfd);
    if (dir == NULL)
    {
        close(dfd);
        exit(EXIT_FAILURE);
    }
    int64_t size = 0;
    int subfd;

    while ((dirp = readdir(dir)) != NULL)
    {
        if (dirp->d_type == DT_DIR)
        {
            /* always skip "." and ".." */
            if (dirp->d_name[0] == '.' && (dirp->d_name[1] == 0 || (dirp->d_name[1] == '.' && dirp->d_name[2] == 0)))
                continue;
            if (fstatat(dfd, dirp->d_name, &buff, AT_SYMLINK_NOFOLLOW) == 0)
                size += stat_size(&buff);

            subfd = openat(dfd, dirp->d_name, O_RDONLY | O_DIRECTORY);
            if (subfd >= 0)
                size += dir_size(subfd);
        }
        else
        {
            if (fstatat(dfd, dirp->d_name, &buff, AT_SYMLINK_NOFOLLOW) == 0)
                size += stat_size(&buff);
        }
    }
    closedir(dir);
    return size;
}

int64_t file_size(char *filename)
{
    struct stat buff;

    if (stat(filename, &buff) == -1)
    {
        printf("Failed to stat %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return buff.st_size;
}

char *calculate_size(int64_t size)
{
    if (size > 0)
    {
        char *result = (char *)malloc(20 * sizeof(char));
        int64_t s_1000 = size_1000;
        // int64_t s_1024 = size_1024;
        signed i;

        for (i = 0; i < ArrayLength(units_1000); i++, s_1000 /= 1000)
        {
            if (size < s_1000)
                continue;

            // if (size % s_1000 == 0)
            //     sprintf(result, "%ld %s", size / s_1000, units_1000[i]);
            // else
            sprintf(result, "%.1f %s", (float)size / s_1000, units_1000[i]);

            return result;
        }

        strcpy(result, "\0");
        free(result);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return 0;
    }
}

#endif /* SIZE_H_ */
