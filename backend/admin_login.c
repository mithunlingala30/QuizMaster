/* =========================================================
   admin_login.c
   CGI program (POST) that authenticates the administrator
   against data/admin.txt. Credentials never appear in any
   frontend file, JS, or HTML - only in this server-side file.
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

    FILE *fp = fopen(ADMIN_FILE, "r");
    if (fp == NULL) {
        printf("{\"success\":false,\"message\":\"Admin data file not found.\"}");
        return 0;
    }

    /* admin.txt has a simple fixed two-field format ("username|password"),
       so it is read directly with fscanf() instead of fgets()+split_line()
       - a clean, textbook demonstration of formatted file reading. */
    char fUser[50], fPass[50];
    int matched = 0;
    while (fscanf(fp, "%49[^|]|%49[^\n]\n", fUser, fPass) == 2) {
        if (strcmp(fUser, username) == 0 && strcmp(fPass, password) == 0) {
            matched = 1;
            break;
        }
    }
    fclose(fp);

    if (matched) {
        printf("{\"success\":true,\"message\":\"Welcome, admin.\"}");
    } else {
        printf("{\"success\":false,\"message\":\"Invalid admin credentials.\"}");
    }
    return 0;
}
