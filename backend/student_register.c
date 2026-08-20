/* =========================================================
   student_register.c
   CGI program (POST) that registers a new student.

   File handling demonstrated:
     fopen(..., "r")  -> scan existing users.txt for duplicate username
     fopen(..., "a")  -> append the new student record
     fgets/fprintf/fclose
   ========================================================= */
#include "common.h"

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char *body = read_post_body();

    char studentId[20], name[100], email[100], username[50], password[50], confirm[50];
    get_form_value(body, "studentId", studentId, sizeof(studentId));
    get_form_value(body, "name",      name,      sizeof(name));
    get_form_value(body, "email",     email,     sizeof(email));
    get_form_value(body, "username",  username,  sizeof(username));
    get_form_value(body, "password",  password,  sizeof(password));
    get_form_value(body, "confirmPassword", confirm, sizeof(confirm));
    free(body);

    /* ---- Server-side validation (never trust the browser) ---- */
    if (strlen(studentId) == 0 || strlen(name) == 0 || strlen(email) == 0 ||
        strlen(username) == 0 || strlen(password) == 0) {
        printf("{\"success\":false,\"message\":\"All fields are required.\"}");
        return 0;
    }
    if (!strchr(email, '@') || !strchr(email, '.')) {
        printf("{\"success\":false,\"message\":\"Please enter a valid email address.\"}");
        return 0;
    }
    if (strcmp(password, confirm) != 0) {
        printf("{\"success\":false,\"message\":\"Passwords do not match.\"}");
        return 0;
    }

    /* ---- Check username / student ID uniqueness by reading the file ---- */
    FILE *fp = fopen(USERS_FILE, "r");
    if (fp != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL) {
            char copy[1024];
            strcpy(copy, line);
            trim_newline(copy);
            char *fields[5];
            if (split_line(copy, fields, 5) == 5) {
                if (strcmp(fields[0], studentId) == 0) {
                    fclose(fp);
                    printf("{\"success\":false,\"message\":\"Student ID already registered.\"}");
                    return 0;
                }
                if (strcmp(fields[3], username) == 0) {
                    fclose(fp);
                    printf("{\"success\":false,\"message\":\"Username already taken.\"}");
                    return 0;
                }
            }
        }
        fclose(fp);
    }

    /* ---- Append the new record: studentId|name|email|username|password ---- */
    fp = fopen(USERS_FILE, "a");
    if (fp == NULL) {
        printf("{\"success\":false,\"message\":\"Could not open user data file.\"}");
        return 0;
    }
    fprintf(fp, "%s|%s|%s|%s|%s\n", studentId, name, email, username, password);
    fclose(fp);

    printf("{\"success\":true,\"message\":\"Registration successful. You can now log in.\"}");
    return 0;
}
