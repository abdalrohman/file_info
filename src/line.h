#ifndef LINE_H_
#define LINE_H_

/*
 * Count line of input file
 */
int line_count(char *filename)
{
    struct stat stats;
    int count = 0;
    char line;
    FILE *inputFile = fopen(filename, "r");
    if (inputFile == NULL)
    {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (lstat(filename, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
        {
            /* 
            * Extract characters from file and store in character c
            */
            for (line = getc(inputFile); line != EOF; line = getc(inputFile))
            {
                /* Increment count if this character is newline */
                if (line == '\n')
                    count = count + 1;
            }

            /*
            * (count + 1) for return the last line number
            * the last line does not have th \n charachter
            */
            return count + 1;
        }
    }
    fclose(inputFile);
    exit(EXIT_SUCCESS);
}

#endif /* LINE_H_ */