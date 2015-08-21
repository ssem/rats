#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>


int which(char* program) {
    char *path = NULL;
    char *paths = strdup(getenv("PATH"));
    DIR *dp;
    struct dirent *ep;
    char *filename;
    int match;
    path = strtok(paths, ":");
    while(path != NULL) {
        dp = opendir(path);
        if (dp != NULL) {
            while (ep = readdir(dp)) {
                match = strcmp(program, ep->d_name);
                if (match == 0) {
                    return 1;
                }
            }
            (void) closedir(dp);
        }
        path = strtok(NULL, ":");
    }
    return 0;
}

int discover(void)
{
    if (which("avggui")) {
        printf("avg\n");
    }
    if (which("avast")) {
        printf("avast\n");
    }
    if (which("bdscan")) {
        printf("bitdefender");
    }
    if (which("clamscan")) {
        printf("clamav\n");
    }
    if (which("fpscan")) {
        printf("f-prot\n");
    }
}
