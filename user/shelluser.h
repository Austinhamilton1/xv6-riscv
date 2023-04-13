#define MAXUSER 20
#define MAXPASS 20

struct user {
    int id;
    char username[MAXUSER];
    char password[MAXPASS];
};