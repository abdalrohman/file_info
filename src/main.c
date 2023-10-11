#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h> /* localtime()   ,  asctime()*/
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
#include "count.h"
#include "line.h"
#include "size.h"

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define BUF_LEN 256

static const char *program_name;

void printPermissions(struct stat stats);
char *printModTime(struct stat stats);
void short_mod(char *pathname);
void long_mod(char *pathname);
void usage(int stats);

int main(int argc, char *argv[])
{
  program_name = argv[0];

  /* Set default is the current directory */
  if (argc < 2)
  {
    // open current directory
    DIR *dir = opendir(".");
    if (dir == NULL)
    {
      exit(EXIT_FAILURE);
    }
    struct stat stats;
    struct dirent *dirp;
    char filePath[1024];

    while ((dirp = readdir(dir)) != NULL)
    {
      /* always skip "." and ".." */
      if ((dirp->d_name[1] == '.' && dirp->d_name[2] == 0))
        continue;

      sprintf(filePath, "%s", dirp->d_name);
      if (lstat(filePath, &stats) != 0)
        continue;

      // Print in short mode.
      short_mod(filePath);
    }
    closedir(dir);
    exit(EXIT_SUCCESS);
  }

  /* TODO 
     show hiden file with support flag (-a). 
     calculate size in 1024 with flag (-s). */

  /* Support command line aurgment. */
  int c;
  int short_flag, long_flag, help_flag;
  short_flag = 0;
  long_flag = 0;
  help_flag = 0;

  while (1)
  {
    c = getopt(argc, argv, "slh");
    if (c == -1)
      break;

    switch (c)
    {
    case 's':
      short_flag = 1;
      break;
    case 'l':
      long_flag = 1;
      break;
    case '?':
    case 'h':
      usage(EXIT_SUCCESS);
      help_flag = 1;
      break;
    default:
      usage(EXIT_FAILURE);
    }
  }

  /* Short mode. */
  if (short_flag)
  {
    while (optind < argc)
    {
      short_mod(argv[optind++]);
    }
    exit(EXIT_SUCCESS);
  }
   
  /* Long mode. */
  if (long_flag)
  {
    while (optind < argc)
    {
      long_mod(argv[optind++]);
      // Print line after each file/folder
      printf("_________________________________________________\n");
    }
    exit(EXIT_SUCCESS);
  }

  /* Usage */
  if (help_flag)
  {
    usage(EXIT_SUCCESS);
  }
  exit(EXIT_SUCCESS);
}

/*
 * Function for print file and folder info in long mode
 * 
 * if file: 
     File name: (name)
          Line count:              lines
          Permissions:            -rwxrwxrwx
          Usr/Grp:                (usr/grp)
          Gid/Uid:                (gid/uid)
          Size:                   size KB/MB/GB
          Modified:               Day Mon h:m:s year
          Inode:                  inode

 * if folder: 
     Folder name: (name)
          Counts:                 { files, directories, symlink}
          Permissions:            drwxrwxrwx
          Usr/Grp:                (usr/grp)
          Gid/Uid:                (gid/uid)
          Size:                   size KB/MB/GB
          Modified:               Day Mon h:m:s year 
          Inode:                  inode
 */
void long_mod(char *pathname)
{
  struct stat buff;
  uid_t uid = getuid();
  gid_t gid = getgid();
  struct group *grp = getgrgid(gid);
  struct passwd *pw = getpwuid(uid);
  char *buf;
  ssize_t nbytes, bufsiz;

  if (lstat(pathname, &buff) == 0)
  {

    if (S_ISDIR(buff.st_mode))
    {
      struct filecount counts;

      counts.dirs = 0;
      counts.files = 0;
      counts.link = 0;
      printf("Folder name: (%s)\n", pathname);

      count_file(pathname, &counts);
      printf("\tCounts:\t\t\t{%ld files,", counts.files);
      printf("%ld directories,", counts.dirs);
      printf("%ld symlink}\n", counts.link);

      printf("\tPermissions:\t\t");
      printPermissions(buff);
      printf("\n");

      // Print uid, gid
      printf("\tUsr/Grp:\t\t(%s/%s)\n", pw->pw_name, grp->gr_name);
      printf("\tGid/Uid:\t\t(%d/%d)\n", pw->pw_gid, pw->pw_uid);

      // Print size
      int dfd = open(pathname, O_RDONLY);
      printf("\tSize:\t\t\t%s\n", calculate_size(dir_size(dfd)));

      // Print last modification time
      printf("\tModified:\t\t%s\n", printModTime(buff));

      // Print inode
      printf("\tInode:\t\t\t%ld\n", buff.st_ino);
      close(dfd);
    }
    else if (S_ISREG(buff.st_mode))
    {
      printf("File name: (%s)\n", pathname);

      printf("\tLine count:\t\t%d lines\n", line_count(pathname));

      // Print permissions
      printf("\tPermissions:\t\t");
      printPermissions(buff);
      printf("\n");

      // Print uid, gid
      printf("\tUsr/Grp:\t\t(%s/%s)\n", pw->pw_name, grp->gr_name);
      printf("\tGid/Uid:\t\t(%d/%d)\n", pw->pw_gid, pw->pw_uid);

      // Print size
      printf("\tSize:\t\t\t%s\n", calculate_size(file_size(pathname)));

      // Print last modification time
      printf("\tModified:\t\t%s\n", printModTime(buff));

      // Print inode
      printf("\tInode:\t\t\t%ld\n", buff.st_ino);
    }
    if (S_ISLNK(buff.st_mode))
    {
      printf("File name: (%s)\n", pathname);

      // Print permissions
      printf("\tPermissions:\t\t");
      printPermissions(buff);
      printf("\n");

      // Print uid, gid
      printf("\tUsr/Grp:\t\t(%s/%s)\n", pw->pw_name, grp->gr_name);
      printf("\tGid/Uid:\t\t(%d/%d)\n", pw->pw_gid, pw->pw_uid);

      // Print size
      printf("\tSize:\t\t\t%s\n", calculate_size(file_size(pathname)));

      // Print last modification time
      printf("\tModified:\t\t%s\n", printModTime(buff));

      // Print inode
      printf("\tInode:\t\t\t%ld\n", buff.st_ino);
      bufsiz = buff.st_size + 1;
      buf = malloc(bufsiz);
      if (buf == NULL)
      {
        perror("malloc");
        exit(EXIT_FAILURE);
      }

      nbytes = readlink(pathname, buf, bufsiz);
      if (nbytes == -1)
      {
        perror("readlink");
        exit(EXIT_FAILURE);
      }

      printf("\tSymbolic link to:\t%.*s\n", (int)nbytes, buf);

      if (nbytes == bufsiz)
        printf("(Returned buffer may have been truncated)\n");

      free(buf);
    }
  }
}

/*
 * Function for print file and folder info in short mode
 * if file: permission, size, lines, ModDate     filename
 * if folder: permission, size,counts{files,dir,link}     direname
 */
void short_mod(char *pathname)
{
  struct stat buff;
  char *buf;
  ssize_t nbytes, bufsiz;
  if (lstat(pathname, &buff) == 0)
  {
    printPermissions(buff);
    printf(", ");

    if (S_ISDIR(buff.st_mode))
    {
      struct filecount counts;

      counts.dirs = 0;
      counts.files = 0;
      counts.link = 0;

      // Print size
      int dfd = open(pathname, O_RDONLY);
      printf("%10s,", calculate_size(dir_size(dfd)));

      count_file(pathname, &counts);
      printf(" %6ldF:", counts.files);
      printf(" %ldD:", counts.dirs);
      printf(" %ldS, ", counts.link);

      // Print last modification time
      printf("%17s", printModTime(buff));

      printf(ANSI_COLOR_GREEN "\t%s\n" ANSI_COLOR_RESET, basename(pathname));

      close(dfd);
    }
    else if (S_ISREG(buff.st_mode))
    {
      // Print size
      printf("%10s,", calculate_size(file_size(pathname)));

      printf("%10d lines, ", line_count(pathname));

      // Print last modification time
      printf("%17s", printModTime(buff));

      printf(ANSI_COLOR_BLUE "\t%s\n" ANSI_COLOR_RESET, basename(pathname));
    }
    if (S_ISLNK(buff.st_mode))
    {
      bufsiz = buff.st_size + 1;
      buf = malloc(bufsiz);
      if (buf == NULL)
      {
        perror("malloc");
        exit(EXIT_FAILURE);
      }

      nbytes = readlink(pathname, buf, bufsiz);
      if (nbytes == -1)
      {
        perror("readlink");
        exit(EXIT_FAILURE);
      }

      // Print last modification time
      printf("%17s", printModTime(buff));
      printf(ANSI_COLOR_MAGENTA "\t'%s' -> '%.*s'\n" ANSI_COLOR_RESET,
             basename(pathname), (int)nbytes, buf);

      if (nbytes == bufsiz)
        printf("(Returned buffer may have been truncated)\n");

      free(buf);
    }
  }
}

/*
 * Return a string of modfication time of file
 */
char *printModTime(struct stat stats)
{
  struct tm *timeinfo;

  /* 
   * Convert to local time format.
   * Use strtok to prevent print new line from asctime() function.
   */
  char buf[BUF_LEN] = {0};

  timeinfo = localtime(&stats.st_ctime);
  strftime(buf, BUF_LEN, "%d %m-%Y %H:%M", timeinfo);
  // return strtok(asctime(timeinfo), "\n");
  return strtok(buf, "\n");
}

/* 
 * Function for print permissons 
 * in this format (drwxrwxrwx).
 */
void printPermissions(struct stat stats)
{
  printf((stats.st_mode & S_IFDIR) ? "d" : "-");
  printf((stats.st_mode & S_IRUSR) ? "r" : "-"); // Read
  printf((stats.st_mode & S_IWUSR) ? "w" : "-"); // Write
  printf((stats.st_mode & S_IXUSR) ? "x" : "-"); // Execute
  printf((stats.st_mode & S_IRGRP) ? "r" : "-");
  printf((stats.st_mode & S_IWGRP) ? "w" : "-");
  printf((stats.st_mode & S_IXGRP) ? "x" : "-");
  printf((stats.st_mode & S_IROTH) ? "r" : "-");
  printf((stats.st_mode & S_IWOTH) ? "w" : "-");
  printf((stats.st_mode & S_IXOTH) ? "x" : "-");
}

/* 
 * Function for print usage information.
 */
void usage(int stats)
{
  printf("Usage: %s [OPTION]... [FILE]...\n", program_name);
  fputs("\
List information about the FILEs/FOLDERs (the current directory by default).\n\
\n\
",
        stdout);
  fputs("\
  -s,                        print info about files/folder in short mode(default).\n\
  -l,                        print info about files/folder in long mode.\n\
  -h/-?,                     print help message.\n\
",
        stdout);
  exit(stats);
}
