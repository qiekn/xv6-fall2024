#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *fmtname(char *path) {
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--);
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ) return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

void ls(char *path) {
  char buf[512], *p;
  int fd;
  struct dirent de;  // directory entity
  struct stat st;

  // 目录也是文件，打开文件获得文件描述符 fd (File Descriptor)
  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  // The fstat system call retrieves information from the inode that a file
  // descriptor refers to. It fills in a struct stat, defined in stat.h
  // 通过 fd 找到打开文件的信息并放进 st 指向的 stat 结构体
  if (fstat(fd, &st) < 0) {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
    case T_DEVICE:
    case T_FILE:
      // 对于文件和设备直接打印信息
      printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, (int)st.size);
      break;

    case T_DIR:
      // buf 存放路径信息，下面对 strlen 计算举例说明 ls /Users/qiekn/code，
      // "/Users/qiekn/code" 目录下有 main.c
      // path     "/Users/qiekn"
      // +1       "/"
      // +DIRSIZ  "main.c"  (DIRSIZ = 14)
      // +1       "EOF"
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        printf("ls: path too long\n");
        break;
      }
      strcpy(buf, path);
      p = buf + strlen(buf);
      *p++ = '/';
      while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;
        memmove(p, de.name, DIRSIZ);  // path + "/" + de.name
        p[DIRSIZ] = 0;                // EOF, c style string
        if (stat(buf, &st) < 0) {
          printf("ls: cannot stat %s\n", buf);
          continue;
        }
        printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, (int)st.size);
      }
      break;
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int i;

  // 参数只有一个，也就是只输入 ls，那么列出当前目录中的 ”文件“
  if (argc < 2) {
    ls(".");
    exit(0);
  }
  // 多个目录的时候，例如 ls ~/a ~/b ~/c, 对每个目录调用 ls(char *path)
  for (i = 1; i < argc; i++) {
    ls(argv[i]);
  }
  exit(0);
}
