void mystrcp(char * from, int cp_num, char * to) {
/* cp_num 代表要复制的字符数 若为0表示全部复制(遇到\0才停止) */
    if (cp_num != 0) {
        for(int i=0; i<cp_num ; i++) {
            to[i] = from[i];
        }
    }
    else {
        for(;*from; from++, to++) {
            *to = *from;
        }
    }
}

int mystrcmp(char * str1, char * str2) {
    for(char *i=str1, *j=str2; *i || *j; i++, j++) {
        if (*i > *j)
            return 1;
        else if (*i < *j)
            return -1;
    }
    return 0;
}

int myatoi(char * str) {
    int value = 0;
    for(char *p=str; *p != '\0'; p++) {
        value = value * 10 + (*p - '0');
    }
    return value;
}

int mystrlen(char *str) {
    int len=0;
    for(;*str;len++, str++);
    return len;
}

int mystartswith(char * str, char * prefix) {
    while( *prefix &&  *str == *prefix ) {
        str++;
        prefix++;
    }
    if( *prefix )
        return 0;
    return 1;
}


int mysplit(char * str, char * sep, char **split_rst, int rst_len) {
/*
 maxsplit 最大分割次数 
 返回分割成功结果个数(即split_rst的最多index) 没有分割过返回 1
 例如： mysplit("hello world sdf", " ", [], 2) 
   返回 2 且 slit_rst =  ["hello", "world sdf"]
 注：此函数会改变原str字符串  
 */
    char *p = str;
    int sep_len = mystrlen(sep);
    int nowsplit = 0;
    int maxsplit = rst_len - 1; // 最多可以有 maxsplit + 1个结果
    char * pre_s = str;
    while(*p) {
        if (mystartswith(p, sep)) {
            split_rst[nowsplit] = pre_s;
            *p = '\0';
            p = p + sep_len;
            pre_s = p;
            if (++nowsplit >= maxsplit)
                break;
        }
        else
            p++;
    }
    split_rst[nowsplit++] = pre_s;
    return nowsplit; 

}


