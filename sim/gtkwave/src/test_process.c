// ppc_dasm_one()
#include "stdio.h"


int main(int argc, char **argv)
{
  while(1)
  {
    char buf[1025], buf2[1025];
    buf[0] = 0;
    fscanf(stdin, "%s", buf);
    if(buf[0])
    {
      int hx;
      sscanf(buf, "%x", &hx);

      // ppc_dasm_one(buf2, 0, hx);
      printf("%s\n", "hi"); // was buf2
      fflush(stdout);
    }
  }
  return(0);
}
