void mystrcp(char * from, int cp_num, char * to) {
    for(int i=0; i<cp_num ; i++) {
        to[i] = from[i];
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


