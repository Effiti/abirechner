#include "lib.h"
#define BIS(i) ((char *)base + (i) * size)
#define BISCMP(x, y) cmp(BIS(x), BIS(y))
char *strcpy(char *restrict dst, const char *restrict src) {
  while ((*dst++ = *src++))
    ;
  return --dst;
}

unsigned int bump_pointer = (unsigned int)&__heap_base;
void *malloc(unsigned long n) {
  unsigned int r = bump_pointer;
  bump_pointer += n;
  return (void *)r;
}

void drop_all_mem() {
  bump_pointer = (unsigned int)&__heap_base;
}

unsigned long strlen(const char *str) {
    size_t length = 0;
    while (*str++) {
        length++;
    }
    return length;
}

char *h(char*s) {
  char * ptr = malloc(strlen(s)+1);
  strcpy(ptr, s);
  return ptr;
}
void free(void *p) {
  // lol
}

static void inline st_swap(void *a, void *b, void *t, size_t size) {
  char temp;
  char *pa = (char *)a;
  char *pb = (char *)b;
  for (size_t i = 0; i < size; i++) {
    temp = pa[i];
    pa[i] = pb[i];
    pb[i] = temp;
  }
}

static int st_partition(void *base, int low, int high, size_t size,
                        compar cmp) {
  char t[size];
  int pivot = high;
  // Index of smaller element and indicates the right position of pivot found so
  // far
  int i = (low - 1);
  for (int j = low; j < high; j++) {
    // If current element is smaller than the pivot
    if (BISCMP(j, pivot) < 0) {
      // increment index of smaller element
      i++;
      st_swap(BIS(i), BIS(j), t, size);
    }
  }
  st_swap(BIS(i + 1), BIS(high), t, size);
  return i + 1;
}
static void st_inner(void *base, int low, int high, size_t size, compar cmp) {
  if (low < high) {
    int pi = st_partition(base, low, high, size, cmp);
    st_inner(base, low, pi - 1, size, cmp);
    st_inner(base, pi + 1, high, size, cmp);
  }
}
void qsort(void *base, size_t num, size_t size, compar cmp) {
  st_inner(base, 0, num - 1, size, cmp);
}
int cmp_ptr(const void *a, const void *b) {
  const int **left = (const int **)a;
  const int **right = (const int **)b;

  return (**left < **right) - (**right < **left);
}

u8 *order_int(const u8 *a, u8 n) {
  const u8 **pointers = malloc(n * sizeof(const u8 *));

  for (u8 i = 0; i < n; i++)
    pointers[i] = a + i;

  qsort(pointers, n, sizeof(const int *), cmp_ptr);

  size_t *indices = malloc(n * sizeof(size_t));

  for (size_t i = 0; i < n; i++)
    indices[i] = pointers[i] - a;

  free(pointers);

  return indices;
}
