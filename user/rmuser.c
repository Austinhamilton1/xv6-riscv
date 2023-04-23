#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/shelluser.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: rmuser <username>\n");
        exit(1);
    }

    if(getuid() != 1) {
        printf("Permision denied: must be admin\n");
        exit(1);
    }

    int fd;
    char userbuf[MAXUSERSSTR];
    memset(userbuf, 0, sizeof(userbuf));
    if((fd = open("users", O_RDONLY)) < 0) {
        printf("Error: could not open users file\n");
        exit(1);
    }

    if(read(fd, userbuf, sizeof(userbuf)) < 0) {
        printf("Error: could not read users file\n");
        exit(1);
    }

    struct userlist *users = deserialize_users(userbuf);
    
    rmuser(users, argv[1]);
    logusers(users);
    freeuserlist(users);

    return 0;
}