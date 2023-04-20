// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "shelluser.h"

char *argv[] = { "sh", 0 };

struct user *user;

void loguser(struct user *user, int method) {
	int fd;

  if(method == 0) {
    if((fd = open("users", O_CREATE | O_WRONLY)) < 0) {
      fprintf(2, "Couldn't create user file.\n");
      exit(1);
    }

    char *userstr = serialize(user);
    fprintf(fd, "%s\n", userstr);
    free(userstr);

    close(fd);
  }
}

void cutnl(char *str) {
  while(*str != '\n')
    str++;
  *str = 0;
}

void getuser(struct user *user) {
	int fd;

	fd = open("users", O_RDONLY);
	
	if(fd < 0) {
		printf("No users. Setup root user.\n");
		printf("Username: ");
		gets(user->username, MAXUSER);
		printf("Password: ");
		gets(user->password, MAXPASS);
		user->id = 1234;
    cutnl(user->username);
    cutnl(user->password);
		loguser(user, 0);
	}
  else {
    printf("Username: ");
    gets(user->username, MAXUSER);
    printf("Password: ");
    gets(user->password, MAXPASS);
  }

}

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  user = (struct user *)malloc(sizeof(struct user));
	getuser(user);

  for(;;){
    printf("init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec("sh", argv);
      printf("init: exec sh failed\n");
      exit(1);
    }

    for(;;){
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *) 0);
      if(wpid == pid){
        // the shell exited; restart it.
        break;
      } else if(wpid < 0){
        printf("init: wait returned an error\n");
        exit(1);
      } else {
        // it was a parentless process; do nothing.
      }
    }
  }
}
