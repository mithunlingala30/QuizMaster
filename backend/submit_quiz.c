/* =========================================================
   submit_quiz.c
   CGI program (POST) - the heart of the project.

   The browser sends only the student's chosen letters, e.g.
     username=student01&quizName=C+Programming+Basics
     &answers=1:A,2:C,3:,4:B,...

   Nothing about correctness is trusted from the client. This
   program re-reads questions.txt itself, compares each answer
   against the real correct answer on the server, computes the
   score, and appends the result to scores.txt with fopen(...,"a").
   ========================================================= */
#include "common.h"

#define MAX_Q 200

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char *body = read_post_body();
    char username[50], quizName[100], answers[4000];
    get_form_value(body, "username", username, sizeof(username));
    get_form_value(body, "quizName", quizName, sizeof(quizName));
    get_form_value(body, "answers", answers, sizeof(answers));
    free(body);

    if (strlen(username) == 0) {
        printf("{\"success\":false,\"message\":\"Missing username.\"}");
        return 0;
    }

    /* ---- Load the answer key from file (server-side truth) ---- */
    struct Question bank[MAX_Q];
    int bankSize = 0;
    FILE *fp = fopen(QUESTIONS_FILE, "r");
    if (fp == NULL) {
        printf("{\"success\":false,\"message\":\"Questions file not found.\"}");
        return 0;
    }
    char line[1200];
    while (fgets(line, sizeof(line), fp) != NULL && bankSize < MAX_Q) {
        char copy[1200];
        strcpy(copy, line);
        trim_newline(copy);
        if (strlen(copy) == 0) continue;
        char *f[10];
        if (split_line(copy, f, 10) != 10) continue;
        bank[bankSize].id = atoi(f[0]);
        strncpy(bank[bankSize].question, f[1], sizeof(bank[bankSize].question) - 1);
        bank[bankSize].correctAnswer = f[6][0];
        bankSize++;
    }
    fclose(fp);

    /* ---- Parse "id:letter,id:letter,..." submitted by the browser ---- */
    int total = 0, correctCount = 0, unanswered = 0;
    char detail[8000] = "[";
    int firstDetail = 1;

    char answersCopy[4000];
    strcpy(answersCopy, answers);
    char *pair = strtok(answersCopy, ",");
    while (pair != NULL) {
        int qid = atoi(pair);
        char *colon = strchr(pair, ':');
        char given = (colon && strlen(colon + 1) > 0) ? colon[1] : '\0';

        /* find the matching question in the bank read from file */
        for (int i = 0; i < bankSize; i++) {
            if (bank[i].id == qid) {
                total++;
                int isCorrect = (given != '\0' && given == bank[i].correctAnswer);
                if (given == '\0') unanswered++;
                if (isCorrect) correctCount++;

                char qEsc[500];
                json_escape(bank[i].question, qEsc, sizeof(qEsc));

                char givenStr[4] = "";
                if (given != '\0') { givenStr[0] = given; givenStr[1] = '\0'; }

                char buf[700];
                snprintf(buf, sizeof(buf),
                    "%s{\"question\":\"%s\",\"yourAnswer\":\"%s\",\"correctAnswer\":\"%c\",\"isCorrect\":%s}",
                    firstDetail ? "" : ",", qEsc, givenStr, bank[i].correctAnswer,
                    isCorrect ? "true" : "false");
                strcat(detail, buf);
                firstDetail = 0;
                break;
            }
        }
        pair = strtok(NULL, ",");
    }
    strcat(detail, "]");

    float percentage = (total > 0) ? (100.0f * correctCount / total) : 0.0f;

    /* ---- Store the result: username|quizName|date|total|correct|percentage ---- */
    char dateStr[20];
    today_date(dateStr, sizeof(dateStr));

    fp = fopen(SCORES_FILE, "a");
    if (fp != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%d|%.2f\n", username, quizName, dateStr, total, correctCount, percentage);
        fclose(fp);
    }

    printf("{\"success\":true,\"total\":%d,\"correct\":%d,\"wrong\":%d,\"unanswered\":%d,"
           "\"percentage\":%.2f,\"date\":\"%s\",\"details\":%s}",
           total, correctCount, total - correctCount - unanswered, unanswered,
           percentage, dateStr, detail);
    return 0;
}
