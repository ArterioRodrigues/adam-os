#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define NULL 0
#define bool uint8_t
#define true 1
#define false 0

#endif

#ifndef STDARG_H
#define STDARG_H

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

#endif
