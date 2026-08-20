/* =========================================================
   edit_question.c
   CGI program (POST) - admin edits an existing question.

   File handling demonstrated (the classic C "update a record"
   technique, since C has no in-place "replace line N" call):
     1. fopen(original, "r")
     2. fopen(temp,     "w")
     3. copy every record across, substituting the edited one
     4. fclose() both files
     5. remove() the original, rename() temp -> original
   ========================================================= */
#include "common.h"

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char *body = read_post_body();

    char idStr[10], question[500], optA[200], optB[200], optC[200], optD[200];
    char correct[5], category[50], difficulty[20], marksStr[10];
    get_form_value(body, "id", idStr, sizeof(idStr));
    get_form_value(body, "question", question, sizeof(question));
    get_form_value(body, "optionA", optA, sizeof(optA));
    get_form_value(body, "optionB", optB, sizeof(optB));
    get_form_value(body, "optionC", optC, sizeof(optC));
    get_form_value(body, "optionD", optD, sizeof(optD));
    get_form_value(body, "correctAnswer", correct, sizeof(correct));
    get_form_value(body, "category", category, sizeof(category));
    get_form_value(body, "difficulty", difficulty, sizeof(difficulty));
    get_form_value(body, "marks", marksStr, sizeof(marksStr));
    free(body);

    int targetId = atoi(idStr);
    int marks = atoi(marksStr);
    if (marks <= 0) marks = 1;

    FILE *src = fopen(QUESTIONS_FILE, "r");
    if (src == NULL) {
        printf("{\"success\":false,\"message\":\"Questions file not found.\"}");
        return 0;
    }
    FILE *dst = fopen(TEMP_FILE, "w");
    if (dst == NULL) {
        fclose(src);
        printf("{\"success\":false,\"message\":\"Could not create temp file.\"}");
        return 0;
    }

    int found = 0;
    char line[1200];
    while (fgets(line, sizeof(line), src) != NULL) {
        int id;
        if (sscanf(line, "%d|", &id) == 1 && id == targetId) {
            fprintf(dst, "%d|%s|%s|%s|%s|%s|%c|%s|%s|%d\n",
                    targetId, question, optA, optB, optC, optD,
                    toupper((unsigned char)correct[0]), category, difficulty, marks);
            found = 1;
        } else {
            fputs(line, dst); /* record unchanged, copy through untouched */
        }
    }
    fclose(src);
    fclose(dst);

    remove(QUESTIONS_FILE);
    rename(TEMP_FILE, QUESTIONS_FILE);

    if (found) {
        printf("{\"success\":true,\"message\":\"Question updated.\"}");
    } else {
        printf("{\"success\":false,\"message\":\"Question ID not found.\"}");
    }
    return 0;
}
