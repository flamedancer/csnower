void setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


const int kReadEvent = 1;
const int kWriteEvent = 2;
const int kEmptyEvent = 0;

#define exit_if(r, ...) if(r) {printf(__VA_ARGS__); printf("error no: %d error msg %s\n", errno, strerror(errno)); exit(1);}


void updateEvents(int efd, int fd, int events, int modify, void * udata) {
    struct kevent ev[2];
    int n = 0;
    if (events & kReadEvent) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, udata);
    } else if (modify){
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, udata);
    }
    if (events & kWriteEvent) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, udata);
    } else if (modify){
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);
    }
    printf("%s fd %d events read %d write %d\n",
           modify ? "mod" : "add", fd, events & kReadEvent, events & kWriteEvent);
    int r = kevent(efd, ev, n, NULL, 0, NULL);
    exit_if(r, "kevent failed ");

}
