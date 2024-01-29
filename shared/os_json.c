#include "shared.h"
#include "cJSON/cJSON.h"

cJSON * json_fread(const char * path, char retry) {
    cJSON * item = NULL;
    char * buffer = NULL;
    const char *jsonErrPtr;

    if (buffer = get_file_content(path, JSON_MAX_FSIZE), !buffer) {
        mdebug("Cannot get the content of the file: %s", path);
        return NULL;
    }
    
    if (item = cJSON_ParseWithOpts(buffer, &jsonErrPtr, 0), !item) {
        if (retry) {
            mdebug("Couldn't parse JSON file '%s'. Trying to clear comments.", path);
            json_strip(buffer);

            if (item = cJSON_ParseWithOpts(buffer, &jsonErrPtr, 0), !item) {
                mdebug("Couldn't parse JSON file '%s'.", path);
            }
        }
    }

    os_free(buffer);
    return item;
}

int json_fwrite(const char * path, const cJSON * item) {
    FILE * fp = NULL;
    char * buffer;
    size_t size;
    int retval = -1;

    if (buffer = cJSON_PrintUnformatted(item), !buffer) {
        mdebug("Internal error dumping JSON into file '%s'", path);
        return -1;
    }

    size = strlen(buffer);

    if (fp = fopen(path, "w"), !fp) {
        mdebug(OPEN_ERROR, errno, strerror(errno));
        goto end;
    }

    if (fwrite(buffer, 1, size, fp) != size) {
        mdebug("Couldn't write JSON into '%s': %s (%d)", path, strerror(errno), errno);
        goto end;
    }

    retval = 0;

end:
    free(buffer);

    if (fp) {
        fclose(fp);
    }

    return retval;
}

void json_strip(char * json) {
    char * line;
    char * cursor;
    char * next;

    for (line = json; line; line = next) {
        if (next = strchr(line, '\n'), next) {
            *next = '\0';
        }
        printf("line: %s\n", line);
        printf("next: %s\n", next);

        // Skip whitespaces
        cursor = line + strspn(line, " \t");

        if (!strncmp(cursor, "//", 2)) {
            if (next) {
                // If there are more lines, copy all of them
                *next = '\n';
                memmove(cursor, next, strlen(next) + 1);
                next = cursor + 1;
            } else {
                // Otherwise end string here
                *cursor = '\0';
                break;
            }
        } else if (!strncmp(cursor, "/*", 2)) {
            if (next) {
                *next = '\n';
            }

            if (next = strstr(cursor + 2, "*/"), next) {
                memmove(cursor, next + 2, strlen(next + 2) + 1);
                next = cursor;
            } else {
                // This is a syntax error - unterminated comment
                break;
            }
        } else if (next) {
            // Restore newline and move forward
            *next++ = '\n';
        }
    }
}


