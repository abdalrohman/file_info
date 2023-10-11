#ifndef COUNT_H_
#define COUNT_H_

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#include <string.h>

/* 
 *  A structure to hold separate file, directory and symlink counts 
 */
struct filecount
{
    unsigned long dirs;
    unsigned long files;
    unsigned long link;
};

/*
 * counts the number of files and directories in the specified directory.
 *
 * path - relative pathname of a directory whose files should be counted
 * counts - pointer to struct containing file/dir/link counts
 */
int count_file(char *path, struct filecount *counts)
{
    DIR *dir;
    struct dirent *dirp;
    char *npath;

    // Opening dir
    dir = opendir(path);
    /* opendir failed... file likely doesn't exist or isn't a directory */
    if (dir == NULL)
    {
        perror(path);
        exit(EXIT_FAILURE);
    }

    while ((dirp = readdir(dir)))
    {
        if (dirp->d_name[0] == '.' && (dirp->d_name[1] == 0 || (dirp->d_name[1] == '.' && dirp->d_name[2] == 0)))
            continue;

        if (dirp->d_type == DT_REG)
        {
            counts->files++;
        }
        if (dirp->d_type == DT_DIR)
        {
            npath = malloc(strlen(path) + strlen(dirp->d_name) + 2);
            sprintf(npath, "%s%c%s", path, PATH_SEPARATOR, dirp->d_name);
            counts->dirs++;
            count_file(npath, counts);
            free(npath);
        }
        if (dirp->d_type == DT_LNK)
        {
            counts->link++;
        }
    }
    closedir(dir);
    return 0;
}

#endif /* COUNT_H_ */