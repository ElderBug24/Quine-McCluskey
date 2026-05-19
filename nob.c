#define NOB_IMPLEMENTATION
#include "../nob.h"

#include <string.h>
#include <time.h>

int main(int argc, char** argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd cmd = {0};

  clock_t begin = clock();

  if (argc - 1 && strcmp(argv[1], "debug") == 0) {
    if (!nob_mkdir_if_not_exists("build")) return 1;
    nob_cmd_append(&cmd, "gcc", "-std=c23", "-Wall", "-Wextra", "-Wconversion", "-pedantic", "-ggdb", "-O0", "-o", "build/debug", "main.c", "-lm");
    if (!nob_cmd_run(&cmd)) return 1;
    nob_cmd_append(&cmd, "build/debug");
    if (!nob_cmd_run(&cmd)) return 1;
  } else if (argc - 1 && strcmp(argv[1], "build") == 0) {
    if (!nob_mkdir_if_not_exists("build")) return 1;
    nob_cmd_append(&cmd, "gcc", "-std=c23", "-Wall", "-Wextra", "-Wconversion", "-pedantic", "-O3", "-o", "build/main", "main.c", "-lm");
    if (!nob_cmd_run(&cmd)) return 1;
  } else {
    if (!nob_mkdir_if_not_exists("build")) return 1;
    nob_cmd_append(&cmd, "gcc", "-std=c23", "-Wall", "-Wextra", "-Wconversion", "-pedantic", "-O3", "-o", "build/main", "main.c", "-lm");
    if (!nob_cmd_run(&cmd)) return 1;
    nob_cmd_append(&cmd, "build/main");
    if (!nob_cmd_run(&cmd)) return 1;
  }

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("Executed in %f secs\n", time_spent);
}

