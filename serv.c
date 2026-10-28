#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK"

static volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    (void)sig;
    stop = 1;
}

int main(int argc, char const* argv[])
{
    int n, val, idx;
    char buf[1024];
    char sbuf[1024];
    char *token, *t2, *str;
    list_t *mylist;

    // Ctrl-C handler: no SA_RESTART so accept()/recv() get interrupted
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind");
        return 1;
    }
    listen(servSockD, 1);

    mylist = list_alloc();

    int clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        list_free(mylist);
        close(servSockD);
        return 0;
    }

    while (!stop) {
        memset(buf, '\0', sizeof(buf));
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;          // disconnect, error, or Ctrl-C
        buf[n] = '\0';

        memset(sbuf, '\0', sizeof(sbuf));
        token = strtok(buf, " ");
        if (token == NULL) continue;

        if (strcmp(token, "exit") == 0) {
            break;
        }
        else if (strcmp(token, "print") == 0) {
            str = listToString(mylist);
            snprintf(sbuf, sizeof(sbuf), "%s", str ? str : "");
            free(str);
        }
        else if (strcmp(token, "get_length") == 0) {
            snprintf(sbuf, sizeof(sbuf), "Length = %d", list_length(mylist));
        }
        else if (strcmp(token, "add_back") == 0) {
            token = strtok(NULL, " ");
            if (!token) snprintf(sbuf, sizeof(sbuf), "Error: missing value");
            else {
                val = atoi(token);
                list_add_to_back(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            }
        }
        else if (strcmp(token, "add_front") == 0) {
            token = strtok(NULL, " ");
            if (!token) snprintf(sbuf, sizeof(sbuf), "Error: missing value");
            else {
                val = atoi(token);
                list_add_to_front(mylist, val);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            }
        }
        else if (strcmp(token, "add_position") == 0) {
            token = strtok(NULL, " ");     // index
            t2 = strtok(NULL, " ");        // value
            if (!token || !t2) snprintf(sbuf, sizeof(sbuf), "Error: usage add_position <index> <value>");
            else {
                idx = atoi(token);
                val = atoi(t2);
                list_add_at_index(mylist, val, idx);   // your list.h order: (l, value, index)
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            }
        }
        else if (strcmp(token, "remove_back") == 0) {
            val = list_remove_from_back(mylist);
            snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
        }
        else if (strcmp(token, "remove_front") == 0) {
            val = list_remove_from_front(mylist);
            snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
        }
        else if (strcmp(token, "remove_position") == 0) {
            token = strtok(NULL, " ");
            if (!token) snprintf(sbuf, sizeof(sbuf), "Error: missing index");
            else {
                idx = atoi(token);
                val = list_remove_at_index(mylist, idx);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            }
        }
        else if (strcmp(token, "get") == 0) {
            token = strtok(NULL, " ");
            if (!token) snprintf(sbuf, sizeof(sbuf), "Error: missing index");
            else {
                idx = atoi(token);
                val = list_get_elem_at(mylist, idx);
                snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
            }
        }
        else if (strcmp(token, "menu") == 0) {
            snprintf(sbuf, sizeof(sbuf), "menu");
        }
        else {
            snprintf(sbuf, sizeof(sbuf), "Unknown command");
        }

        send(clientSocket, sbuf, strlen(sbuf) + 1, 0);
    }

    // graceful cleanup
    list_free(mylist);
    close(clientSocket);
    close(servSockD);
    return 0;
}