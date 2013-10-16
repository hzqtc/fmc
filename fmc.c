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

#define channel_max 128
#define local_channel 999
fm_channel_t channels[channel_max];

void read_channels()
{
    const char *file = "/tmp/fm_channels";
    char buf[4096];
    size_t size;
    int fd;
    int i;

    for (i = 0; i < channel_max; i++) {
        channels[i].id = -1;
    }

    fd = open(file, O_RDONLY);
    if (fd >= 0) {
        memset(buf, 0, sizeof(buf));
        size = read(fd, buf, sizeof(buf));
        if (size > 0) {
            json_object *obj = json_tokener_parse(buf);
            if (obj) {
                array_list *channel_objs = json_object_get_array(json_object_object_get(obj, "channels"));
                if (channel_objs) {
                    for (i = 0; i < array_list_length(channel_objs); i++) {
                        json_object *o = (json_object*) array_list_get_idx(channel_objs, i);
                        int id = json_object_get_int(json_object_object_get(o, "channel_id"));
                        const char *name = json_object_get_string(json_object_object_get(o, "name"));
                        channels[id].id = id;
                        strcpy(channels[id].name, name);
                    }
                }
                json_object_put(obj);
            }
        }
        close(fd);
    }
    else {
        FILE *f = fopen(file, "w");
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.douban.com/j/app/radio/channels?app_name=radio_desktop_win&version=100");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(f);
        read_channels();
    }
}

char *get_local_channel_name() {
    char *login = "";
    login = getlogin();
    if (!login)
        login = "Red-Heart";
    return login;
}

void print_channels()
{
    int i;
    printf("%3s %s\n", "id", "name");
    printf("%3d %s\n", local_channel, get_local_channel_name());
    for (i = 0; i < channel_max; i++) {
        if (channels[i].id >= 0) {
            printf("%3d %s\n", channels[i].id, channels[i].name);
        }
    }
}

void print_usage()
{
    printf("Usage: fmc [-a address] [-p port] [cmd] [argument]\n"
           "       fmc help          - show this help infomation\n"
           "       fmc info [format] - show current fmd information\n"
           "                           if the format argument is given, the following specifier will be replaced accordingly\n"
           "                           %%a -- artist \n"
           "                           %%t -- song title \n"
           "                           %%b -- album \n"
           "                           %%y -- release year \n"
           "                           %%i -- cover image \n"
           "                           %%d -- douban url \n"
           "                           %%c -- channel \n"
           "                           %%p -- currtime \n"
           "                           %%l -- totaltime \n"
           "                           %%u -- status \n"
           "                           %%k -- kbps \n"
           "                           %%r -- rate (0 or 1) \n"
           "                           %%%% -- a literal %% \n"
           "       fmc play          - start playback\n"
           "       fmc pause         - pause playback\n"
           "       fmc toggle        - toggle between play and pause\n"
           "       fmc stop          - stop playback\n"
           "       fmc skip/next     - skip current song\n"
           "       fmc ban           - don't ever play current song again\n"
           "       fmc rate          - mark current song as \"liked\"\n"
           "       fmc unrate        - unmark current song\n"
           "       fmc channels      - list all FM channels\n"
           "       fmc website       - open the douban page using the browser defined in $BROWSER\n"
           "       fmc setch <id>    - set channel through channel's id\n"
           "       fmc kbps <kbps>   - set music quality to the specified kbps\n"
           "       fmc launch        - tell fmd to restart\n"
           "       fmc end           - tell fmd to quit\n"
          );
}

void time_str(int time, char *buf)
{
    int sec = time % 60;
    int min = time / 60;
    sprintf(buf, "%d:%02d", min, sec);
}

int main(int argc, char *argv[])
{
    read_channels();

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

    char input_buf[64] = "";
    char output_format[512] = "";
    char output_buf[1024] = "";
    int buf_size;

    if (optind < argc) {
        strcpy(input_buf, argv[optind]);
        int i;
        char *buf = strcmp(input_buf, "info") == 0 ? output_format : input_buf;
        for (i = optind + 1; i < argc; i++) {
            strcat(buf, " ");
            strcat(buf, argv[i]);
        }
    } else {
        strcpy(input_buf, "info");
    }

    if (strcmp(input_buf, "channels") == 0) {
        print_channels();
        return 0;
    }
    else if (strcmp(input_buf, "help") == 0) {
        print_usage();
        return 0;
    }
    else if (strcmp(input_buf, "launch") == 0) {
        // forcefully restart fmd
        system("killall fmd;"
               "sleep 1;"
               "fmd;");
        return 0;
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

    send(sock_fd, input_buf, strlen(input_buf), 0);
    buf_size = recv(sock_fd, output_buf, sizeof(output_buf), 0);
    if (buf_size == 0) {
        close(sock_fd);
        return 0;
    }
    output_buf[buf_size] = '\0';
    close(sock_fd);

    json_object *obj = json_tokener_parse(output_buf);
    char *status = obj ? strdup(json_object_get_string(json_object_object_get(obj, "status"))) : "error", 
         *channel = "", *artist = "", *title = "", pos[16] = "", len[16] = "", kbps[8] = "", *album = "", *cover = "", year[8] = "", *douban_url = "", *like = "";

    if (strcmp(status, "error") != 0) {
        int c_id = json_object_get_int(json_object_object_get(obj, "channel"));
        if (c_id == local_channel) {
            channel = get_local_channel_name();
        }
        else if (c_id < 0 || c_id >= channel_max || channels[c_id].id < 0) {
            channel = "未知兆赫";
        }
        else {
            channel = channels[c_id].name;
        }
        sprintf(kbps, "%d", json_object_get_int(json_object_object_get(obj, "kbps")));
        if (strcmp(status, "stop") != 0) {
            time_str(json_object_get_int(json_object_object_get(obj, "pos")), pos);
            time_str(json_object_get_int(json_object_object_get(obj, "len")), len);
            like = json_object_get_int(json_object_object_get(obj, "like")) ? "1" : "0";
            artist = strdup(json_object_get_string(json_object_object_get(obj, "artist")));
            title = strdup(json_object_get_string(json_object_object_get(obj, "title")));
            album = strdup(json_object_get_string(json_object_object_get(obj, "album")));
            sprintf(year, "%d", json_object_get_int(json_object_object_get(obj, "year")));
            cover = strdup(json_object_get_string(json_object_object_get(obj, "cover")));
            douban_url = strdup(json_object_get_string(json_object_object_get(obj, "url")));
        }
    }

    if (output_format[0] == '\0') {
        if (strcmp(status, "error") == 0) {
            printf("%s\n", obj ? json_object_get_string(json_object_object_get(obj, "message")) : "Unknown error");
        } else {
            printf("FMD %s - %s / %s kbps\n", strcmp(status, "play") == 0? "Playing": (strcmp(status, "pause") == 0? "Paused": "Stopped"), channel, kbps);

            if (strcmp(status, "stop") != 0) {
                printf("%s%s - %s\n%s / %s\n", 
                        like[0] == '1' ? "[Like] ": "", artist, title, 
                        pos, len);
            }
        }
    } else {
        char info[1024], *arg = "";
        int l = strlen(output_format) + 1;
        // we must print out the information the user wants
        // trim the space in front 
        int i = 1, pi = 0;
        while ( i < l) {
            char ch = output_format[i++];
            if (i < l - 1 && ch == '%') {
                char spec = output_format[i++];
                switch (spec) {
                    case 'a': arg = artist; break;
                    case 't': arg = title; break;
                    case 'b': arg = album; break;
                    case 'y': arg = year; break;
                    case 'i': arg = cover; break;
                    case 'd': arg = douban_url; break;
                    case 'c': arg = channel; break;
                    case 'p': arg = pos; break;
                    case 'l': arg = len; break;
                    case 'u': arg = status; break;
                    case 'k': arg = kbps; break;
                    case 'r': arg = like; break;
                    case '%': arg = "%%"; break;
                    default: 
                        printf("Unknown specifier %c. Try help\n", spec);
                        return 1;
                }
                // loop through the arg and copy the chars
                while (*arg != '\0') {
                    info[pi++] = *arg;
                    arg++;
                }
            } else 
                info[pi++] = ch;
        }
        printf(info);
    }
    json_object_put(obj);

    return 0;
}
