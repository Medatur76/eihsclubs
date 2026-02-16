#include "server.h"

int git_pull();

char *http202 = "HTTP/1.1 202 Accepted\r\nTransfer-Encoding: chunked\r\n\r\n", *http500 = "HTTP/1.1 500 Internal Server Error\r\nTransfer-Encoding: chunked\r\n\r\n";

int api_handler(int client, Method method, char *request, size_t request_size) {
    int code = EXIT_SUCCESS;
    //TODO Make this less hardcoded if I expand on this
    if (method == POST && strcmp(request, "pushEvent") == 0) {
        signal(SIGCHLD, SIG_DFL);  // Restore default SIGCHLD handling for git_pull
        int output = git_pull();
        if (output != 0) {
            write(client, http500, strlen(http500));
            code = EXIT_FAILURE;
        } else {
            write(client, http202, strlen(http202));
        }
        writeFileToSocket(open("web/api/.hidden/pushEvent", O_RDONLY), client);
    } else if (method == GET) {
        size_t targetSize = 1;
        char *domainTarget = mmap(NULL, targetSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), current;
        while (targetSize <= request_size && (current = request[targetSize-1]) != '/' && current != '\0') {
            domainTarget[targetSize-1] = current;
            domainTarget = mremap(domainTarget, targetSize, ++targetSize, MREMAP_MAYMOVE);
        }
        domainTarget[--targetSize] = '\0';
    }
    return code;
}