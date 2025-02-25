#include "lib.h"

const char* NAMES_NAT = "M\0PH\0CH\0IF\0";

typedef struct {
  void* start;
  void* end;
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

void log_type(u8 type) {
  switch (type) {
  case 0b001:
    console_log("Q1");
    break;
  case 0b010:
    console_log("Q2");
    break;
  case 0b011:
    console_log("Q1Q2");
    break;
  case 0b100:
    console_log("BLL");
    break;
  case 0b101:
    console_log("A3");
    break;
  case 0b110:
    console_log("A4");
    break;
  case 0b111:
    console_log("LK");
    break;
  }
}

// returns an 'added' style u8
u8 n_max_point_indices(Course course, u8 n) {
  u8 ret = 0;
  u8* indices = order_int(course.data, LENS[course.type]);
  for(u8 i = 0; i<=n; i++) {
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
  if(namecmp(course.name, "KR") || namecmp(course.name, "ER"))
    return n_max_point_indices(course, 2);
  if(namecmp(course.name, "D")) 
    return 0b00001111;
return 0;
}



Span construct_courses(u8 *p) {
  log_num((int) *p);
  short total_len = *((unsigned short *)p);
  malloc(total_len + 3);
  p += sizeof(short);
  focus = *p;
  p += sizeof(u8);
  //console_log("total_len:");
  log_num((int)total_len);
  Span* span = malloc(sizeof(Span));
  u8 *ptr = p;
  span->start = p;

  while ((ptr - p) < total_len) {
    Course *course = malloc(sizeof(Course));
    Type type;

    type = *ptr;
    ptr++;
    course->name = (char *)ptr;
    ptr += 2;
    console_log((char *)course->name);

    course->data = ptr;
    course->data_len = LENS[type];
    course->type = type;
    course->added = 0;
    ptr += LENS[type];
    log_type(type);
    log_num(LENS[type]);
    console_log("...");
  }

  span->end = ptr;
  return *span;
}

void tag_courses(Span span) {
  u8* ptr = span.start;
  while((int)ptr-(int)span.end>=0) {
    
  }
  
}

unsigned short calculate_from_courses(Span span) {
  
  return 0;
}


unsigned short compute(u8 *p) {
  Span span = construct_courses(p);
  return calculate_from_courses(span);
}
