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

void getcreds(char *username, char *password) {
  memset(username, 0, MAXUSER);
  memset(password, 0, MAXPASS);
  printf("Username: ");
  gets(username, MAXUSER-1);
  printf("Password: ");
  gets(password, MAXPASS-1);
  cutnl(username);
  cutnl(password);
}

void login() {
	int fd;
  struct user *user = inituser(0, 0, 0);
	char userbuf[MAXUSERSSTR];

	fd = open("/users", O_RDONLY);
	
	if(fd < 0) {
		printf("No users. Setup root user.\n");
		getcreds(user->username, user->password);
    userlist = inituserlist();
    adduser(userlist, user);
		logusers(userlist);
    freeuserlist(userlist);
	}
  else {
    int uid;
    getcreds(user->username, user->password);
		if(read(fd, userbuf, sizeof(userbuf)) < 0) {
			printf("Couldn't get users!\n");
			exit(1);
		}
    close(fd);
		userlist = deserialize_users(userbuf);
    int i;
		for(i = 0; i < 3 && (uid = getuser(userlist, 0, user->username, user->password)) < 0; i++) {
      printf("Invalid username or password\n");
      getcreds(user->username, user->password);
    }
    if(uid < 0) {
      printf("Too many failed login attempts\n");
      exit(1);
    }
    user->id = uid;
    freeuserlist(userlist);
  }
  setuid(user->id);
  freeuser(user);
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
