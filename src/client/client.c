#include "../conversion.c"
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

struct clientOptions
{
    int file_cnt;
    char *server_ip;
    in_port_t port_out;
};

static void send_file(FILE *fp, char *fname, int sockfd);
static void options_init(struct clientOptions *opts);
static void parse_client_arguments(int argc, char *argv[], struct clientOptions *opts);

#define DEFAULT_PORT 5000
#define SIZE 1024
const char* files[20];

int main (int argc, char *argv[]) {
    struct clientOptions opts;

    options_init(&opts);
    parse_client_arguments(argc, argv, &opts);

    int clientSocket, ret;
    struct sockaddr_in serverAddr;

    FILE *fp;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        printf("[-]Error in connection.\n");
        return EXIT_FAILURE;
    }
    printf("[+]Client socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(opts.port_out);
    serverAddr.sin_addr.s_addr = inet_addr(opts.server_ip);

    ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("[-]Error in connection.\n");
        return EXIT_FAILURE;
    }
    printf("[+]Connect to Server.\n");

    for (int i = 0; i < opts.file_cnt; i++){
        fp = fopen(files[i], "r");
        if (fp == NULL) {
            perror("[-]Error in reading file.");
            return EXIT_FAILURE;
        }
        char *fname = (char*) malloc(sizeof(files[i])+1);
        strcpy(fname, files[i]);

        printf("[+]Sending: %s....\n", fname);
        send_file(fp, fname, clientSocket);
    }
    // Indicates end of files.
    file_info info;
    strcpy(info.fname, "");
    info.fsize = 0;
    write(clientSocket, &info, sizeof(info));

    close(clientSocket);
    printf("[+]File data send successfully.\n");

    return EXIT_SUCCESS;
}

static void options_init(struct clientOptions *opts)
{
    memset(opts, 0, sizeof(struct clientOptions));
    opts->server_ip = "127.0.0.1"; //default localhost
    opts->port_out  = DEFAULT_PORT;
}

void send_file(FILE *file, char *fname, int clientSocket) {
    char* buffer;
    long numbytes;

    fseek(file, 0L, SEEK_END);
    numbytes = ftell(file);
    fseek(file, 0L, SEEK_SET);

    buffer = (char*)calloc(numbytes, sizeof(char));

    if (buffer == NULL)
        EXIT_FAILURE;

    fread(buffer, sizeof(char), numbytes, file);

    file_info info;
    strcpy(info.fname, fname);
    info.fsize = numbytes;

    write(clientSocket, &info, sizeof(info));

    if (numbytes > 0) {
        printf("File sending.. file name: %s, file size:  %ld \n", fname, numbytes);
        write(clientSocket, buffer, numbytes);
    }

    fclose(file);
    free(buffer);
}

static void parse_client_arguments(int argc, char *argv[], struct clientOptions *opts)
{
    int c;

    while ((c = getopt(argc, argv, "s:p:*:")) != -1) {
        switch(c) {
            case 's':
            {
                opts->server_ip = optarg;
                printf("Serverip : %s\n", opts->server_ip);
                break;
            }
            case 'p':
            {
                opts->port_out = parse_port(optarg, 10);
                printf("Port out : %hu\n", opts->port_out);
                break;
            }
            case ':':
            {
                fatal_message(__FILE__, __func__ , __LINE__, "\"Option requires an operand\"", 5);
                break;
            }
            case '?':
            {
                fatal_message(__FILE__, __func__ , __LINE__, "Unknown", 6);
                break;
            }
        }
    }

    int i = 0;
    //Read only txt file
    for (; optind < argc; optind++) {
        if (strstr(argv[optind], ".txt")) {
            files[i++] = argv[optind];
        }
    }

    opts->file_cnt = i;
}
