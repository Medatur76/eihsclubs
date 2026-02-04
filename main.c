#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#define errclose exit_code = EXIT_FAILURE; goto cleanup

//TODO Make this stop from going from one domain to another (e.g. eihsclubs.com/../outlet/index.html)
//And fix the output in the console (e.g. web/eihsclubs/index(web/eihsclubs/index.html (200 text/html), and overlapped messages)
bool isSafePath(char *);
char *parseHost(int, char *, size_t);
void print(char *);
char *format(size_t, const char *__restrict format, ...);
const char *get_mime_type(const char *);

int main() {
    int server_fd, client_socket;
    char *readAddr;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    size_t writeBufSize = 16384;
    char *http200 = "HTTP/1.1 200 OK\r\nContent-Type: %s; charset=UTF-8\r\nTransfer-Encoding: chunked\r\n\r\n",
    *http404 = "HTTP/1.1 404 Not Found\r\n\r\n404",
    *http403 = "HTTP/1.1 403 Forbidden\r\n\r\n";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 50) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    print("Listening on port 8080...\n");

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket <= 0) {
            perror("client accept error");
            exit(EXIT_FAILURE);
        }

        signal(SIGCHLD, SIG_IGN);
        int process = fork();
        
        if (process < 0) {
            perror("Forking error");
            exit(EXIT_FAILURE);
        } else if (process != 0) continue;
        int exit_code = EXIT_SUCCESS;
        char *readBuf = malloc(1), *writeBuf = malloc(writeBufSize);
        read(client_socket, readBuf, 1);
        while (readBuf[0] != '/') read(client_socket, readBuf, 1);
        size_t readSize = 1;
        readAddr = mmap(NULL, readSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (readAddr == MAP_FAILED) {
            perror("MAPPING FAIL");
            errclose;
        }
        bool fileExt = false;
        read(client_socket, readBuf, 1);
        if (readBuf[0] == ' ') {
            readAddr = mremap(readAddr, readSize, readSize += 4, MREMAP_MAYMOVE);
            memcpy(readAddr + readSize - 5, "index", 5);
        } else {
            //Should change this to read faster
            while (1) {
                readAddr[readSize - 1] = readBuf[0];
                read(client_socket, readBuf, 1);
                if (readBuf[0] == ' ') break;
                if (readBuf[0] == '.') fileExt = true;
                readAddr = mremap(readAddr, readSize, ++readSize, MREMAP_MAYMOVE);
            }
        }
        char *fileAddr = parseHost(client_socket, readAddr, readSize);
        print(fileAddr);

        int filePtr = open(fileAddr, O_RDONLY);
        if (filePtr < 0) {
            if (!fileExt) {
                size_t tmpSize;
                fileAddr = realloc(fileAddr, tmpSize = strlen(fileAddr) + 6);
                memcpy(fileAddr + tmpSize - 6, ".html\0", 6);
                filePtr = open(fileAddr, O_RDONLY);
                if (filePtr < 0) {
                    write(client_socket, http404, strlen(http404));
                    print("(404)\n");
                    errclose;
                }
            } else {
                write(client_socket, http404, strlen(http404));
                print("(404)\n");
                errclose;
            }
            print("(");
            print(fileAddr);
            print(" ");
        }
        const char *type = get_mime_type(fileAddr);
        if (type == NULL || !isSafePath(fileAddr)) {
            write(client_socket, http403, strlen(http403));
            print("(403)");
            goto cleanup;
        }
        print("(200 ");
        print((char *) type);
        print(")\n");
        char *header = format(strlen(http200) + 25, http200, get_mime_type(fileAddr));
        write(client_socket, header, strlen(header));
        free(header);
        int iRead;
        while ((iRead = read(filePtr, writeBuf, writeBufSize)) > 0) {
            char *chunkHeader = format(32, "%x\r\n", iRead);
            write(client_socket, chunkHeader, strlen(chunkHeader));
            int totalWritten = 0;
            while (totalWritten < iRead) {
                int iWritten = write(client_socket, writeBuf + totalWritten, iRead - totalWritten);
                if (iWritten < 0) {
                    perror("write to client");
                    errclose;
                }
                totalWritten += iWritten;
            }
            write(client_socket, "\r\n", 2);
            free(chunkHeader);
        }
        if (iRead < 0) {
            perror("read file");
            errclose;
        }
        write(client_socket, "0\r\n\r\n", 5);
        cleanup:
        if (filePtr >= 0) close(filePtr);
        shutdown(client_socket, SHUT_WR);
        close(client_socket);
        munmap(readAddr, readSize);
        free(fileAddr);
        free(readBuf);
        free(writeBuf);
        exit(exit_code);
    }
    close(server_fd);
    return 0;
}


/*ChatGPT again cuz i was NOT bouta figure out linux filesystems*/
bool isSafePath(char *request) {
    char webFolder[PATH_MAX];
    char requestedFile[PATH_MAX];

    if (!realpath("web/", webFolder)) {
        perror("realpath(\"web/\", webFolder)");
        return false;
    }

    if (!realpath(request, requestedFile)) {
        perror("realpath(request, requestedFile)");
        return false;
    }

    size_t len = strlen(webFolder);

    // Folder must match prefix exactly
    if (strncmp(requestedFile, webFolder, len) != 0)
        return 0;

    // Ensure boundary ("/" or end of string)
    return requestedFile[len] == '\0' || requestedFile[len] == '/';
}


void print(char *data) {
    int len = strlen(data);
    write(1, data, len);
}

char *format(size_t maxlen, const char *__restrict format, ...) {
    char *out = malloc(maxlen);
    
    va_list ap;
    va_start(ap, format);
    vsnprintf(out, maxlen, format, ap);
    va_end(ap);

    return out;
}

char *parseHost(int clientFd, char *readAddr, size_t memSize) {
    char *readBuf = malloc(1), *testBuf = malloc(5), *output = NULL;
    while (read(clientFd, readBuf, 1) == 1) {
        if (readBuf[0] != '\n' && readBuf[0] != '\r') continue;
        read(clientFd, readBuf, 1);
        size_t testSize;
        if (testSize = read(clientFd, testBuf, 5) != 5) break;
        if (strncmp(testBuf, "Host:", testSize) != 0) continue;
        read(clientFd, readBuf, 1);
        if (readBuf[0] == ' ') read(clientFd, readBuf, 1);
        size_t domainSize = 1;
        char *domainBuf = mmap(NULL, domainSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        domainBuf[0] = readBuf[0];
        read(clientFd, readBuf, 1);
        while (readBuf[0] != '\r' && readBuf[0] != '\n' && readBuf[0] != '.') {
            domainBuf = mremap(domainBuf, domainSize, ++domainSize, MREMAP_MAYMOVE);
            domainBuf[domainSize - 1] = readBuf[0];
            read(clientFd, readBuf, 1);
        }
        output = format(memSize + 6 + domainSize, "web/%s/%s\0", domainBuf, readAddr);
        munmap(domainBuf, domainSize);
        break;
    }
    free(readBuf);
    free(testBuf);
    return output == NULL ? format(memSize + 15, "web/eihsclubs/%s\0", readAddr) : output;
}

/*ChatGPT made this. I will make a better version in the loop for ASM
TODO Add restricted file types instead of default case*/
const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (ext == NULL) return "text/plain";
    ext++; // skip '.'

    if (!strcasecmp(ext, "html")) return "text/html";
    if (!strcasecmp(ext, "css"))  return "text/css";
    if (!strcasecmp(ext, "js"))   return "application/javascript";
    if (!strcasecmp(ext, "json")) return "application/json";
    if (!strcasecmp(ext, "png"))  return "image/png";
    if (!strcasecmp(ext, "jpg") || !strcasecmp(ext, "jpeg"))
        return "image/jpeg";
    if (!strcasecmp(ext, "gif"))  return "image/gif";
    if (!strcasecmp(ext, "svg"))  return "image/svg+xml";
    if (!strcasecmp(ext, "pdf"))  return "application/pdf";
    if (!strcasecmp(ext, "webp")) return "image/webp";
    if (!strcasecmp(ext, "ico")) return "image/vnd.microsoft.icon";

    //return "application/octet-stream";
    return NULL;
}
