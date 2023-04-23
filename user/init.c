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

struct userlist *userlist;

void logusers(struct userlist *users, int method) {
	int fd;

  if(method == 0) {
    if((fd = open("users", O_CREATE | O_WRONLY)) < 0) {
      fprintf(2, "Couldn't create user file.\n");
      exit(1);
    }

    char *userstr = serialize_users(users);
    fprintf(fd, "%s", userstr);
    free(userstr);

    close(fd);
  }
}

void cutnl(char *str) {
  while(*str != '\n')
    str++;
  *str = 0;
}

void login() {
	int fd;
  struct user *user = (struct user *)malloc(sizeof(struct user));
	char userbuf[MAXUSERSSTR];
  memset(user->username, 0, MAXUSER);
  memset(user->password, 0, MAXPASS);

	fd = open("users", O_RDONLY);
	
	if(fd < 0) {
		printf("No users. Setup root user.\n");
		printf("Username: ");
		gets(user->username, MAXUSER-1);
		printf("Password: ");
		gets(user->password, MAXPASS-1);
		user->id = 1;
    cutnl(user->username);
    cutnl(user->password);
    userlist = (struct userlist *)malloc(sizeof(struct userlist));
    adduser(userlist, user);
		logusers(userlist, 0);
	}
  else {
    int uid;
    printf("Username: ");
    gets(user->username, MAXUSER-1);
    printf("Password: ");
    gets(user->password, MAXPASS-1);
    cutnl(user->username);
    cutnl(user->password);
		if(read(fd, userbuf, sizeof(userbuf)) < 0) {
			printf("Couldn't get users!\n");
			exit(1);
		}
		userlist = deserialize_users(userbuf);
		if((uid = getuser(userlist, user->username, user->password)) < 0) {
      printf("Invalid username or password.\n");
      login(user);
    }
    user->id = uid;
  }
  setuid(user->id);
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
  
  login();

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
