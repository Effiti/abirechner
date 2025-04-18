#include "lib.h"

const char *NAMES_NAT = "M\0PH\0CH\0IF\0";

typedef struct {
  void *start;
  void *end;
} Span;

typedef enum { Q1 = (u8)1, Q2 = 2, Q1Q2 = 3, BLL, A3, A4, LK } Type;

typedef enum { LAN = (u8)0, SOC, NAT } Focus;
Focus focus;

const u8 LENS[8] = {
    0, // NONE
    2, // Q1
    2, // Q2,
    4, // Q1Q2
    1, // BLL
    5, // A3
    5, // A4
    5, // LK
};

typedef struct {
  Type type;
  u8 data_len;
  u8 *data;   // points to "argument space"
  u8 added;   // which parts were added: first 3 bits: 0; last 5 bits: bitmap of
              // wether that data point is added;
  char *name; // points to arg space. We could inline the fixed-len name field
              // like so `char name[3];`, but this way we spare an explicit copy
              // and only have to copy for logging (i.e when we need the string
              // to be null terminated)
} Course;

typedef struct {
  Course *ptr;
  u8 n;
} Courses;
// compare a null terminated str, b to a name, a
u8 namecmp(char *a, char *b) {
  u8 i = 0;
  while (b[i] != 0 && i <= 2) {
    if (a[i] != b[i])
      return 0;
    i++;
  }
  return *b == *a;
}

void log_name(char *name) {
  const char n[3] = {name[0], name[1],
                     0}; // because we need the name-Str to be null terminated,
                         // we actually have to create a stack-variable;
  console_log((char *)n);
}

// returns an 'added' style u8
u8 n_max_point_indices(Course course, u8 n) {
  u8 ret = 0;
  u8 *indices = order_int(course.data, LENS[course.type]);
  for (u8 i = 0; i <= n; i++) {
    ret |= 1 << (indices[i]);
  }
  return ret;
}

u8 required_add(Course course) {
  if (course.type == LK)
    return 0b00011111;
  if (course.type == BLL)
    return 0b00010000;
  if (course.type == A3)
    return 0b00011111;
  if (course.type == A4)
    return 0b00011111;
  if (namecmp(course.name, "KR") || namecmp(course.name, "ER"))
    return n_max_point_indices(course, 2);
  if (namecmp(course.name, "D"))
    return 0b00001111;
  return 0;
}

Courses *construct_courses(u8 *p) {
  log_num((unsigned short)*p);
  short total_len = *((unsigned short *)p);
  malloc(total_len + 3);
  p += sizeof(short);
  focus = *p;
  p += sizeof(u8);
  log_num((int)total_len);
  Courses *header = malloc(sizeof(Courses));
  u8 *ptr = p;

  int i = 0;
  while ((ptr - p) < total_len) {
    i++;
    Course *course = malloc(sizeof(Course));
    Type type;


    type = *ptr;
    ptr++;
    course->name = (char *)ptr;
    ptr += 2;
    //log_name((char *)course->name);

    course->data = ptr;
    course->data_len = LENS[type];
    course->type = type;
    course->added = 0;
    ptr += LENS[type];
    log_num(LENS[type]);
  }
  header->n = i;

  return (header);
}

u8 get_sum(Course tagged) {
  u8 sum = 0;
  log_num(tagged.data_len);
  for (u8 i = 0; i < tagged.data_len; i++) {
    log_num(tagged.data[i]);
    sum += tagged.data[i];
  }
  if (tagged.type == LK)
    return sum * 2;
  return sum;
}

void tag_courses(Course *data, u8 n) {
  
}

unsigned short calculate_from_courses(Course *courses, u8 n) {
  unsigned short sum = 0;
  for (u8 i = 0; i < n; i++) {
    log_num(1000+i);
    sum += get_sum(courses[i]);

  }
  return sum;
}

unsigned short compute(u8 *p) {
  for (int i = 0; i<15; i++) {
      log_num((u8) *(p+i));
  }
  Courses *course = construct_courses(p);
  u8 n = course->n;
  log_num(177);
  tag_courses(course->ptr, n);;;
  drop_all_mem();
  return calculate_from_courses(course->ptr, n);

}

