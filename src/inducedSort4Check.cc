#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define handle_error(msg)                               \
  do {perror(msg); exit(EXIT_FAILURE);} while(0)

int char_to_value(char a)
{
  switch (a)
    {
    case '$':
      return 0;
    case 'A': case 'a':
      return 1;
    case 'C': case 'c':
      return 2;
    case 'G': case 'g':
      return 3;
    case 'T': case 't':
      return 4;
    default:
      return 5;
      // {
      //   fprintf(stderr, "There is a character not $ACGT(acgt)!!: %c\n", a);
      //   exit(EXIT_FAILURE);
      // }
    }
}

void encode(char* array, int* s, int len)
{
  int i;
  for (i=0; i<len; i++)
    s[i] = char_to_value(array[i]);
}

void displayArray(int *array, int len, int fd)
{
  int i;
  if (fd == 1)
    {
      for (i=0; i<len; i++)
        printf("%d ", array[i]);
      printf("\n");
    }
  else if (fd == 2)
    {
      for (i=0; i<len; i++)
        fprintf(stderr, "%d ", array[i]);
      fprintf(stderr, "\n");
    }
  else
    {
      fprintf(stderr, "unknown file descriptor\n");
      exit(EXIT_FAILURE);
    }
}

void lms(int* s, int* type, int len)
{
  // type 0: LMS
  // type 1: S
  // type 2: L
  int i;
  for (i=len-2; i>=0; i--)
    {
      if (s[i] == s[i+1])
        type[i] = type[i+1];
      else if (s[i] < s[i+1])
        type[i] = 1;
      else
        type[i] = 2;
    }
  
  for (i=len-2; i>0; i--)
    if (type[i-1] == 2 && type[i] == 1)
      type[i] = 0;
}

void occurrence(int* s, int len, int* count, int types)
{
  int i;
  for (i=0; i<len; i++)
    count[s[i]]++;
  for (i=1; i<types; i++)
    count[i] += count[i-1];
}

int initialize_sa(int* s, int* sa, int* type, int len, int* count, int* cursor, int types)
{
  int i;
  int lmscount=0;
  
  for (i=0; i<types; i++)
    cursor[i] = count[i];

  for (i=0; i<len; i++)
    if (type[i] == 0)
      {
        sa[--cursor[s[i]]] = i;
        lmscount++;
      }

  return lmscount;
}

void initialize_sa_with_lms(int* s, int* lms, int lmslen, int* sa, int* count, int* cursor, int types)
{
  int i;
  
  for (i=0; i<types; i++)
    cursor[i] = count[i];

  for (i=lmslen-1; i>=0; i--)
    sa[--cursor[s[lms[i]]]] = lms[i];
}

void inducedSort(int* s, int* sa, int* type, int len, int* count, int* cursor, int types)
{
  int i;
  
  cursor[0] = 0;
  for (i=1; i<types; i++)
    cursor[i] = count[i-1];

  // fprintf(stderr, "phase1:\n");
  i=0;
  while (i<len)
    {
      if (sa[i] != -1 && sa[i] != 0 && type[sa[i]-1] == 2)
        {
          // fprintf(stderr, "now: %d\n", sa[i]);
          sa[cursor[s[sa[i]-1]]++] = sa[i]-1;
              
          // fprintf(stderr, "sa: ");
          // displayArray(sa, len, 2);
          // fprintf(stderr, "\n");
        }
      i++;
    }

  for (i=0; i<types; i++)
    cursor[i] = count[i];

  // fprintf(stderr, "phase2:\n");
  i=len-1;
  while (i>=0)
    {
      if (sa[i] != -1 && sa[i] != 0 && type[sa[i]-1] != 2)
        {
          // fprintf(stderr, "now: %d\n", sa[i]);
          sa[--cursor[s[sa[i]-1]]] = sa[i]-1;
              
          // fprintf(stderr, "sa: ");
          // displayArray(sa, len, 2);
          // fprintf(stderr, "\n");
        }
      i--;
    }
}

int* inducedSortLms(int* s, int len, int types)
{
  int i,j;
  int* count = (int*)calloc(len, sizeof(int));
  if (count == NULL)
    handle_error("calloc");
  
  occurrence(s, len, count, types);

  int* type = (int*)calloc(len, sizeof(int));
  if (type == NULL)
    handle_error("calloc");

  int* sa = (int*)malloc(len*sizeof(int));
  if (sa == NULL)
    handle_error("malloc");
  
  memset(sa, -1, len*sizeof(int));
  
  lms(s, type, len);

  // fprintf(stderr, "s\t");
  // displayArray(s, len, 2);
  // fprintf(stderr, "t\t");
  // displayArray(type, len, 2);

  int* cursor = (int*)calloc(types, sizeof(int));
  if (cursor == NULL)
    handle_error("calloc");
  
  int lmslen = initialize_sa(s, sa, type, len, count, cursor, types);

  cursor[0] = 0;
  for (i=1; i<types; i++)
    cursor[i] = count[i-1];

  // fprintf(stderr, "lms phase1:\n");
  i=0;
  while (i<len)
    {
      if (sa[i] != -1 && sa[i] != 0 && type[sa[i]-1] == 2)
        {
          // fprintf(stderr, "now: %d\n", sa[i]);
          
          sa[cursor[s[sa[i]-1]]++] = sa[i]-1;
              
          // fprintf(stderr, "sa: ");
          // displayArray(sa, len, 2);
        }
      i++;
    }

  for (i=0; i<types; i++)
    cursor[i] = count[i];

  // fprintf(stderr, "lms phase2\n");
  i=len-1;
  while (i>=0)
    {
      if (sa[i] != -1 && sa[i] != 0 && type[sa[i]-1] != 2)
        {
          // fprintf(stderr, "now: %d\n", sa[i]);
          
          sa[--cursor[s[sa[i]-1]]] = sa[i]-1;
              
          // fprintf(stderr, "sa: ");
          // displayArray(sa, len, 2);
        }
      i--;
    }

  int* s2 = (int*)calloc(len, sizeof(int));
  if (s2 == NULL)
    handle_error("calloc");
  memset(s2, -1, len*sizeof(int));
  
  int rank=-1;
  s2[len-1] = ++rank;

  int prevpos = len-1;
  for (i=1; i<len; i++)
    {
      if (type[sa[i]] == 0)
        {
          if (s[sa[i]] == s[prevpos])
            {
              j=1;
              while (type[sa[i]+j]!=0 && type[prevpos+j]!=0 && s[sa[i]+j] == s[prevpos+j])
                j++;
              if (type[sa[i]+j]==0 && type[prevpos+j]==0)
                {
                  if (s[sa[i]+j] == s[prevpos+j])
                    s2[sa[i]] = rank;
                  else
                    {
                      s2[sa[i]] = ++rank;
                      prevpos = sa[i];
                    }
                }
              else
                {
                  s2[sa[i]] = ++rank;
                  prevpos = sa[i];
                }
            }
          else
            {
              s2[sa[i]] = ++rank;
              prevpos = sa[i];
            }
          // fprintf(stderr, "s2\t");
          // displayArray(s2, len, 2);
        }
    }
  // fprintf(stderr, "rank+1: %d\n", rank+1);
  
  j=0;
  for (i=0; i<len; i++)
    if (s2[i] != -1)
      s2[j++] = s2[i];
  
  int* lmss = (int*) calloc(lmslen, sizeof(int));
  if (lmss == NULL)
    handle_error("calloc");

  j=0;
  for (i=0; i<len; i++)
    if (type[i] == 0)
      lmss[j++] = i;

  // fprintf(stderr, "lmss: ");
  // displayArray(lmss, lmslen, 2);
  // fprintf(stderr, "s2: ");
  // displayArray(s2, lmslen, 2);

  int* sortedlms = (int*)calloc(lmslen, sizeof(int));
  if (sortedlms == NULL)
    handle_error("calloc");
        
  if (rank+1 == lmslen)
    {
      for (i=0; i<lmslen; i++)
        sortedlms[s2[i]] = lmss[i];
      free(s2);
    }
  else
    {
      int* ret = inducedSortLms(s2, lmslen, rank+1);
      for (i=0; i<lmslen; i++)
        sortedlms[i] = lmss[ret[i]];
      free(ret);
    }

  free(lmss);
  // fprintf(stderr, "sortedlms: ");
  // displayArray(sortedlms, lmslen, 2);

  memset(sa, -1, len*sizeof(int));
  initialize_sa_with_lms(s, sortedlms, lmslen, sa, count, cursor, types);
  
  inducedSort(s, sa, type, len, count, cursor, types);

  free(s);
  free(type);
  free(count);
  free(cursor);
  
  return sa;
}

void bucketSort(int* s, int* sa, int* isa, int len)
{
  // All elements are assumed to be nonnegative integers.
  
  int i=0;
  // Array for counting numbers of the character.
  int count[5] = {0,0,0,0,0};
  
  for(i=0; i<len; i++)
    count[s[i]]++;
  
  // Count the number of elements <= i.
  for(i=1; i<5; i++)
    count[i] += count[i-1];
  
  for(i=0; i<len; i++)
    isa[i] = count[s[i]] - 1;
  
  // Scan the input array from the tail, and
  // insert each element in the proper position.
  for(i=len-1; i>=0; i--)
    sa[ --count[s[i]] ] = i;
}

void ternaryPartition(int* sa, int* isa, int left, int right, int depth, int* newleftright)
{
  // Select a number between left and right at random.
  int random = left + rand() % (right - left + 1);

  // Exchange target[right] and target[random].
  int tmp = sa[right];
  sa[right] = sa[random];
  sa[random] = tmp;

  int pivot = isa[sa[right]+depth];
  int i = left-1;
  // i scans the array from the left.
  int j = right;
  // j scans the array from the right.

  int mi=left-1;
  int mj=right;
  
  for (;;)
    {
      // Move from the left until hitting a value no less than the pivot.
      for(i++; isa[sa[i]+depth] <= pivot && i <= j && i<right; i++)
        {
          if (isa[sa[i]+depth]==pivot)
            {
              tmp = sa[i];
              sa[i] = sa[++mi];
              sa[mi] = tmp;
            }
        }

      // Move from the right until hitting a value no greater than the pivot.
      for(j--; pivot <= isa[sa[j]+depth] && i <= j; j--)
        {
          if (isa[sa[j]+depth]==pivot)
            {
              tmp = sa[j];
              sa[j] = sa[--mj];
              sa[mj] = tmp;
            }
        }

      if (i >= j) break;

      // Exchange sa[i] and sa[j].
      tmp = sa[i];
      sa[i] = sa[j];
      sa[j] = tmp;
    }

  while (mi>=left)
    {
      tmp = sa[mi];
      sa[mi] = sa[j];
      sa[j] = tmp;
      mi--;
      j--;
    }
  
  while (mj<=right)
    {
      tmp = sa[mj];
      sa[mj] = sa[i];
      sa[i] = tmp;
      mj++;
      i++;
    }

  newleftright[0] = j;
  newleftright[1] = i;
}

void ternaryQsort(int* sa, int* isa, int aLeft, int right, int depth)
{
  int i;
  int newleftright[2];
  int left = aLeft;
  while (left < right)
    {
      // fprintf(stderr, "left:%d\tright:%d\n", left, right);
      ternaryPartition(sa, isa, left, right, depth, newleftright);
      
      ternaryQsort(sa, isa, left, newleftright[0], depth);

      for (i=newleftright[0]+1; i<newleftright[1]; i++)
        {
          // fprintf(stderr, "isa[sa[%d]] = %d\n", i, newleftright[1]-1);
          isa[sa[i]] = newleftright[1]-1;
        }
      
      left = newleftright[1];
    }
  if (left == right)
    {
      // fprintf(stderr, "isa[sa[%d]] = %d\n\n", left, left);
      isa[sa[left]] = left;
    }
}

int* larssonSadakane(int* s, int* sa, int* isa, int len)
{

  bucketSort(s, sa, isa, len);

  // displayArray(sa, len);
  // displayArray(isa, len);
  
  int i=0, depth=1;
  bool notfin = true;
  while (notfin)
    {
      notfin = false;
      while (i<len)
        {
          if (i != isa[sa[i]])
            {
              // fprintf(stderr, "%d\t%d\n", i, isa[sa[i]]);
              notfin = true;
              ternaryQsort(sa, isa, i, isa[sa[i]], depth);
            }
          i = isa[sa[i]] + 1;
        }
      // displayArray(sa, len);
      // displayArray(isa, len);
      i=0;
      depth*=2;
    }
  return sa;
}

void randArray(int* s, int len)
{
  int i;
  for (i=0; i<len-1; i++)
    s[i] = rand()%4 + 1;
  s[len-1] = 0;
}

int main()
{
  int i,j;
  int sample = 10000;
  int len = 10000;

  srand((unsigned) time(NULL));
  
  int* s = (int*)calloc(len, sizeof(int));
  if (s == NULL)
    handle_error("calloc");

  int* sa;
  
  int* larssonSadakane_sa = (int*)calloc(len, sizeof(int));
  if (larssonSadakane_sa == NULL)
    handle_error("calloc");

  int* isa = (int*)calloc(len, sizeof(int));
  if (isa == NULL)
    handle_error("calloc");
  
  for (j=0; j<sample; j++)
    {
      randArray(s, len);

      int* s2 = (int*)calloc(len, sizeof(int));
      if (s2==NULL)
        handle_error("calloc");

      for (i=0; i<len; i++)
        s2[i] = s[i];

      memset(larssonSadakane_sa, 0, len*sizeof(int));
      memset(isa, 0, len*sizeof(int));
      larssonSadakane(s, larssonSadakane_sa, isa, len);
      
      sa = inducedSortLms(s2, len, 6);
      
      // fprintf(stderr, "final suffix array:\n");
      // displayArray(sa, len, 2);

      int bug = 0;
      for (i=0; i<len; i++)
        if (sa[i] != larssonSadakane_sa[i])
          {
            printf("bug pos\t%d %d\n", sa[i], larssonSadakane_sa[i]);
            bug = 1;
          }
      if (bug)
        {
          fprintf(stderr, "BUG!!!\n");
          printf("randArray\t");
          displayArray(s, len, 1);
          printf("LS-SA\t");
          displayArray(larssonSadakane_sa, len, 1);
          printf("IS-SA\t");
          displayArray(sa, len, 1);
          printf("\n");
          printf("LS-SA's suffix\n");
          for (i=0; i<len; i++)
            {
              printf("%d\t", larssonSadakane_sa[i]);
              for (j=larssonSadakane_sa[i]; j<len; j++)
                printf("%d", s[j]);
              printf("\n");
            }
          printf("\n");
          printf("IS-SA's suffix\n");
          for (i=0; i<len; i++)
            {
              printf("%d\t", sa[i]);
              for (j=sa[i]; j<len; j++)
                printf("%d", s[j]);
              printf("\n");
            }
          printf("\n");
        }
      // else
      //   {
      //     fprintf(stderr, "correct\n");
      // fprintf(stderr, "LS-SA\t");
      // displayArray(larssonSadakane_sa, len, 2);
      // fprintf(stderr, "IS-SA\t");
      // displayArray(sa, len, 2);
      // fprintf(stderr, "\n");
      // }
    }
  
  free(sa);
  free(larssonSadakane_sa);
  free(isa);
  
  return 0;
}
