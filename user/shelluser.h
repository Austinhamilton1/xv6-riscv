#include "user/user.h"

#define MAXUSER     20
#define MAXPASS     20
#define MAXIDSTR    10
#define MAXSTR      64

struct user {
    unsigned int id;
    char username[MAXUSER];
    char password[MAXPASS];
};

char *serialize(struct user *user) {
    char *str = malloc(MAXIDSTR + 1 + sizeof(user->username) + 1 + sizeof(user->password));
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

    *ptr = 0;

    return str;
}

struct user *deserialize(char *str) {
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
    while(*str != 0)
        *pass++ = *str++;
    *(++pass) = 0;

    return user;
}