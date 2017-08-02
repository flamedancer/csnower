#define HASHTABLELEN 1000   // 最大 handler数


struct hash_item {
    struct hash_item * next_item; // hash冲突 采用 链表法
    char * key;
    char * (*handler_fuc)(char * response_s, int max_respone_size, struct request * this_request); // 处理函数
     
};

struct hash_item *hash_table[HASHTABLELEN];


int hash_fuc(char * str) {
    int hash_value = 0;
    while( *str ) {
        hash_value = hash_value + *str;
        str++;
    }
    return hash_value;
}

void init_hash_table() {
    for(int i=0; i < HASHTABLELEN; i++) {
        hash_table[i] = NULL;
    }
}

struct hash_item * create_hash_item(char *key, char *(*handler_fuc)(char* , int, struct request *)) {
    struct hash_item * item = malloc(sizeof(struct hash_item));
    item->next_item = NULL;
    item->key = key;
    item->handler_fuc = handler_fuc;
    return item;
}


struct hash_item * find_hash_item(char *key) {
    int hash_index = hash_fuc(key) / HASHTABLELEN;
    struct hash_item * this_hash_item = hash_table[hash_index];
    if (NULL == this_hash_item) {
        return NULL;
    }
    else {
        // 若有则返回
        do {
            if (mystrcmp(key, this_hash_item->key) == 0) {
                return this_hash_item;
            }
        }
        while( (this_hash_item = this_hash_item->next_item) );
        return NULL;
    }
}

void update_hash_item(char * key, char * (*handler_fuc)(char *, int, struct request *) ) {
    int hash_index = hash_fuc(key) / HASHTABLELEN;
    struct hash_item * this_hash_item = hash_table[hash_index];
    struct hash_item * item = create_hash_item(key, handler_fuc);
    if (NULL == this_hash_item) {
        hash_table[hash_index] = item;
    }
    else {
        // 若有则更新 handler_fuc   没有则插入
        do {
            if (mystrcmp(key, this_hash_item->key) == 0) {
                this_hash_item->handler_fuc = handler_fuc;
                return;
            }
        }
        while( (this_hash_item = this_hash_item->next_item) );
        item->next_item = hash_table[hash_index];
        hash_table[hash_index] = item;
    }
}

char * default_handler(char * response_str, int max_response_len, struct request * this_request) {
    char * old_res = response_str;
    char * status = "HTTP/1.1 200 OK\n";
    char * header = "Content-Type: text/html; charset=utf-8\n\n";
    int remain_len = max_response_len;
    int response_len = min( remain_len-1, mystrlen(status) );
    mystrcp(status, response_len, response_str);
    response_str += response_len;
    remain_len -= response_len;

    response_len = min( remain_len-1, mystrlen(header) );
    mystrcp(header, response_len, response_str);
    response_str += response_len;
    remain_len -= response_len;

    char * str1 = "your version is:";
    response_len = min( remain_len-1, mystrlen(str1) );
    mystrcp(str1, response_len, response_str);
    response_str += response_len;
    remain_len -= response_len;

    response_len = min( remain_len-1, mystrlen(this_request->version) );
    mystrcp(this_request->version, response_len, response_str);
    response_str += response_len;
    remain_len -= response_len;
    *response_str = '\0'; 
    return old_res;


}


char * handler0(char * response_str, int max_response_len, struct request * this_request) {
    char * old_res = response_str;
    char * response = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\nHello EverOne\r\n";
    int response_len = min( response_len-1, mystrlen(response) );
    mystrcp(response, response_len, response_str);
    response_str[response_len] = '\0'; 
    return old_res;
    
}


char * handler1(char * response_str, int max_response_len, struct request * this_request) {
    char * old_res = response_str;
    char * response = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\nthis is the reponse of handler1\r\n";
    int response_len = min( response_len-1, mystrlen(response) );
    mystrcp(response, response_len, response_str);
    response_str[response_len] = '\0'; 
    return old_res;
}


void install_handlers() {
    update_hash_item("", default_handler);
    update_hash_item("hello", handler0);
    update_hash_item("admin", handler1);
    

}


