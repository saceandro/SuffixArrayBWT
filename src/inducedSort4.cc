#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

void encode(char* array, int* s, int len)
{
  // 文字列を数値列に変換する
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
  // suffixのtypeを求める
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
  // 文字列に含まれる各文字の数を数え、累積度数を求める。
  int i;
  for (i=0; i<len; i++)
    count[s[i]]++;
  for (i=1; i<types; i++)
    count[i] += count[i-1];
}

int initialize_sa(int* s, int* sa, int* type, int len, int* count, int* cursor, int types)
{
  // suffix arrayを、LMS suffixをセットして初期化する。LMS suffixの数を返す。
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
  // 順序のついたLMS suffixをセットして、suffix arrayを初期化する。
  int i;
  
  for (i=0; i<types; i++)
    cursor[i] = count[i];

  for (i=lmslen-1; i>=0; i--)
    sa[--cursor[s[lms[i]]]] = lms[i];
}

void inducedSort(int* s, int* sa, int* type, int len, int* count, int* cursor, int types)
{
  // 順序のついたLMS suffixがセットされたsuffix arrayを用いて、induced sortを行う。
  int i;

  // カーソルを、各文字の領域の先頭にセットする。
  cursor[0] = 0;
  for (i=1; i<types; i++)
    cursor[i] = count[i-1];

  // L-type suffixをソートする。
  // fprintf(stderr, "phase1:\n");
  i=0;
  while (i<len)
    {
      // suffixがセットされていて0ではなく、かつ一つ前のsuffixがL-typeならば、その文字のカーソル位置にsuffixを挿入し、カーソルを一つ後ろに進める。
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

  // カーソルを、各文字の領域の終端の次にセットする。
  for (i=0; i<types; i++)
    cursor[i] = count[i];

  // S-type suffixをソートする。
  // fprintf(stderr, "phase2:\n");
  i=len-1;
  while (i>=0)
    {
      // suffixがセットされていて0ではなく、かつ一つ前のsuffixがS-type(LMSを含む)ならば、その文字のカーソル位置を一つ前に進め、suffixを挿入する。
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
  // 順序のついていないLMS suffixを、LMS prefixをソートすることによってソートする。
  // すべてのLMS substringに順序がつくまで、再帰的にソートを行う。
  int i,j;
  int* count = (int*)calloc(len, sizeof(int));
  if (count == NULL)
    handle_error("calloc");

  // 文字の累積度数を求める
  occurrence(s, len, count, types);

  int* type = (int*)calloc(len, sizeof(int));
  if (type == NULL)
    handle_error("calloc");

  int* sa = (int*)malloc(len*sizeof(int));
  if (sa == NULL)
    handle_error("malloc");

  // suffix arrayを-1で初期化する。
  memset(sa, -1, len*sizeof(int));

  // suffixのtypeを求める
  lms(s, type, len);

  // fprintf(stderr, "s\t");
  // displayArray(s, len, 2);
  // fprintf(stderr, "t\t");
  // displayArray(type, len, 2);

  int* cursor = (int*)calloc(types, sizeof(int));
  if (cursor == NULL)
    handle_error("calloc");

  // suffix arrayを、順序のついていないLMS suffixで初期化する。
  int lmslen = initialize_sa(s, sa, type, len, count, cursor, types);

  // カーソルを、各文字の領域の先頭にセットする。
  cursor[0] = 0;
  for (i=1; i<types; i++)
    cursor[i] = count[i-1];

  // L-type suffixをソートする。
  // fprintf(stderr, "lms phase1:\n");
  i=0;
  while (i<len)
    {
      // suffixがセットされていて0ではなく、かつ一つ前のsuffixがL-typeならば、その文字のカーソル位置にsuffixを挿入し、カーソルを一つ後ろに進める。
      if (sa[i] != -1 && sa[i] != 0 && type[sa[i]-1] == 2)
        {
          // fprintf(stderr, "now: %d\n", sa[i]);
          
          sa[cursor[s[sa[i]-1]]++] = sa[i]-1;
              
          // fprintf(stderr, "sa: ");
          // displayArray(sa, len, 2);
        }
      i++;
    }
  
  // カーソルを、各文字の領域の終端の次にセットする。
  for (i=0; i<types; i++)
    cursor[i] = count[i];
  
  // S-type suffixをソートする。
  // fprintf(stderr, "lms phase2\n");
  i=len-1;
  while (i>=0)
    {
      // suffixがセットされていて0ではなく、かつ一つ前のsuffixがS-type(LMSを含む)ならば、その文字のカーソル位置を一つ前に進め、suffixを挿入する。
      if (sa[i] != -1 && sa[i] != 0 && type[sa[i]-1] != 2)
        {
          // fprintf(stderr, "now: %d\n", sa[i]);
          
          sa[--cursor[s[sa[i]-1]]] = sa[i]-1;
              
          // fprintf(stderr, "sa: ");
          // displayArray(sa, len, 2);
        }
      i--;
    }

  // LMS substringの順位を格納する配列。
  int* s2 = (int*)calloc(len, sizeof(int));
  if (s2 == NULL)
    handle_error("calloc");
  memset(s2, -1, len*sizeof(int));

  // 末尾はrank=0
  int rank=-1;
  s2[len-1] = ++rank;

  // 前のLMS substringと同順か、それとも下位かを判定する。
  int prevpos = len-1;
  for (i=1; i<len; i++)
    {
      // fprintf(stderr, "sa[%d] = %d\n", i, sa[i]);
      // fprintf(stderr, "type[%d] = %d\n\n", sa[i], type[sa[i]]);
      
      if (type[sa[i]] == 0)
        {
          // LMS substringの頭文字が一致しているとき(していなければ下位)
          if (s[sa[i]] == s[prevpos])
            {
              j=1;
              // 前のLMS substringか現在のLMS substringか、いずれかが終端に達するまで、文字が一致するかどうか見る。一致していなければ、下位である。
              while (type[sa[i]+j]!=0 && type[prevpos+j]!=0 && s[sa[i]+j] == s[prevpos+j])
                j++;
              // ともに終端に達していて、かつLMS substringの末尾の文字が一致していれば、同順である。
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

  // LMS substringの順位を、再帰的にsuffix arrayを構築すべき配列とするため、前に詰める。
  j=0;
  for (i=0; i<len; i++)
    if (s2[i] != -1)
      s2[j++] = s2[i];
  
  int* lmss = (int*) calloc(lmslen, sizeof(int));
  if (lmss == NULL)
    handle_error("calloc");

  // 0~lmslenと、LMS suffixの対応をつける。
  j=0;
  for (i=0; i<len; i++)
    if (type[i] == 0)
      lmss[j++] = i;

  // fprintf(stderr, "lmss: ");
  // displayArray(lmss, lmslen, 2);
  // fprintf(stderr, "s2: ");
  // displayArray(s2, lmslen, 2);

  // ソートされたLMS suffixを格納するための配列。
  int* sortedlms = (int*)calloc(lmslen, sizeof(int));
  if (sortedlms == NULL)
    handle_error("calloc");

  // rank+1 == lmslenならば、すべてのLMS suffixに順序がついたので、そのままソートされたLMS suffix arrayを構築。
  if (rank+1 == lmslen)
    {
      for (i=0; i<lmslen; i++)
        sortedlms[s2[i]] = lmss[i];
      free(s2);
    }
  // そうでなければ、同立のLMS substringがあるので、再帰的に順序をつける。
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

  // 順序のついたLMS suffixをセットして、suffix arrayを初期化する。
  memset(sa, -1, len*sizeof(int));
  initialize_sa_with_lms(s, sortedlms, lmslen, sa, count, cursor, types);

  // induced sortを行い、suffix arrayを構築する。
  inducedSort(s, sa, type, len, count, cursor, types);

  free(s);
  free(type);
  free(count);
  free(cursor);
  
  return sa;
}

int main()
{
  char *array = NULL;
  size_t allocated = 0;
  int len = 0;

  if ((len = (int)getline(&array, &allocated, stdin)) == -1)
    handle_error("getline");
  array[--len] = '\0';
  
  int* s = (int*)calloc(len, sizeof(int));
  if (s == NULL)
    handle_error("calloc");

  // 読み込んだ文字列を、数値列に変換。
  encode(array, s, len);
  
  int* sa = inducedSortLms(s, len, 6);
  
  fprintf(stderr, "final suffix array:\n");
  displayArray(sa, len, 1);

  // int i;
  // for (i=0; i<len; i++)
  //   {
  //     fprintf(stderr, "%d\t", sa[i]);
  //     fprintf(stderr, "%s\n", array+sa[i]);
  //   }

  free(array);
  free(sa);
  
  return 0;
}
