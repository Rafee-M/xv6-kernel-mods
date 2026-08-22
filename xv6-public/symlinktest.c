#include "types.h"
#include "stat.h"
#include "param.h"
#include "user.h"
#include "fcntl.h"

/* These guards keep this test buildable before the corresponding headers land. */
#ifndef T_SYMLINK
#define T_SYMLINK 4
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0x400
#endif
#ifndef MAXPATH
#define MAXPATH 128
#endif

int symlink(const char*, const char*);

static char *cleanup_paths[] = {
  "/slt/base", "/slt/basic", "/slt/xdata", "/slt/xdir/link",
  "/slt/two", "/slt/two-a", "/slt/two-b",
  "/slt/nine", "/slt/nine-a", "/slt/nine-b", "/slt/nine-c",
  "/slt/nine-d", "/slt/nine-e", "/slt/nine-f", "/slt/nine-g",
  "/slt/nine-h", "/slt/nine-i",
  "/slt/eleven", "/slt/eleven-a", "/slt/eleven-b",
  "/slt/eleven-c", "/slt/eleven-d", "/slt/eleven-e",
  "/slt/eleven-f", "/slt/eleven-g", "/slt/eleven-h",
  "/slt/eleven-i", "/slt/eleven-j", "/slt/eleven-k",
  "/slt/dangle", "/slt/dangle-link", "/slt/later",
  "/slt/later-link", "/slt/empty", "/slt/oversized", "/slt/self",
  "/slt/cycle-a", "/slt/cycle-b", "/slt/duplicate",
  "/slt/dir-link", "/slt/unlink-target", "/slt/unlink-link",
  "/slt/inspect-target", "/slt/inspect-link",
  "/slt/hard-source", "/slt/hard-copy", "/slt/dev", "/slt/dev-link",
  "/slt/real-dir/inside", "/slt/real-dir", "/slt/path-dir",
  "/slt/xdir", "/slt/dir", "/slt",
  0
};

static void
fail(char *name)
{
  printf(1, "FAIL %s\n", name);
  exit();
}

static void
pass(char *name)
{
  printf(1, "PASS %s\n", name);
}

static void
check(int condition, char *name)
{
  if(!condition)
    fail(name);
}

static void
cleanup(void)
{
  int i;

  for(i = 0; cleanup_paths[i] != 0; i++)
    unlink(cleanup_paths[i]);
}

static void
make_file(char *path, char *contents)
{
  int fd;
  int len;

  fd = open(path, O_CREATE | O_RDWR);
  if(fd < 0)
    fail("create file");
  len = strlen(contents);
  if(write(fd, contents, len) != len){
    close(fd);
    fail("write file");
  }
  close(fd);
}

static int
read_matches(char *path, int mode, char *expected)
{
  char buf[128];
  int fd;
  int n;
  int len;

  fd = open(path, mode);
  if(fd < 0)
    return -1;
  n = read(fd, buf, sizeof(buf) - 1);
  close(fd);
  if(n < 0)
    return -1;
  buf[n] = 0;
  len = strlen(expected);
  return n == len && strcmp(buf, expected) == 0 ? 0 : -1;
}

static void
test_basic(void)
{
  make_file("/slt/base", "basic");
  check(symlink("/slt/base", "/slt/basic") == 0,
        "basic follow");
  check(read_matches("/slt/basic", O_RDONLY, "basic") == 0,
        "basic follow");
  pass("basic follow");
}

static void
test_cross_directory(void)
{
  check(mkdir("/slt/xdir") == 0, "cross-directory");
  make_file("/slt/xdata", "cross");
  check(symlink("/slt/xdata", "/slt/xdir/link") == 0,
        "cross-directory");
  check(read_matches("/slt/xdir/link", O_RDONLY, "cross") == 0,
        "cross-directory");
  pass("cross-directory");
}

static void
test_two_hop(void)
{
  make_file("/slt/two", "two-hop");
  check(symlink("/slt/two", "/slt/two-b") == 0, "2-hop chain");
  check(symlink("/slt/two-b", "/slt/two-a") == 0, "2-hop chain");
  check(read_matches("/slt/two-a", O_RDONLY, "two-hop") == 0,
        "2-hop chain");
  pass("2-hop chain");
}

static void
test_depth(void)
{
  char *nine[] = {
    "/slt/nine-a", "/slt/nine-b", "/slt/nine-c", "/slt/nine-d",
    "/slt/nine-e", "/slt/nine-f", "/slt/nine-g", "/slt/nine-h",
    "/slt/nine-i", 0
  };
  char *eleven[] = {
    "/slt/eleven-a", "/slt/eleven-b", "/slt/eleven-c",
    "/slt/eleven-d", "/slt/eleven-e", "/slt/eleven-f",
    "/slt/eleven-g", "/slt/eleven-h", "/slt/eleven-i",
    "/slt/eleven-j", "/slt/eleven-k", 0
  };
  int i;

  make_file("/slt/nine", "nine");
  for(i = 0; nine[i + 1] != 0; i++)
    check(symlink(nine[i + 1], nine[i]) == 0, "9/11-deep chains");
  check(symlink("/slt/nine", nine[8]) == 0, "9/11-deep chains");
  check(read_matches(nine[0], O_RDONLY, "nine") == 0,
        "9-deep success");

  make_file("/slt/eleven", "eleven");
  for(i = 0; eleven[i + 1] != 0; i++)
    check(symlink(eleven[i + 1], eleven[i]) == 0, "9/11-deep chains");
  check(symlink("/slt/eleven", eleven[10]) == 0, "9/11-deep chains");
  check(open(eleven[0], O_RDONLY) < 0, "11-deep failure");
  pass("9-deep success / 11-deep failure");
}

static void
test_dangling(void)
{
  check(symlink("/slt/dangle", "/slt/dangle-link") == 0,
        "dangling target");
  check(open("/slt/dangle-link", O_RDONLY) < 0, "dangling target");
  pass("dangling target");
}

static void
test_dangling_then_created(void)
{
  check(symlink("/slt/later", "/slt/later-link") == 0,
        "dangling-then-created");
  check(open("/slt/later-link", O_RDONLY) < 0,
        "dangling-then-created");
  make_file("/slt/later", "created");
  check(read_matches("/slt/later-link", O_RDONLY, "created") == 0,
        "dangling-then-created");
  pass("dangling-then-created");
}

static void
test_creation_rejection(void)
{
  char oversized[MAXPATH + 1];
  int i;

  check(symlink("", "/slt/empty") < 0, "empty target rejection");
  for(i = 0; i < MAXPATH; i++)
    oversized[i] = 'x';
  oversized[MAXPATH] = 0;
  check(symlink(oversized, "/slt/oversized") < 0,
        "oversized target rejection");
  pass("empty target rejected");
  pass("oversized target rejected");
}

static void
test_cycles(void)
{
  check(symlink("/slt/self", "/slt/self") == 0, "self-cycle");
  check(open("/slt/self", O_RDONLY) < 0, "self-cycle");
  pass("self-cycle");

  check(symlink("/slt/cycle-b", "/slt/cycle-a") == 0, "2-cycle");
  check(symlink("/slt/cycle-a", "/slt/cycle-b") == 0, "2-cycle");
  check(open("/slt/cycle-a", O_RDONLY) < 0, "2-cycle");
  pass("2-cycle");
}

static void
test_duplicate_name(void)
{
  make_file("/slt/duplicate", "file");
  check(symlink("/slt/base", "/slt/duplicate") < 0,
        "duplicate-name rejection");
  pass("duplicate-name rejection");
}

static void
test_existing_symlink_create(void)
{
  check(open("/slt/basic", O_CREATE | O_RDWR) < 0,
        "O_CREATE existing symlink rejection");
  pass("O_CREATE existing symlink rejection");
}

static void
test_hard_link_symlink(void)
{
  struct stat source, copy;
  int sourcefd, copyfd;

  check(symlink("/slt/base", "/slt/hard-source") == 0,
        "hard-link symlink");
  check(link("/slt/hard-source", "/slt/hard-copy") == 0,
        "hard-link symlink");
  sourcefd = open("/slt/hard-source", O_RDONLY | O_NOFOLLOW);
  copyfd = open("/slt/hard-copy", O_RDONLY | O_NOFOLLOW);
  check(sourcefd >= 0 && copyfd >= 0, "hard-link symlink");
  check(fstat(sourcefd, &source) == 0 && fstat(copyfd, &copy) == 0,
        "hard-link symlink");
  close(sourcefd);
  close(copyfd);
  check(source.type == T_SYMLINK && copy.type == T_SYMLINK &&
        source.ino == copy.ino, "hard-link symlink");
  pass("hard-link symlink");
}

static void
test_device_target(void)
{
  int fd;

  check(mknod("/slt/dev", 1, 0) == 0, "symlink-to-device");
  check(symlink("/slt/dev", "/slt/dev-link") == 0,
        "symlink-to-device");
  fd = open("/slt/dev-link", O_RDONLY);
  check(fd >= 0, "symlink-to-device");
  close(fd);
  pass("symlink-to-device");
}

static void
test_intermediate_component(void)
{
  check(mkdir("/slt/real-dir") == 0, "intermediate symlink component");
  make_file("/slt/real-dir/inside", "inside");
  check(symlink("/slt/real-dir", "/slt/path-dir") == 0,
        "intermediate symlink component");
  check(open("/slt/path-dir/inside", O_RDONLY) < 0,
        "intermediate symlink component");
  pass("intermediate symlink component is not followed");
}

static void
test_directory_target(void)
{
  int fd;

  check(mkdir("/slt/dir") == 0, "symlink-to-dir behavior");
  check(symlink("/slt/dir", "/slt/dir-link") == 0,
        "symlink-to-dir behavior");
  fd = open("/slt/dir-link", O_RDONLY);
  check(fd >= 0, "symlink-to-dir read-only");
  close(fd);
  check(open("/slt/dir-link", O_WRONLY) < 0 &&
        open("/slt/dir-link", O_RDWR) < 0,
        "symlink-to-dir write rejection");
  pass("symlink-to-dir read-only / write behavior");
}

static void
test_unlink_link_only(void)
{
  make_file("/slt/unlink-target", "keep");
  check(symlink("/slt/unlink-target", "/slt/unlink-link") == 0,
        "unlink removes link only");
  check(unlink("/slt/unlink-link") == 0, "unlink removes link only");
  check(read_matches("/slt/unlink-target", O_RDONLY, "keep") == 0,
        "unlink removes link only");
  check(open("/slt/unlink-link", O_RDONLY) < 0,
        "unlink removes link only");
  pass("unlink removes link only");
}

static void
test_nofollow(void)
{
  char *target = "/slt/inspect-target";
  char buf[128];
  struct stat st;
  int fd;
  int len;
  int n;

  make_file(target, "real target");
  check(symlink(target, "/slt/inspect-link") == 0,
        "O_NOFOLLOW inspection");
  fd = open("/slt/inspect-link", O_RDONLY | O_NOFOLLOW);
  check(fd >= 0, "O_NOFOLLOW inspection");
  check(fstat(fd, &st) == 0, "O_NOFOLLOW fstat");
  len = strlen(target);
  check(st.type == T_SYMLINK && st.size == len, "O_NOFOLLOW fstat");
  n = read(fd, buf, sizeof(buf) - 1);
  close(fd);
  check(n == len, "O_NOFOLLOW read-back");
  buf[n] = 0;
  check(strcmp(buf, target) == 0, "O_NOFOLLOW read-back");
  pass("O_NOFOLLOW fstat / read-back");
}

int
main(int argc, char *argv[])
{
  cleanup();
  check(mkdir("/slt") == 0, "test directory");

  test_basic();
  test_cross_directory();
  test_two_hop();
  test_depth();
  test_dangling();
  test_dangling_then_created();
  test_creation_rejection();
  test_cycles();
  test_duplicate_name();
  test_existing_symlink_create();
  test_hard_link_symlink();
  test_device_target();
  test_directory_target();
  test_intermediate_component();
  test_unlink_link_only();
  test_nofollow();

  cleanup();
  printf(1, "ALL TESTS PASSED\n");
  exit();
}
