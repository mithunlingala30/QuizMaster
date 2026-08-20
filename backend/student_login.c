/* =========================================================
   student_login.c
   CGI program (POST) that authenticates a student against
   data/users.txt using fopen("r") + fgets().
   ========================================================= */
#include "common.h"

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char *body = read_post_body();
    char username[50], password[50];
    get_form_value(body, "username", username, sizeof(username));
    get_form_value(body, "password", password, sizeof(password));
    free(body);

    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("{\"success\":false,\"message\":\"Username and password are required.\"}");
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL) {
        printf("{\"success\":false,\"message\":\"User data file not found.\"}");
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        char copy[1024];
        strcpy(copy, line);
        trim_newline(copy);
        char *fields[5]; /* studentId|name|email|username|password */
        if (split_line(copy, fields, 5) == 5) {
            if (strcmp(fields[3], username) == 0) {
                fclose(fp);
                if (strcmp(fields[4], password) == 0) {
                    char nameEsc[100];
                    json_escape(fields[1], nameEsc, sizeof(nameEsc));
                    printf("{\"success\":true,\"studentId\":\"%s\",\"name\":\"%s\",\"username\":\"%s\"}",
                           fields[0], nameEsc, fields[3]);
                } else {
                    printf("{\"success\":false,\"message\":\"Incorrect password.\"}");
                }
                return 0;
            }
        }
    }
    fclose(fp);

    printf("{\"success\":false,\"message\":\"No account found with that username.\"}");
    return 0;
}
