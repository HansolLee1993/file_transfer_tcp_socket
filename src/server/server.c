#include "../conversion.c"
#include "../utils.c"
#include "../error.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct fileInfo {
    char fname[NAME_MAX + 1];
    long fsize;
} file_info;

struct serverOptions
{
    char *file_location;
    char *ip_in;
    in_port_t port_in;
};

static void options_init(struct serverOptions *opts);
static void parse_server_arguments(int argc, char *argv[], struct serverOptions *opts);
static int download_file(int newSocket, struct serverOptions *opts);

#define SIZE 1024
#define DEFAULT_FILE_LOCATION "~/server/downloads"
#define DEFAULT_PORT 5000
#define MAX_PENDING 10

int main (int argc, char *argv[]) {
    struct serverOptions opts;

    options_init(&opts);
    parse_server_arguments(argc, argv, &opts);

    int sockfd, ret;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;

    pid_t childip;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[-]Error in connection.\n");
        return EXIT_FAILURE;
    }
    printf("[+]Server socket created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(opts.port_in);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind ip address to specific port
    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("[-]Error in binding\n");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, MAX_PENDING) == 0)
        printf("Listening...\n");
    else
        printf("[-]Error in binding.\n");

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);

        if (newSocket < 0)
            EXIT_FAILURE;

        //set current input ip
        opts.ip_in = inet_ntoa(newAddr.sin_addr);

        printf("Connection accept from %s:%d\n", opts.ip_in, opts.port_in);

        if ((childip = fork()) == 0) {
            close(sockfd);

            if (download_file(newSocket, &opts) == 0) {
                printf("[+]File downloaded.\n");
                break;
            }
        }
    }

    close(newSocket);
    return EXIT_SUCCESS;
}


static void parse_server_arguments(int argc, char *argv[], struct serverOptions *opts) {
    int c;
    while ((c = getopt(argc, argv, "d:p:")) != -1) {
        switch(c) {
            case 'd':
            {
                opts->file_location = optarg;
                break;
            }
            case 'p':
            {
                opts->port_in = parse_port(optarg, 10);
                break;
            }
            case ':':
            {
                fatal_message(__FILE__, __func__ , __LINE__, "\"Option requires an operand\"", 5);
            }
            case '?':
            {
                fatal_message(__FILE__, __func__ , __LINE__, "Unknown", 6);
            }
        }
    }
}

static int download_file(int newSocket, struct serverOptions *opts) {
    char path[FILENAME_MAX];

    strcpy(path, opts->file_location);
    strcat(path, "/");
    strcat(path, opts->ip_in);
    strcat(path, "/");

    //create directory
    create_directory(path);

    while (1){
        file_info finfo;
        read(newSocket, &finfo, sizeof(file_info));

        if (strlen(finfo.fname) == 0)
            break;

        char buffer[SIZE];
        //Get the content
        read(newSocket, buffer, finfo.fsize);

        FILE *fp;

        printf("[+]%s downloading....\n", finfo.fname);
        fp = fopen(append(path, finfo.fname),  "w");
        if (fp == NULL) {
            perror("[-]Error in creating file.");
            EXIT_FAILURE;
        }

        //write the content to file
        fprintf(fp, buffer, finfo.fsize);
    }

    return EXIT_SUCCESS;
}

static void options_init(struct serverOptions *opts) {
    memset(opts, 0, sizeof(struct serverOptions));
    opts->port_in  = DEFAULT_PORT;
    opts->file_location = DEFAULT_FILE_LOCATION;
}