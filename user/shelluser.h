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

//maximum length of a single user string in the users file (+3 = 2 semicolons and null)
#define MAXUSERSTR  MAXIDSTR + MAXUSER + MAXPASS + 3

struct userlist {
    struct user *admin;
};

//maximum amount of data in the users file (+MAXUSERS = \n for each user, +1 = null)
#define MAXUSERSSTR MAXUSERS * MAXUSERSTR + MAXUSERS + 1

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

void adduser(struct userlist *users, struct user *user) {
    if(users == 0) {
        printf("Error: invalid userlist\n");
        exit(1);
    }
    if(users->admin == 0) {
        users->admin = user;
        return;
    }
    struct user *current = users->admin;
    while(current->next != 0) {
        current = current->next;
    }
    current->next = user;
}

int getuser(struct userlist *users, char *username, char *password) {
    if(users == 0)
        return -1; 
    struct user *current = users->admin;
    while(current != 0) {
        if(strcmp(current->username, username) == 0 
        && strcmp(current->password, password) == 0) {
            return current->id;
        }
        current = current->next;
    }
    return -1;
}

char *serialize_users(struct userlist *userlist) {
    char *str = (char *)malloc(MAXUSERSSTR);
    memset(str, 0, MAXUSERSTR);
    char *ptr = str;
    int i = 0;
    struct user *user = userlist->admin;
    while(user != 0 && i++ < MAXUSERS) {
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

void freelist(struct userlist *userlist) {
    struct user *current = userlist->admin;
    struct user *next;
    while(current->next != 0) {
        next = current->next;
        free(current);
        current = next;
    }
    free(userlist);
}

void test1() {
    char *usersstr = "1:root:toor\n2:user:password\n3:jack:12345\n";
    struct userlist *userlist = deserialize_users(usersstr);
    printf("Deserialization completed\n");
    printf("User List: %s\n", serialize_users(userlist));
    freelist(userlist);
    printf("Freed userlist\n");
}
