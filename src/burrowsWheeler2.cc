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
      // fprintf(stderr, "sa[%d] = %d\n", i, sa[i]);
      // fprintf(stderr, "type[%d] = %d\n\n", sa[i], type[sa[i]]);
      
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

void burrowsWheelerTransform(int* s, int* sa, int* bwt, int len)
{
  // Construct the Burrows Wheeler Transform using suffix array.
  int i;
  for (i=0; i<len; i++)
    bwt[i] = s[(sa[i]+len-1)%len];
}

void countBWT(int* bwt, int* occ, int len, int* count)
{
  // Calculate the occurrence and the cumulative frequency.
  int i,j;
  occ[bwt[0]-1] = 1;
  for (i=1; i<len; i++)
    for (j=0; j<4; j++)
      occ[i*4 + j] = occ[(i-1)*4 + j] + (bwt[i] == j+1);

  count[0] = 0;
  count[1] = 1;
  for (j=1; j<4; j++)
    count[j+1] = count[j] + occ[(len-1)*4 + j-1];
}

int lb(int* q, int querylen, int* occ, int len, int* count)
{
  int lb = count[q[querylen-1]];

  for (int i=querylen-2; i>=0; i--)
    lb = count[q[i]] + occ[(lb-1)*4 + (q[i]-1)];
  
  return lb;
}

int ub(int* q, int querylen, int* occ, int len, int* count)
{
  int ub = len-1;

  for (int i=querylen-1; i>=0; i--)
    ub = count[q[i]] + occ[ub*4 + (q[i]-1)] - 1;
  
  return ub;
}

// int lb(int* q, int* occ, int len, int* count)
// {
//   if (*q == 0)
//     return 0;
//   else if (lb(q+1, occ, len, count) == 0)
//     return count[*q];
//   else
//     return count[*q] + occ[(lb(q+1, occ, len, count)-1)*4 + (*q-1)];
// }

// int ub(int* q, int* occ, int len, int* count)
// {
//   if (*q == 0)
//     return len-1;
//   else
//     return count[*q] + occ[ub(q+1, occ, len, count)*4 + (*q-1)] - 1;
// }

int main()
{
  char *array = NULL;
  size_t allocated = 0;
  int len = 0;

  if ((len = (int)getline(&array, &allocated, stdin)) == -1)
    handle_error("getline");
  array[--len] = '\0';

  char *query = NULL;
  size_t query_allocated = 0;
  int querylen = 0;
  if ((querylen = (int)getline(&query, &query_allocated, stdin)) == -1)
    handle_error("getline");
  query[--querylen] = '\0';
  
  int* s = (int*)calloc(len, sizeof(int));
  if (s == NULL)
    handle_error("calloc");
  
  encode(array, s, len);
  
  int* sa = inducedSortLms(s, len, 6);
  
  fprintf(stderr, "final suffix array:\n");
  displayArray(sa, len, 2);

  int i, j;
  for (i=0; i<len; i++)
    {
      fprintf(stderr, "%d\t", sa[i]);
      fprintf(stderr, "%s\n", array+sa[i]);
    }

  s = (int*)calloc(len, sizeof(int));
  if (s == NULL)
    handle_error("calloc");
  
  encode(array, s, len);

  int* bwt = (int*)calloc(len, sizeof(int));
  if (bwt == NULL)
    handle_error("calloc");
  
  burrowsWheelerTransform(s, sa, bwt, len);
  
  fprintf(stderr, "bwt\t");
  displayArray(bwt, len, 2);

  int* occ = (int*)calloc(len*4, sizeof(int));
  if (occ == NULL)
    handle_error("calloc");

  fprintf(stderr, "occ\n");
  for (i=0; i<len; i++)
    {
      for (j=0; j<4; j++)
        fprintf(stderr, "%d ", occ[i*4 + j]);
      fprintf(stderr, "\n");
    }

  int* count = (int*)calloc(5, sizeof(int));
  if (count == NULL)
    handle_error("calloc");

  countBWT(bwt, occ, len, count);

  fprintf(stderr, "occ\n");
  for (i=0; i<len; i++)
    {
      for (j=0; j<4; j++)
        fprintf(stderr, "%d ", occ[i*4 + j]);
      fprintf(stderr, "\n");
    }
          
  fprintf(stderr, "count\t");
  displayArray(count, 5, 2);
  
  fprintf(stderr, "query\t");
  fprintf(stderr, "%s\n", query);

  int* encoded_query = (int*)calloc(querylen, sizeof(int));
  if (s == NULL)
    handle_error("calloc");
  
  encode(query, encoded_query, querylen);

  fprintf(stderr, "encoded_query\n");
  displayArray(encoded_query, querylen, 2);

  fprintf(stderr, "lb:%d ub:%d\n", lb(encoded_query, querylen, occ, len, count), ub(encoded_query, querylen, occ, len, count));
  // free(array);
  // free(sa);

  fprintf(stderr, "answer\t");
  for (i=lb(encoded_query, querylen, occ, len, count); i<=ub(encoded_query, querylen, occ, len, count); i++)
    printf("%d ", sa[i]);
  printf("\n");
  
  return 0;
}
