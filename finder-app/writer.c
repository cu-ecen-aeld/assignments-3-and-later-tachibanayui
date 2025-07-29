#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <dir> <search>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_CREAT | O_WRONLY, 0644);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    write(fd, argv[2], strlen(argv[2]));
    openlog("writer", 0, LOG_USER);
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    closelog();
    close(fd);
}