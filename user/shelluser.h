#include "user/user.h"

#define MAXUSER     20
#define MAXPASS     20
#define MAXIDSTR    10
#define MAXUSERS    10

struct user {
    unsigned int id;
    char username[MAXUSER];
    char password[MAXPASS];
    struct user *next;
};

//maximum length of a single user string in the users file 
#define MAXUSERSTR  64

struct userlist {
    struct user *admin;
};

//maximum amount of data in the users file
#define MAXUSERSSTR 1024

void cutnl(char *str) {
  while(*str != '\n')
    str++;
  *str = 0;
}

struct user *inituser(int id, char *username, char *password) {
    struct user *user = (struct user *)malloc(sizeof(struct user));
    user->id = id;
    memset(user->username, 0, sizeof(user->username));
    memset(user->password, 0, sizeof(user->password));
    memcpy(user->username, username, sizeof(user->username)-1);
    memcpy(user->password, password, sizeof(user->password)-1);
    return user;
}

void freeuser(struct user *user) {
    if(user != 0)
        free(user);
}

char *serialize_user(struct user *user) {
    char *str = malloc(MAXUSERSTR);
    memset(str, 0, MAXUSERSTR);
    char *ptr = str;

    char *idstr = (char *)malloc(10);
    itoa(idstr, user->id);

    memcpy(ptr, idstr, sizeof(idstr));
    ptr += strlen(idstr);

    *ptr = ':';
    ptr++;

    memcpy(ptr, user->username, sizeof(user->username));
    ptr += strlen(user->username);

    *ptr = ':';
    ptr++;

    memcpy(ptr, user->password, sizeof(user->password));
    ptr += strlen(user->password);

    return str;
}

struct user *deserialize_user(char *str) {
    struct user *user = (struct user *)malloc(sizeof(struct user));
    
    while(*str != ':') {
        user->id *= 10;
        user->id += (unsigned int)(*str - 48);
        str++;
    }
    str++;

    char *name = user->username;
    while(*str != ':')
        *name++ = *str++;
    
    *(++name) = 0;
    str++;

    char *pass = user->password;
    while(*str != 0 && *str != '\n')
        *pass++ = *str++;
    *(++pass) = 0;

    return user;
}

struct userlist *inituserlist() {
    struct userlist *users = (struct userlist *)malloc(sizeof(struct userlist));
    return users;
}

void freeuserlist(struct userlist *users) {
    if(users == 0)
        return;
    struct user *current = users->admin;
    struct user *next;
    while(current != 0) {
        next = current->next;
        free(current);
        current = next;
    }
    free(users);
}

int length(struct userlist *users) {
    int counter = 0;
    if(users == 0 || users->admin == 0)
        return 0;
    for(struct user *ptr = users->admin; ptr != 0; ptr = ptr->next)
        counter += 1;
    return counter;
}

int exists(struct userlist *users, char *username) {
    if(users == 0 || users->admin == 0)
        return 0;
    struct user *ptr;
    for(ptr = users->admin; ptr != 0; ptr = ptr->next) {
        if(strcmp(ptr->username, username) == 0)
            return 1;
    }
    return 0;
}

void adduser(struct userlist *users, struct user *user) {
    if(users == 0) {
        printf("Error: invalid userlist\n");
        exit(1);
    }
    if(length(users) == MAXUSERS) {
        printf("Max users reached\n");
        return;
    }
    if(users->admin == 0) {
        user->id = 1;
        users->admin = user;
        return;
    }
    if(exists(users, user->username)) {
        printf("Username %s already exists\n", user->username);
        return;
    }
    struct user *current = users->admin;
    while(current->next != 0) {
        current = current->next;
    }
    user->id = current->id + 1;
    current->next = user;
}

void rmuser(struct userlist *users, char *username) {
    if(users == 0) {
        printf("Error: invalid userlist\n");
        exit(1);
    }
    if(users->admin == 0) {
        printf("Error: no admin\n");
        exit(1);
    }
    if(!exists(users, username)) {
        printf("No user %s\n", username);
        return;
    }
    struct user *current = users->admin, *prev = 0;
    while(current != 0 && strcmp(current->username, username) != 0) {
        prev = current;
        current = current->next;
    }
    prev->next = current->next;
}

int getuser(struct userlist *users, int id, char *username, char *password) {
    if(users == 0)
        return -1; 
    struct user *current = users->admin;
    while(current != 0) {
        if((current->id == id || strcmp(current->username, username) == 0) 
        && strcmp(current->password, password) == 0) {
            return current->id;
        }
        current = current->next;
    }
    return -1;
}

char *serialize_users(struct userlist *userlist) {
    char *str = (char *)malloc(MAXUSERSSTR);
    memset(str, 0, MAXUSERSSTR);
    char *ptr = str;
    struct user *user = userlist->admin;
    while(user != 0) {
        char *userstr = serialize_user(user);
        int length = strlen(userstr);
        memcpy(ptr, userstr, length);
        free(userstr);
        ptr += length;
        *(ptr++) = '\n';
        user = user->next;
    }
    return str;
}

int get_userstr(char *dst, char *str) {
    while(*str != 0 && *str != '\n') {
        *(dst++) = *(str++);
    }
    if(*(str-1) == 0)
        return 0;
    if(*(str++) == '\n')
        return 1;
    return 0;
}

struct userlist *deserialize_users(char *str) {
    struct userlist *users = (struct userlist *)malloc(sizeof(struct userlist));
    char userbuf[MAXUSERSTR];
    memset(userbuf, 0, sizeof(userbuf));
    while(get_userstr(userbuf, str) > 0) {
        str += strlen(userbuf) + 1;
        adduser(users, deserialize_user(userbuf));
        memset(userbuf, 0, sizeof(userbuf));
    }
    return users;
}

void logusers(struct userlist *users) {
    int fd;
    if((fd = open("/users", O_CREATE | O_WRONLY)) < 0) {
        printf("Couldn't create user file.\n");
        exit(1);
    }
    if(write(fd, serialize_users(users), MAXUSERSSTR) < 0) {
        printf("Couldn't write to user file\n");
        exit(1);
    }

    close(fd);
}

void test1() {
    char *usersstr = "1:root:toor\n2:user:password\n3:jack:12345\n";
    struct userlist *userlist = deserialize_users(usersstr);
    printf("Deserialization completed\n");
    printf("User List: %s\n", serialize_users(userlist));
    freeuserlist(userlist);
    printf("Freed userlist\n");
}

void test2() {
    char *usersstr = "1:a:a\n2:b:b\n3:c:c\n4:d:d\n5:e:e\n6:f:f\n7:g:g\n8:h:h\n9:i:i\n10:j:j\n11:k:k";
    struct userlist *userlist = deserialize_users(usersstr);
    printf("Deserialization completed\n");
    printf("Length: %d\n", length(userlist));
    printf("User List: %s\n", serialize_users(userlist));
    freeuserlist(userlist);
    printf("Freed userlist\n");
}
