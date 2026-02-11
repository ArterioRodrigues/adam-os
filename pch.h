#ifndef PCH_H
#define PCH_H

#include "cpu/idt.h"
#include "cpu/exceptions.h"
#include "cpu/syscall.h"
#include "drivers/keyboard.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "kernel/config.h"
#include "kernel/frame.h"
#include "kernel/kmalloc.h"
#include "kernel/types.h"
#include "kernel/user.h"
#include "kernel/page-table.h"
#include "lib/math.h"
#include "lib/ramfs.h"
#include "lib/shell.h"
#include "lib/mem.h"
#include "lib/string.h"

#endif
