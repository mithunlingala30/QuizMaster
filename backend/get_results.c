/* =========================================================
   get_results.c
   CGI program (GET) - reads data/scores.txt and returns results
   as JSON.
     ?username=student01  -> only that student's history (My Results)
     (no param)            -> every result (Admin > Results)
   ========================================================= */
#include "common.h"

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char username[50] = "";
    char *qs = getenv("QUERY_STRING");
    if (qs != NULL) get_form_value(qs, "username", username, sizeof(username));

    FILE *fp = fopen(SCORES_FILE, "r");
    if (fp == NULL) {
        printf("[]");
        return 0;
    }

    printf("[");
    char line[512];
    int first = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        char copy[512];
        strcpy(copy, line);
        trim_newline(copy);
        if (strlen(copy) == 0) continue;

        char *f[6]; /* username|quizName|date|total|correct|percentage */
        if (split_line(copy, f, 6) != 6) continue;

        if (strlen(username) > 0 && strcmp(f[0], username) != 0) continue;

        char quizEsc[100];
        json_escape(f[1], quizEsc, sizeof(quizEsc));

        if (!first) printf(",");
        first = 0;
        printf("{\"username\":\"%s\",\"quizName\":\"%s\",\"date\":\"%s\","
               "\"total\":%s,\"correct\":%s,\"percentage\":%s}",
               f[0], quizEsc, f[2], f[3], f[4], f[5]);
    }
    fclose(fp);
    printf("]");
    return 0;
}
