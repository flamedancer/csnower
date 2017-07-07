#include "utils.c"
#define buff_len 1024
#define MAXHEADERSLEN 30

char backup[buff_len];
int backup_seek=0;
int read_over=0;

struct header {
    char key[200];
    char value[1024]; 
};

struct request {
    char * method;
    char * url;
    char * version;
    struct header headers[MAXHEADERSLEN];
    char * body; 
};


struct header create_header() {
    struct header tmp = {
        "",
        "",
    }; 
    return tmp;
}

struct request create_request() {
    struct request tmp = {
        NULL,
        NULL,
        NULL,
        {},
        NULL,
    };
    for (int i=0; i < MAXHEADERSLEN; i++) {
        tmp.headers[i] = create_header();
    }
    return tmp;
}


void clear_request(struct request * req) {
    if ( NULL != req->method) {
        free(req->method);
        req->method = NULL;
    }
    if ( NULL != req->body) {
        free(req->body);
        req->body = NULL;
    }
}

void reset_backup() {
    backup[0] = '\0';
    backup_seek = 0;
}

char * read_line(int client, int max_read_len) {
    int backup_line_len=0;  // 备份行的长度包括 '\0'
    char * buff;
    int j;
    for(j=backup_seek; j < buff_len; j++) {
        // 先判断 backup
        if (backup[j] == '\0') {            
            j--;  // 为了好算backup_line_len
            break;
        }
        if (backup[j] == '\n') {            
            if (j !=backup_seek && backup[j-1] == '\r') {
                backup[j-1] = '\0';
                backup_line_len = j - backup_seek;
            }
            else {
                backup[j] = '\0';
                backup_line_len = j - backup_seek + 1;
            }
            // 取到空行
            if (backup_line_len == 0)
                break;
            buff = (char*)malloc(backup_line_len*sizeof(char));
            if (!buff) {
                printf("ERROR: malloc error!");
                read_over = 1;
                return NULL;
            }
            mystrcp(backup + backup_seek, backup_line_len, buff);
            // 判断 当前backup 已被利用完
            if (j == buff_len - 1 ) {
                reset_backup();
            }
            else {
                backup_seek = j + 1;
            }
            return buff;
        }
    }

    // 没有找到 \n

    backup_line_len = j - backup_seek + 1;
    if (read_over) {
        if (backup_line_len == 0) {
            return NULL;
        }
        else {
            buff = (char*)malloc(backup_line_len + 1);
            mystrcp(backup + backup_seek, backup_line_len, buff);
            buff[backup_line_len] = '\0';
            printf("debug: end missing '\\0'  %d %s\n", backup_line_len, buff);
            reset_backup();
            return buff;
        }
    }
    int read_len = 0;
    int all_read_len = 0;
    int read_cnt = 0;
    int buff_seek;
    buff = (char*)malloc(backup_line_len + buff_len*sizeof(char));
    mystrcp(backup + backup_seek, backup_line_len, buff);
    buff[backup_line_len] = '\0';
    while(1) {
        if (read_over) {
            return buff;
        }
        if (read_cnt != 0) {
            buff = (char*)realloc(buff, backup_line_len + (read_cnt+1)*buff_len*sizeof(char));
        } 
        buff_seek = backup_line_len + read_cnt*buff_len;
        read_len = read(client, buff+buff_seek, buff_len);
        all_read_len += read_len;
        if (read_len < buff_len || (max_read_len > 0 && all_read_len >= max_read_len) ) {
            printf(">>> read client over \n");
            read_over=1;
        }
        for(int k=0; k < read_len; k++) {
            if (buff[buff_seek + k] == '\n') {            
                if ( (read_len - 1) > k ) {
                    mystrcp(buff+buff_seek+(k+1), read_len - k - 1, backup);
                }
                // 设置 结束标记
                backup[read_len-k-1] = '\0';
                backup_seek=0;
                if ( (all_read_len + k) !=0 && buff[buff_seek+k-1] == '\r') {
                    buff[buff_seek+k-1] = '\0';
                }
                else {
                    buff[buff_seek+k] = '\0';
                }
                return buff;
            }
        }
        read_cnt++;
    }
    // backup 已经利用完
    if (j == buff_len - 1 || backup[j] == '\0' ) {
        reset_backup();
    }

}

void suck_method_url_version(struct request * req, char * line_p) {
    int split1=0, split2=0;   // 两个 空格分隔符 所在位置
    int index = 0;
    for (char *p = line_p; *p != '\0'; p++, index++) {
        if ( *p == ' ') {
            if (split1 == 0)
                split1 = index;
            else {
                split2 = index;
                break;
            }
        }
    }

    req->method = line_p; 
    *(line_p + split1) = '\0';
    req->url = line_p + split1 + 1; 
    *(line_p + split2) = '\0';
    req->version = line_p + split2 + 1;
}

void suck_headers(struct request * req, char * line_p) {
    int split=0;   // ":" 分隔符 所在位置
    int index = 0;
    int line_len = 0;
    for (char *p = line_p; *p != '\0'; p++, index++) {
        if ( *p == ':' && split == 0) {
            split = index;
        }
    }
    line_len = index;
    for (int i=0; i < MAXHEADERSLEN; i++) {
        if ( req->headers[i].key[0] == '\0') {
            mystrcp(line_p, split, req->headers[i].key); 
            req->headers[i].key[split + 1] = '\0';
            mystrcp(line_p + split + 2, line_len - split, req->headers[i].value); 
            req->headers[i].value[line_len - split] = '\0';
            break;
        }
    }
}

void suck_body(struct request * req, char * line_p) {
    req->body = line_p;
}


int get_content_length(struct request * req) {
    for (int i=0; i < MAXHEADERSLEN; i++) {
        if ( req->headers[i].key[0] == '\0') {
            return 0;
        }
        else if (mystrcmp(req->headers[i].key, "Content-Length") == 0)  {
            return myatoi(req->headers[i].value);
        }
    }
    return 0;
}


void print_readlines(int client) {
    reset_backup();
    read_over=0;
    int step = 0;  //  0 method_url_version  1 header 2 post_data
    char * line;
    int keep_line = 0;
    char * none_line = "";
    int content_length = -1;
    struct request this_request = create_request();
    while(1) { 
        keep_line = 0;
        line = read_line(client, content_length);
        printf("|||: %s\n", line);
        if (NULL == line) {
            break;
        }
        if (mystrcmp(line, none_line) == 0) {
            content_length = get_content_length(&this_request);
            // 数据结束
            if (content_length == 0) {
                read_over = 1;
            }
            else
                step++;
        }
        else {
            if (step == 0) {
                suck_method_url_version(&this_request, line);
                keep_line = 1;
                step++;
            }
            else if (step == 1) {
                suck_headers(&this_request, line);
            }
            else if (step == 2) {
                suck_body(&this_request, line);
                keep_line = 1;
            }

            // todo  如果 content_lenght 有值则继续 否则read_over
        }
        if (!keep_line)  {
            free(line);
            line = NULL;
        }
    }
    printf(">>> request method is %s\n", this_request.method);
    printf(">>> request url is %s\n", this_request.url);
    printf(">>> request version is %s\n", this_request.version);
    for (int i=0; i < MAXHEADERSLEN; i++) {
        if (this_request.headers[i].key[0] != '\0') {
            printf(">>> request header key `%s`  value '%s' \n", this_request.headers[i].key, this_request.headers[i].value); 
        }
        
    }
    printf(">>> request body is %s\n", this_request.body);
    clear_request(&this_request);
}