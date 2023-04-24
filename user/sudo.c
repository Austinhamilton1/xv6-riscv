#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"
#include "user/shelluser.h"

void runcommand(char *argv[]) {
	int pid;
	if((pid = fork()) < 0) {
		printf("Fork failed\n");
		exit(1);
	}

	if(pid == 0) {
		exec(argv[0], argv);
	}
	else {
		wait((int *)0);
	}
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Usage: sudo <command>\n");
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

	char password[MAXPASS];
	memset(password, 0, sizeof(password));
	printf("Root password: ");
	gets(password, sizeof(password)-1);
	cutnl(password);

	if((uid = getuser(users, 1, 0, hash(password))) != 1) {
		printf("Invalid admin password\n");
		exit(1);
	}

	setuid(uid);
	freeuserlist(users);

	char *args[MAXARG];
	int i = 0;
	for(i = 0; i < MAXARG - 1; i++) {
		args[i] = argv[i+1];
	}
	args[i] = 0;

	runcommand(args);
	
	return 0;
}
