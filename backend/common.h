/* =========================================================
   common.h
   Shared structures + CGI/file-handling helpers for QuizMaster
   Every backend .c file #includes this so the same fopen()/
   fclose()/fprintf()/fscanf() based file-handling logic and
   the same CGI request-parsing logic isn't duplicated 10 times.
   ========================================================= */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>

/* Makes the ../data/... paths below work no matter which CGI-capable
   server runs the program. Apache's mod_cgi chdir()s into the script's
   own directory before running it, but some lightweight servers (e.g.
   Python's `http.server --cgi`, used for quick local testing) do not -
   they keep the directory the server itself was launched from. Calling
   this once at the top of main(argc, argv) makes every program locate
   its own folder from argv[0] and chdir() there, so "../data/..." always
   resolves the same way regardless of the server. */
/* Makes the ../data/... paths below work no matter which CGI-capable
   server runs the program. Apache's mod_cgi chdir()s into the script's
   own directory before running it, but some lightweight servers (e.g.
   Python's `http.server --cgi`, used for quick local testing) do not -
   they keep the directory the server itself was launched from, AND they
   invoke the program with a bare filename in argv[0] (no path), so
   dirname(argv[0]) can't be trusted either. /proc/self/exe is a Linux
   symlink that always points at the real running binary regardless of
   how it was invoked, so we resolve the executable's own folder from
   that first and only fall back to argv[0] if it's unavailable
   (e.g. running on a non-Linux system). */
static void goto_own_directory(char *argv0) {
    char exePath[1024];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len > 0) {
        exePath[len] = '\0';
        chdir(dirname(exePath));
        return;
    }
    char pathCopy[1024];
    strncpy(pathCopy, argv0, sizeof(pathCopy) - 1);
    pathCopy[sizeof(pathCopy) - 1] = '\0';
    chdir(dirname(pathCopy));
}

/* ---------------- Data file locations ---------------- */
#define USERS_FILE     "../data/users.txt"
#define ADMIN_FILE     "../data/admin.txt"
#define QUESTIONS_FILE "../data/questions.txt"
#define SCORES_FILE    "../data/scores.txt"
#define TEMP_FILE      "../data/temp.txt"

/* ---------------- Data structures --------------------- */
struct Student {
    int  studentId;
    char name[100];
    char email[100];
    char username[50];
    char password[50];
};

struct Question {
    int  id;
    char question[500];
    char optionA[200];
    char optionB[200];
    char optionC[200];
    char optionD[200];
    char correctAnswer;   /* 'A' | 'B' | 'C' | 'D' */
    char category[50];
    char difficulty[20];
    int  marks;
};

struct Score {
    char  username[50];
    char  quizName[100];
    char  date[20];
    int   totalQuestions;
    int   correctAnswers;
    float percentage;
};

/* ---------------- CGI helpers -------------------------- */

/* Every CGI program must print this before any other output. */
static void print_json_header(void) {
    printf("Content-Type: application/json\r\n\r\n");
}

/* Decodes %XX and '+' escaping produced by application/x-www-form-urlencoded bodies. */
static void url_decode(const char *src, char *dst, size_t dstSize) {
    size_t i = 0, j = 0;
    while (src[i] != '\0' && j + 1 < dstSize) {
        if (src[i] == '%' && isxdigit((unsigned char)src[i+1]) && isxdigit((unsigned char)src[i+2])) {
            char hex[3] = { src[i+1], src[i+2], '\0' };
            dst[j++] = (char) strtol(hex, NULL, 16);
            i += 3;
        } else if (src[i] == '+') {
            dst[j++] = ' ';
            i++;
        } else {
            dst[j++] = src[i++];
        }
    }
    dst[j] = '\0';
}

/* Reads the raw POST body from stdin using CONTENT_LENGTH (standard CGI 1.1). */
static char *read_post_body(void) {
    char *lenStr = getenv("CONTENT_LENGTH");
    int len = lenStr ? atoi(lenStr) : 0;
    if (len <= 0) return strdup("");
    char *body = malloc(len + 1);
    if (!body) return strdup("");
    size_t readBytes = fread(body, 1, len, stdin);
    body[readBytes] = '\0';
    return body;
}

/* Finds "key=value" inside a urlencoded body/query string and url-decodes it. */
static int get_form_value(const char *data, const char *key, char *out, size_t outSize) {
    char search[128];
    snprintf(search, sizeof(search), "%s=", key);
    const char *pos = strstr(data, search);
    if (!pos) { out[0] = '\0'; return 0; }
    pos += strlen(search);
    const char *end = strchr(pos, '&');
    size_t rawLen = end ? (size_t)(end - pos) : strlen(pos);
    char *raw = malloc(rawLen + 1);
    memcpy(raw, pos, rawLen);
    raw[rawLen] = '\0';
    url_decode(raw, out, outSize);
    free(raw);
    return 1;
}

/* Escapes quotes/backslashes so field values are safe to embed in JSON output. */
static void json_escape(const char *src, char *dst, size_t dstSize) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j + 2 < dstSize; i++) {
        if (src[i] == '"' || src[i] == '\\') dst[j++] = '\\';
        dst[j++] = src[i];
    }
    dst[j] = '\0';
}

/* Strips the trailing \n / \r\n left by fgets(). */
static void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

/* Splits one pipe-delimited line into `max` fields (in place, using strtok). */
static int split_line(char *line, char *fields[], int max) {
    int count = 0;
    char *tok = strtok(line, "|");
    while (tok != NULL && count < max) {
        fields[count++] = tok;
        tok = strtok(NULL, "|");
    }
    return count;
}

/* Returns today's date as DD Mon YYYY, e.g. "20 Aug 2026". */
static void today_date(char *out, size_t outSize) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(out, outSize, "%d %b %Y", tm_info);
}

#endif /* COMMON_H */
