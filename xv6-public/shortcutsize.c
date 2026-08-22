#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define ORIGINAL "shortcut_original.txt"
#define SHORTCUT "shortcut"
#define MESSAGE "Greetings sir. This is a larger original file for the symbolic-link size demo."

static void
fail(char *message)
{
  printf(2, "shortcutsize: %s\n", message);
  exit();
}

int
main(void)
{
  struct stat original_st, shortcut_st, resolved_st;
  int fd;
  int created_original = 0;
  int created_shortcut = 0;
  int message_len = strlen(MESSAGE);

  if((fd = open(ORIGINAL, O_RDONLY)) < 0){
    if((fd = open(ORIGINAL, O_CREATE | O_RDWR)) < 0)
      fail("could not create original file");
    if(write(fd, MESSAGE, message_len) != message_len){
      close(fd);
      fail("could not write original file");
    }
    created_original = 1;
  }
  close(fd);

  if((fd = open(SHORTCUT, O_RDONLY | O_NOFOLLOW)) < 0){
    if(symlink(ORIGINAL, SHORTCUT) < 0)
      fail("could not create symbolic link");
    created_shortcut = 1;
  } else {
    if(fstat(fd, &shortcut_st) < 0){
      close(fd);
      fail("could not inspect shortcut");
    }
    close(fd);
    if(shortcut_st.type != T_SYMLINK)
      fail("shortcut already exists and is not a symbolic link");
  }

  if(stat(ORIGINAL, &original_st) < 0)
    fail("could not stat original file");

  if((fd = open(SHORTCUT, O_RDONLY | O_NOFOLLOW)) < 0)
    fail("could not inspect symbolic link");
  if(fstat(fd, &shortcut_st) < 0){
    close(fd);
    fail("could not stat symbolic link");
  }
  close(fd);

  if((fd = open(SHORTCUT, O_RDONLY)) < 0)
    fail("could not follow symbolic link");
  if(fstat(fd, &resolved_st) < 0){
    close(fd);
    fail("could not stat resolved target");
  }
  close(fd);

  printf(1, "original file: %s (%d bytes, %s)\n", ORIGINAL,
         (int)original_st.size,
         created_original ? "created" : "already existed");
  printf(1, "shortcut inode: %s (%d bytes, %s)\n", SHORTCUT,
         (int)shortcut_st.size,
         created_shortcut ? "created" : "already existed");
  printf(1, "shortcut resolved size: %d bytes\n", (int)resolved_st.size);
  printf(1, "Note: the shortcut inode size is the target-path length.\n");
  exit();
}
