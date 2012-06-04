#include <json/json.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    char *addr = "localhost";
    char *port = "10098";
    int c;

    while ((c = getopt(argc, argv, "a:p:")) != -1) {
        switch (c) {
            case 'a':
                addr = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                break;
        }
    }

    struct addrinfo hints, *results, *p;
    int sock_fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(addr, port, &hints, &results) != 0) {
        return -1;
    }

    for (p = results; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) {
            continue;
        }
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(sock_fd);
    }
    if (p == NULL) {
        perror("connect");
        return -1;
    }
    
    freeaddrinfo(results);
    
    char *input_buf = "info";
    char output_buf[1024];
    int buf_size;

    if (optind < argc) {
        input_buf = argv[optind];
    }
    send(sock_fd, input_buf, strlen(input_buf), 0);
    buf_size = recv(sock_fd, output_buf, sizeof(output_buf), 0);
    if (buf_size == 0) {
        close(sock_fd);
        return 0;
    }
    output_buf[buf_size] = '\0';
    close(sock_fd);

    json_object *obj = json_tokener_parse(output_buf);
    const char *status = json_object_get_string(json_object_object_get(obj, "status"));
    if (strcmp(status, "error") == 0) {
        printf("%s\n", json_object_get_string(json_object_object_get(obj, "message")));
    }
    else if (strcmp(status, "stop") == 0) {
        printf("[FMD %s] [User: %s] [Channel: %d]\n",
                status, json_object_get_string(json_object_object_get(obj, "user")),
                json_object_get_int(json_object_object_get(obj, "channel")));
    }
    else {
        printf("[FMD %s] [User: %s] [Channel: %d]\n[%s - %s] [%s (%d)]\n%s[%d / %d]\n",
                status, json_object_get_string(json_object_object_get(obj, "user")),
                json_object_get_int(json_object_object_get(obj, "channel")),
                json_object_get_string(json_object_object_get(obj, "artist")),
                json_object_get_string(json_object_object_get(obj, "title")),
                json_object_get_string(json_object_object_get(obj, "album")),
                json_object_get_int(json_object_object_get(obj, "year")),
                json_object_get_int(json_object_object_get(obj, "like"))? "[Like] ": "",
                json_object_get_int(json_object_object_get(obj, "pos")),
                json_object_get_int(json_object_object_get(obj, "len"))
                );
    }
    json_object_put(obj);

    return 0;
}
