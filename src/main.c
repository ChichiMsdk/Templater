/* NOTE: Generate a build.bat and folder src/main.c for new project */

#define SUB_CONSOLE
#include "chihab.h"
#include "cm_string.c"
#include "cm_io.c"
#include "cm_win32.c"
#include "cm_memory.c"

#include "templater.h"

u64 
strlengths_impl(char* str, ...)
{
  va_list args;
  va_start(args, str);
  char* first;
  u64 total_size = strlen(str);
  while ((first = va_arg(args, char*))) total_size += strlen(first);

  va_end(args);
  return total_size;
}

#define strlengths(str, ...) strlengths_impl(str, __VA_ARGS__, NULL)

#define REAL_MAX_PATH 32767

i32
main(i32 argc, char** argv)
{ 
  char  *append, *path;
  char  *source, *program, *extension, *build_file;
  u32   template_len, cwd_size, hypot_size;

  program   = (argc >= 2) ? argv[1] : "default";
  source    = (argc >= 3) ? argv[2] : "main";
  extension = (argc >= 4) ? argv[3] : "exe";

  if (argc < 2) {printf("templater <program> <source_file> <extension>\n");}
  printf("Generating source '%s.c' for program '%s.%s'\n", source, program, extension);

  /* NOTE: Create the build file */
  {
    template_len = strlengths(g_template_build, source, program, extension) + 1;
    heap_alloc_dz(sizeof(char) * template_len, build_file);
    template_len = wnsprintf(build_file, template_len, g_template_build, program, source, extension);
  }

  heap_alloc_dz(sizeof(char) * (REAL_MAX_PATH + 1), path);
  cwd_size = cwd_get(path, REAL_MAX_PATH);

  append = "bin";
  {
    hypot_size = cwd_size + strlen(append) + 2;
    /* FIXME: Is this even correct?!..*/
    hypot_size = (hypot_size >= REAL_MAX_PATH - 2) ? REAL_MAX_PATH - 2 : hypot_size;
    wnsprintf(&path[cwd_size], hypot_size, "\\%s", append);

    directory_create_rf(path);
    memset(&path[cwd_size], 0, REAL_MAX_PATH - cwd_size - 1);
  }

  append = "src";
  {
    hypot_size = cwd_size + strlen(append) + 2;
    hypot_size = (hypot_size >= REAL_MAX_PATH - 2) ? REAL_MAX_PATH - 2 : hypot_size;
    wnsprintf(path + cwd_size, hypot_size, "\\%s", append);

    directory_create_rf(path);
    memset(&path[cwd_size], 0, REAL_MAX_PATH - cwd_size - 1);
  }

  append = "build.bat";
  {
    hypot_size = cwd_size + strlen(append) + 2;
    hypot_size = (hypot_size >= REAL_MAX_PATH - 2) ? REAL_MAX_PATH - 2 : hypot_size;
    wnsprintf(path + cwd_size, hypot_size, "\\%s", append);
    if (!file_dump(path, build_file, template_len)) printf("Finished writing to %s\n", path);
    else printf("Something went wrong when dumping to %s..\n", path);
  }

  append = "src";
  {
    hypot_size = cwd_size + strlen(append) + strlen(source) + 1 + 2 + 2;
    hypot_size = (hypot_size >= REAL_MAX_PATH - 2) ? REAL_MAX_PATH - 2 : hypot_size;
    wnsprintf(path + cwd_size, hypot_size, "\\%s\\%s.c", append, source);
    if (!file_dump(path, g_template_source, strlen(g_template_source))) printf("Finished writing to %s\n", path);
    else printf("Something went wrong when dumping to %s..\n", path);
  }

  heap_free_dz(path);
  heap_free_dz(build_file);
  return 0;
}

ENTRY
{
  i32           return_value;
  _Command_Line cl;

  if (!command_line_args_ansi(&cl)) {printf("Failed retrieving command line\n"); RETURN_FROM_MAIN(EXIT_FAILURE);}

  return_value = main(cl.argc, cl.argv);

  RETURN_FROM_MAIN((u32)return_value);
}
