#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  char *p = sbrk(PGSIZE * 17);
  p += PGSIZE * 16 + 32;
  write(2, p, 8);

  exit(1);
}
