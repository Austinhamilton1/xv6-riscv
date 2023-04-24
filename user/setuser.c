#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/shelluser.h"

int main(int argc, char*argv[]) {
        if(argc != 3) {
                printf("Usage: setuid <username> <password>\n");
                exit(1);
        }

        int fd, uid;
        if((fd = open("users", O_RDONLY)) < 0) {
                printf("No users file\n");
                exit(1);
        }

        char userstr[MAXUSERSSTR];
        if(read(fd, userstr, sizeof(userstr)) < 0) {
                printf("Read failed\n");
                exit(1);
        }

        struct userlist *users = deserialize_users(userstr);

        if((uid = getuser(users, 0, argv[1], hash(argv[2]))) < 0) {
                printf("Invalid user\n");
                exit(1);
        }

        setuid(uid);
        printf("User id: %d\n", getuid());

        return 0;
}
