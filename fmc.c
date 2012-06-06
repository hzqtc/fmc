#include <curl/curl.h>
#include <json/json.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef struct {
    int id;
    char name[32];
} fm_channel_t;

fm_channel_t channels[128];

void read_channels()
{
    const char *file = "/tmp/fm_channels";
    char buf[4096];
    size_t size;
    int fd;
    int i;

    for (i = 0; i < 128; i++) {
        channels[i].id = -1;
    }

    fd = open(file, O_RDONLY);
    if (fd >= 0) {
        memset(buf, 0, sizeof(buf));
        size = read(fd, buf, sizeof(buf));
        if (size > 0) {
            json_object *obj = json_tokener_parse(buf);
            array_list *channel_objs = json_object_get_array(json_object_object_get(obj, "channels"));

            for (i = 0; i < array_list_length(channel_objs); i++) {
                json_object *o = (json_object*) array_list_get_idx(channel_objs, i);
                int id = json_object_get_int(json_object_object_get(o, "channel_id"));
                const char *name = json_object_get_string(json_object_object_get(o, "name"));
                channels[id].id = id;
                strcpy(channels[id].name, name);
            }

            json_object_put(obj);
        }
        close(fd);
    }
    else {
        FILE *f = fopen(file, "w");
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.douban.com/j/app/radio/channels");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(f);
        read_channels();
    }
}

void time_str(int time, char *buf)
{
    int sec = time % 60;
    int min = time / 60;
    sprintf(buf, "%d:%02d", min, sec);
}

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

    read_channels();
    
    char input_buf[64];
    char output_buf[1024];
    int buf_size;

    if (optind < argc) {
        strcpy(input_buf, argv[optind]);
        int i;
        for (i = optind + 1; i < argc; i++) {
            strcat(input_buf, " ");
            strcat(input_buf, argv[i]);
        }
    }
    else {
        strcpy(input_buf, "info");
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
    else{
        printf("FMD %s - ", strcmp(status, "play") == 0? "Playing": (strcmp(status, "pause") == 0? "Paused": "Stopped"));
        int c_id = json_object_get_int(json_object_object_get(obj, "channel"));
        if (channels[c_id].id < 0) {
            printf("Channel %d\n", c_id);
        }
        else {
            printf("%s\n", channels[c_id].name);
        }

        if (strcmp(status, "stop") != 0) {
            char pos[16], len[16];
            time_str(json_object_get_int(json_object_object_get(obj, "pos")), pos);
            time_str(json_object_get_int(json_object_object_get(obj, "len")), len);
            printf("%s%s - %s\n%s / %s\n",
                    json_object_get_int(json_object_object_get(obj, "like"))? "[Like] ": "",
                    json_object_get_string(json_object_object_get(obj, "artist")),
                    json_object_get_string(json_object_object_get(obj, "title")), pos, len);
        }
    }
    json_object_put(obj);

    return 0;
}
