
#define buff_len 102

char backup[buff_len];
int backup_seek=0;
int read_over=0;

struct header {
    char key[200];
    char value[1024]; 
};

struct request {
    char method[10];
    char * url;
    char version[10];
    struct header headers[30];
    char * body; 
};

void * reset_backup() {
    backup[0] = '\0';
    backup_seek = 0;
}

void * mystrcp(char * from, int cp_num, char * to) {
    for(int i=0; i<cp_num ; i++) {
        to[i] = from[i];
    }
}

char * read_line(int client) {
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
            printf("debug: backup_line_len  %d \n", backup_line_len);
            buff = (char*)malloc(backup_line_len*sizeof(char));
            if (!buff) {
                printf("ERROR: malloc error!");
                read_over = 1;
                return NULL;
            }
            printf("debug buff addr %d \n", buff);                
            mystrcp(backup + backup_seek, backup_line_len, buff);
            // 判断 当前backup 已被利用完
            if (j == buff_len - 1 ) {
                reset_backup();
            }
            else {
                backup_seek = j + 1;
            }
            printf("debug: read for backup :: %s \n", buff);
            printf("debug buff addr %d \n", buff);                
            return buff;
        }
    }
    // 没有找到 \n
    backup_line_len = j - backup_seek + 1;
    if (backup_line_len == 0 && read_over) {
        printf(" read line over ");
        return NULL;
    }
    else if (backup_line_len > 0 && read_over) {
        buff = (char*)malloc(backup_line_len + 1);
        mystrcp(backup + backup_seek, backup_line_len, buff);
        buff[backup_line_len] = '\0';
        printf("debug: end missing '\\0'  %d %s\n", backup_line_len, buff);
        reset_backup();
        return buff;
    }
    int read_len=0;
    int read_cnt = 0;
    int buff_seek;
    buff = (char*)malloc(backup_line_len + buff_len*sizeof(char));
    mystrcp(backup + backup_seek, backup_line_len, buff);
    buff[backup_line_len] = '\0';
    printf("debug: backup_line_len for head-adding  %d %s \n", backup_line_len, buff);

    while(1) {
        if (read_over) {
            return buff;
        }
        if (read_cnt != 0) {
            buff = (char*)realloc(buff, backup_line_len + (read_cnt+1)*buff_len*sizeof(char));
        } 
        buff_seek = backup_line_len + read_cnt*buff_len;
        read_len = read(client, buff+buff_seek, buff_len);
        if (read_len < buff_len) {
            printf(" read client over \n");
            read_over=1;
        }
        for(int j=0; j< buff_len; j++) {
            if (buff[buff_seek + j] == '\n') {            
                mystrcp(buff+buff_seek+(j+1), buff_len - j - 1, backup);
                // 设置 结束标记
                backup[buff_len-j-1] = '\0';
                backup_seek=0;
                if (j !=0 && buff[buff_seek+j-1] == '\r') {
                    buff[buff_seek+j-1] = '\0';
                }
                else {
                    buff[buff_seek+j] = '\0';
                }
                return buff;
                break;
            }
        }
        read_cnt++;
    }
    // backup 已经利用完
    if (j == buff_len - 1 || backup[j] == '\0' ) {
        reset_backup();
    }

}


void print_readlines(int client) {
    reset_backup();
    read_over=0;
    char * line;
    while(1) { 
        line = read_line(client);
        if (!line)
            break;
        printf("%s\n", line);
        printf("debug: readline ok ! \n");
        printf("debug line addr %d \n", line);
        free(line);
        line = NULL;
        printf("debug: free ok ! \n");
    }
}
