#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "shelluser.h"

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage: adduser <username> <password>\n");
        exit(1);
    }

    if(strlen(argv[1]) > MAXUSER-1 || strlen(argv[2]) > MAXPASS-1) {
        printf("Max username: %d\nMax password: %d\n", MAXUSER, MAXPASS);
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

    struct user *newuser = inituser(0, argv[1], argv[2]);

    struct userlist *users = deserialize_users(userbuf);
    adduser(users, newuser);
    logusers(users);
    freeuserlist(users);

    return 0;
}