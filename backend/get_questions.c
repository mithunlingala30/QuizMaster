/* =========================================================
   get_questions.c
   CGI program (GET) - lists questions as a JSON array.

   ?admin=1  -> includes correctAnswer/category/difficulty/marks
               (used by manage-questions.html / edit-question.html)
   (no param)-> options only, correct answer withheld
               (used by quiz.html, so the browser never receives
                the answer key - scores are always checked
                server-side in submit_quiz.c)
   ========================================================= */
#include "common.h"

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char adminFlag[5] = "";
    char *qs = getenv("QUERY_STRING");
    if (qs != NULL) get_form_value(qs, "admin", adminFlag, sizeof(adminFlag));
    int isAdmin = (strcmp(adminFlag, "1") == 0);

    FILE *fp = fopen(QUESTIONS_FILE, "r");
    if (fp == NULL) {
        printf("[]");
        return 0;
    }

    /* fseek()/rewind() demo: jump to the end to check the file isn't
       empty before scanning it, then rewind back to the start to read
       every record from the beginning. */
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);
    if (fileSize == 0) {
        printf("[]");
        fclose(fp);
        return 0;
    }

    printf("[");
    char line[1200];
    int first = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        char copy[1200];
        strcpy(copy, line);
        trim_newline(copy);
        if (strlen(copy) == 0) continue;

        char *f[10]; /* id|question|A|B|C|D|correct|category|difficulty|marks */
        if (split_line(copy, f, 10) != 10) continue;

        char qEsc[500], aEsc[200], bEsc[200], cEsc[200], dEsc[200];
        json_escape(f[1], qEsc, sizeof(qEsc));
        json_escape(f[2], aEsc, sizeof(aEsc));
        json_escape(f[3], bEsc, sizeof(bEsc));
        json_escape(f[4], cEsc, sizeof(cEsc));
        json_escape(f[5], dEsc, sizeof(dEsc));

        if (!first) printf(",");
        first = 0;

        if (isAdmin) {
            printf("{\"id\":%s,\"question\":\"%s\",\"optionA\":\"%s\",\"optionB\":\"%s\","
                   "\"optionC\":\"%s\",\"optionD\":\"%s\",\"correctAnswer\":\"%s\","
                   "\"category\":\"%s\",\"difficulty\":\"%s\",\"marks\":%s}",
                   f[0], qEsc, aEsc, bEsc, cEsc, dEsc, f[6], f[7], f[8], f[9]);
        } else {
            printf("{\"id\":%s,\"question\":\"%s\",\"optionA\":\"%s\",\"optionB\":\"%s\","
                   "\"optionC\":\"%s\",\"optionD\":\"%s\",\"category\":\"%s\",\"difficulty\":\"%s\"}",
                   f[0], qEsc, aEsc, bEsc, cEsc, dEsc, f[7], f[8]);
        }
    }
    fclose(fp);
    printf("]");
    return 0;
}
