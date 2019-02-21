#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
using namespace std;

#define handle_error(msg)                               \
  do {perror(msg); exit(EXIT_FAILURE);} while(0)

int char_to_value(char a)
{
  // 文字を数値に変換する
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

void displayArray(int *array, int n)
{
  int i;
  for (i=0; i<n; i++)
    printf("%d ", array[i]);
  printf("\n");
}

void bucketSort(char* s, int* sa, int* isa, int len)
{
  // Construct first suffix array and inverse suffix array by bucket sort.
  // All elements are assumed to be nonnegative integers.
  
  int i=0;
  // Array for counting numbers of the character.
  int count[5] = {0,0,0,0,0};
  
  for(i=0; i<len; i++)
    count[char_to_value(s[i])]++;
  
  // Count the number of elements <= i.
  for(i=1; i<5; i++)
    count[i] += count[i-1];

  // Construct inverse suffix array at this stage.
  for(i=0; i<len; i++)
    isa[i] = count[char_to_value(s[i])] - 1;
  
  // Scan the input array from the tail, and
  // insert each element in the proper position.
  for(i=len-1; i>=0; i--)
    sa[ --count[char_to_value(s[i])] ] = i;
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
      // Move from the left until hitting a value larger than the pivot.(order by isa[sa[i]+depth])
      for(i++; isa[sa[i]+depth] <= pivot && i <= j && i<right; i++)
        {
          if (isa[sa[i]+depth]==pivot)
            {
              tmp = sa[i];
              sa[i] = sa[++mi];
              sa[mi] = tmp;
            }
        }

      // Move from the right until hitting a value smaller than the pivot.(order by isa[sa[i]+depth])
      for(j--; pivot <= isa[sa[j]+depth] && i <= j; j--)
        {
          if (isa[sa[j]+depth]==pivot)
            {
              tmp = sa[j];
              sa[j] = sa[--mj];
              sa[mj] = tmp;
            }
        }
      // Not equal any more.
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

      // Update inverse suffix array after smaller subarray is sorted.
      for (i=newleftright[0]+1; i<newleftright[1]; i++)
        {
          // fprintf(stderr, "isa[sa[%d]] = %d\n", i, newleftright[1]-1);
          isa[sa[i]] = newleftright[1]-1;
        }
      
      left = newleftright[1];
    }
  // If left == right, its order is already determined. Update inverse suffix array.
  if (left == right)
    {
      // fprintf(stderr, "isa[sa[%d]] = %d\n\n", left, left);
      isa[sa[left]] = left;
    }
}

int* larssonSadakane(char* s, int* sa, int* isa, int len)
{
  // Construct the suffix array by Larsson-Sadakane Algorithm.
  
  // Construct the first suffix array and inverse suffix array by bucket sort.
  bucketSort(s, sa, isa, len);

  // displayArray(sa, len);
  // displayArray(isa, len);

  // Until all the suffixes are ordered, repeat ternaryQsort between i and isa[sa[i]] and double the length(denoted as depth).
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

int main()
{
  int i;
  char *s = NULL;
  size_t allocated = 0;
  int len = 0;

  srand(time(NULL));
  
  if ((len = (int)getline(&s, &allocated, stdin)) == -1)
    handle_error("getline");
  s[--len] = '\0';
  
  int* sa = (int*)calloc(len, sizeof(int));
  if (sa == NULL)
    {
      free(s);
      handle_error("calloc");
    }
  
  int* isa = (int*)calloc(len, sizeof(int));
  if (isa == NULL)
    {
      free(s);
      free(sa);
      handle_error("calloc");
    }

  larssonSadakane(s, sa, isa, len);

  printf("suffix array:\n");
  displayArray(sa, len);
  printf("inverse suffix array:\n");
  displayArray(isa, len);

  printf("LS's suffix\n");
  for (i=0; i<len; i++)
    {
      printf("%d\t", sa[i]);
      printf("%s\n", s+sa[i]);
      // for (j=sa[i]; j<len; j++)
      //   printf("%d", s[j]);
      // printf("\n");
    }
  printf("\n");
  
  free(s);
  free(sa);
  free(isa);
  
  return 0;
}
