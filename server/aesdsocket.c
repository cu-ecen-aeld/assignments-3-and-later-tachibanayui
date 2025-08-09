#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

#define UNWRAP(caller, msg) \
    if ((caller) < 0)       \
    {                       \
        perror(msg);        \
        exit(-1);           \
    }

// From https://github.com/cu-ecen-aeld/aesd-lectures/blob/master/lecture9/signal_handler.c
bool caught_sigint = false;
bool caught_sigterm = false;
static void signal_handler(int signal_number)
{
    /**
     * Save a copy of errno so we can restore it later.  See https://pubs.opengroup.org/onlinepubs/9699919799/
     * "Operations which obtain the value of errno and operations which assign a value to errno shall be
     *  async-signal-safe, provided that the signal-catching function saves the value of errno upon entry and
     *  restores it before it returns."
     */
    int errno_saved = errno;
    if (signal_number == SIGINT)
    {
        caught_sigint = true;
    }
    else if (signal_number == SIGTERM)
    {
        caught_sigterm = true;
    }
    errno = errno_saved;
}

void send_log(int logfd, int sfd)
{
    char buf[1024];
    size_t wb = 0;
    UNWRAP(lseek(logfd, 0, SEEK_SET), "Failed to seek!");
    while ((wb = read(logfd, buf, sizeof(buf))) > 0)
    {
        UNWRAP(send(sfd, buf, wb, 0), "Failed to send");
    }
    UNWRAP(wb, "Failed to read log");
    return;
}

int main(int argc, char *argv[])
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Fail to open socket!");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    UNWRAP(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)), "Fail to bind to 0.0.0.0:9000!")
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        switch (fork())
        {
        case -1:
            perror("Failed to fork!");
            return -1;
        case 0:
            break;
        default:
            return 0;
        }
    }

    UNWRAP(listen(sockfd, 100), "Fail to listen to socket");
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    UNWRAP(sigaction(SIGTERM, &new_action, NULL), "Failed to register SIGTERM handler")
    UNWRAP(sigaction(SIGINT, &new_action, NULL), "Failed to register SIGINT handler")
    char buf[1024];
    int logfd;
    UNWRAP(logfd = open("/var/tmp/aesdsocketdata", O_CREAT | O_APPEND | O_RDWR, 0644), "Fail to open log file");
    openlog("aesdsocket", 0, LOG_USER);
    while (!(caught_sigint || caught_sigterm))
    {
        struct sockaddr_in client;
        socklen_t len = 0;
        int sfd = accept(sockfd, (struct sockaddr *)&client, &len);
        if (sfd < 0)
        {
            if (caught_sigint || caught_sigterm)
            {
                break;
            }

            perror("Failed to accept connection!");
            return -1;
        }

        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in *)&client)->sin_addr, ipstr, sizeof(ipstr));
        syslog(LOG_INFO, "Accepted connection from %s", ipstr);

        size_t wb;
        while ((wb = recv(sfd, buf, sizeof(buf), 0)) > 0)
        {
            UNWRAP(write(logfd, buf, wb), "Failed to write log");
            if (buf[wb - 1] == '\n')
            {
                send_log(logfd, sfd);
            }
        }
        UNWRAP(wb, "Failed to recv");
        UNWRAP(close(sfd), "Failed to close session socket!");
        syslog(LOG_INFO, "Closed connection from %s", ipstr);
    }

    UNWRAP(close(logfd), "Failed to close log file!");
    UNWRAP(close(sockfd), "Failed to close socket!");
    UNWRAP(remove("/var/tmp/aesdsocketdata"), "Fail to remove log file!");
    syslog(LOG_INFO, "Caught signal, exiting");
    return 0;
}