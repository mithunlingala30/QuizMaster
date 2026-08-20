/* =========================================================
   get_students.c
   CGI program (GET) - admin "Students" page data source.
   Joins users.txt (identity) with scores.txt (performance)
   entirely through file reads - no database involved.
   ========================================================= */
#include "common.h"

#define MAX_S 500

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char names[MAX_S][50];
    int attempts[MAX_S];
    float totalPct[MAX_S];
    int sCount = 0;

    /* ---- Pass 1: read users.txt for identity fields ---- */
    struct { char id[20]; char name[100]; char email[100]; char username[50]; } users[MAX_S];
    int uCount = 0;

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL && uCount < MAX_S) {
            char copy[1024];
            strcpy(copy, line);
            trim_newline(copy);
            if (strlen(copy) == 0) continue;
            char *f[5];
            if (split_line(copy, f, 5) != 5) continue;
            strncpy(users[uCount].id, f[0], sizeof(users[uCount].id) - 1);
            strncpy(users[uCount].name, f[1], sizeof(users[uCount].name) - 1);
            strncpy(users[uCount].email, f[2], sizeof(users[uCount].email) - 1);
            strncpy(users[uCount].username, f[3], sizeof(users[uCount].username) - 1);
            uCount++;
        }
        fclose(fp);
    }

    /* ---- Pass 2: read scores.txt and aggregate per username ---- */
    fp = fopen(SCORES_FILE, "r");
    if (fp != NULL) {
        char line[512];
        while (fgets(line, sizeof(line), fp) != NULL) {
            char copy[512];
            strcpy(copy, line);
            trim_newline(copy);
            if (strlen(copy) == 0) continue;
            char *f[6];
            if (split_line(copy, f, 6) != 6) continue;

            int idx = -1;
            for (int i = 0; i < sCount; i++) if (strcmp(names[i], f[0]) == 0) { idx = i; break; }
            if (idx == -1 && sCount < MAX_S) {
                idx = sCount++;
                strncpy(names[idx], f[0], sizeof(names[idx]) - 1);
                attempts[idx] = 0;
                totalPct[idx] = 0;
            }
            if (idx != -1) {
                attempts[idx]++;
                totalPct[idx] += atof(f[5]);
            }
        }
        fclose(fp);
    }

    /* ---- Emit joined JSON ---- */
    printf("[");
    for (int i = 0; i < uCount; i++) {
        int a = 0; float avg = 0;
        for (int j = 0; j < sCount; j++) {
            if (strcmp(names[j], users[i].username) == 0) {
                a = attempts[j];
                avg = (a > 0) ? (totalPct[j] / a) : 0;
                break;
            }
        }
        char nameEsc[100], emailEsc[100];
        json_escape(users[i].name, nameEsc, sizeof(nameEsc));
        json_escape(users[i].email, emailEsc, sizeof(emailEsc));

        printf("%s{\"studentId\":\"%s\",\"name\":\"%s\",\"email\":\"%s\",\"username\":\"%s\","
               "\"attempts\":%d,\"average\":%.2f}",
               i == 0 ? "" : ",", users[i].id, nameEsc, emailEsc, users[i].username, a, avg);
    }
    printf("]");
    return 0;
}
