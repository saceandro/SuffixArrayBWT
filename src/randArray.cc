#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char value_to_char(int v)
{
  switch (v)
    {
    case 0:
      return '$';
    case 1:
      return 'A';
    case 2:
      return 'C';
    case 3:
      return 'G';
    case 4:
      return 'T';
    default:
      return 'N';
      // {
      //   fprintf(stderr, "There is a character not $ACGT(acgt)!!: %c\n", a);
      //   exit(EXIT_FAILURE);
      // }
    }
}

int main(int argc, char** argv)
{
  srand((unsigned) time(NULL));
  FILE* f = fopen(argv[1], "w");
  int i;
  for (i=0; i<atoi(argv[2]); i++)
    fprintf(f, "%c", value_to_char(rand()%4 + 1));
  fprintf(f, "$\n");
  return 0;
}
