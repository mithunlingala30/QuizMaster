/* =========================================================
   delete_question.c
   CGI program (GET or POST) - admin deletes a question by ID.

   File handling demonstrated:
     Rebuild the file skipping the target record, using the
     same read/write-temp/remove/rename pattern as edit_question.c
   ========================================================= */
#include "common.h"

#define BACKUP_FILE "../data/deleted_questions_backup.dat"
#define REC_SIZE 1200

int main(int argc, char *argv[]) {
    goto_own_directory(argv[0]);
    print_json_header();

    char idStr[10] = "";
    char *qs = getenv("QUERY_STRING");
    if (qs != NULL) get_form_value(qs, "id", idStr, sizeof(idStr));
    if (strlen(idStr) == 0) {
        char *body = read_post_body();
        get_form_value(body, "id", idStr, sizeof(idStr));
        free(body);
    }
    int targetId = atoi(idStr);

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
            found = 1;

            /* fwrite() demo: keep a fixed-size binary backup record of
               every deleted question, in case the admin needs to recover
               it. Padded/truncated to REC_SIZE so records are addressable
               by index later (see fread()/fseek() readback below). */
            char record[REC_SIZE];
            memset(record, 0, REC_SIZE);
            strncpy(record, line, REC_SIZE - 1);
            FILE *bak = fopen(BACKUP_FILE, "ab");
            if (bak != NULL) {
                fwrite(record, sizeof(char), REC_SIZE, bak);
                fclose(bak);
            }
            continue; /* skip -> effectively deletes this record */
        }
        fputs(line, dst);
    }
    fclose(src);
    fclose(dst);

    remove(QUESTIONS_FILE);
    rename(TEMP_FILE, QUESTIONS_FILE);

    /* fread()/fseek() demo: immediately read the last backup record back
       from disk to confirm the binary backup was written successfully. */
    int backedUp = 0;
    if (found) {
        FILE *bak = fopen(BACKUP_FILE, "rb");
        if (bak != NULL) {
            fseek(bak, -REC_SIZE, SEEK_END);   /* jump to last record */
            char verify[REC_SIZE];
            if (fread(verify, sizeof(char), REC_SIZE, bak) == REC_SIZE) {
                backedUp = 1;
            }
            fclose(bak);
        }
    }

    if (found) {
        printf("{\"success\":true,\"message\":\"Question deleted.\",\"backedUp\":%s}",
               backedUp ? "true" : "false");
    } else {
        printf("{\"success\":false,\"message\":\"Question ID not found.\"}");
    }
    return 0;
}
