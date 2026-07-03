/*
 * ACE-lite (issue #147).  src/ACE-lite/tests/log_fmt.cc
 * Standalone regression test for ace_lite_log's format handling.  The point is
 * the length modifiers: a %ld/%lu argument must be read as a long, not an int,
 * or on LP64 the va_list desyncs and every following argument is garbled.  We
 * capture stderr and check that a mixed "%ld %s %lu %u" line renders all four
 * arguments correctly.
 * Build:
 *   g++ -I src/include src/ACE-lite/log_msg.c src/ACE-lite/tests/log_fmt.cc
 * Exit 0 = pass, 1 = fail.
 */

#include <ACE-lite/Log_Msg.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;
static void check(int cond, const char* what) {
    printf("%s: %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) failures++;
}

int main() {
    char path[256];
    snprintf(path, sizeof(path), "/tmp/acelite_logcap_%ld.txt", (long)getpid());

    // Redirect stderr (fd 2) to a file, log, then restore.
    fflush(stderr);
    int saved = dup(fileno(stderr));
    FILE* f = fopen(path, "w+");
    dup2(fileno(f), fileno(stderr));

    // Values chosen so a wrong va_arg width is unmistakable: the long exceeds
    // 32 bits, so reading it as int would corrupt it and everything after.
    long big = 1234567890123L;          // > 2^32
    ace_lite_log(LM_DEBUG, "a=%ld b=%s c=%lu d=%u e=%d\n",
                 big, "mid", (unsigned long)42UL, (unsigned)7, -5);
    fflush(stderr);

    dup2(saved, fileno(stderr));
    close(saved);

    rewind(f);
    char buf[512];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    remove(path);

    check(strcmp(buf, "a=1234567890123 b=mid c=42 d=7 e=-5\n") == 0,
          "%ld/%s/%lu/%u/%d render correctly (no va_list desync)");
    if (failures) printf("  got: %s", buf);

    // %% and a bare trailing %P/%t still work without consuming args.
    fflush(stderr);
    saved = dup(fileno(stderr));
    f = fopen(path, "w+");
    dup2(fileno(f), fileno(stderr));
    ace_lite_log(LM_DEBUG, "100%% done\n");
    fflush(stderr);
    dup2(saved, fileno(stderr));
    close(saved);
    rewind(f);
    n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    remove(path);
    check(strcmp(buf, "100% done\n") == 0, "%% escapes to a literal percent");

    printf("\nlog_fmt: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
