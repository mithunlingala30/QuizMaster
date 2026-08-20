/* =========================================================
   add_question.c
   CGI program (POST) - admin adds a new question.

   File handling demonstrated:
     fopen(..., "r") to scan for the highest existing ID
     fopen(..., "a") to append the new record
   ========================================================= */
#include "common.h"

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char *body = read_post_body();

    char question[500], optA[200], optB[200], optC[200], optD[200];
    char correct[5], category[50], difficulty[20], marksStr[10];
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

    if (strlen(question) == 0 || strlen(optA) == 0 || strlen(optB) == 0 ||
        strlen(optC) == 0 || strlen(optD) == 0 || strlen(correct) == 0) {
        printf("{\"success\":false,\"message\":\"All question fields are required.\"}");
        return 0;
    }

    /* ---- Find the next ID by scanning existing records ---- */
    int nextId = 1;
    FILE *fp = fopen(QUESTIONS_FILE, "r");
    if (fp != NULL) {
        char line[1200];
        while (fgets(line, sizeof(line), fp) != NULL) {
            int id;
            if (sscanf(line, "%d|", &id) == 1 && id >= nextId) nextId = id + 1;
        }
        fclose(fp);
    }

    int marks = atoi(marksStr);
    if (marks <= 0) marks = 1;

    fp = fopen(QUESTIONS_FILE, "a");
    if (fp == NULL) {
        printf("{\"success\":false,\"message\":\"Could not open questions file.\"}");
        return 0;
    }
    fprintf(fp, "%d|%s|%s|%s|%s|%s|%c|%s|%s|%d\n",
            nextId, question, optA, optB, optC, optD,
            toupper((unsigned char)correct[0]), category, difficulty, marks);
    fclose(fp);

    printf("{\"success\":true,\"message\":\"Question added.\",\"id\":%d}", nextId);
    return 0;
}
