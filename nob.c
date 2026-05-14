#define NOB_IMPLEMENTATION
#include "../nob.h"

#include <string.h>

int main(int argc, char** argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd cmd = {0};

  if (argc - 1 && strcmp(argv[1], "test") == 0) {
    if (!nob_mkdir_if_not_exists("build")) return 1;
    nob_cmd_append(&cmd, "gcc", "-Wall", "-Wextra", "-pedantic", "-ggdb", "-o", "build/test", "test.c");
    if (!nob_cmd_run(&cmd)) return 1;
    nob_cmd_append(&cmd, "build/test");
    if (!nob_cmd_run(&cmd)) return 1;
  } else {
    if (!nob_mkdir_if_not_exists("build")) return 1;
    nob_cmd_append(&cmd, "gcc", "-Wall", "-Wextra", "-pedantic", "-Wconversion", "-ggdb", "-O0", "-o", "build/main", "main.c");
    if (!nob_cmd_run(&cmd)) return 1;
    nob_cmd_append(&cmd, "build/main");
    if (!nob_cmd_run(&cmd)) return 1;
  }
}

