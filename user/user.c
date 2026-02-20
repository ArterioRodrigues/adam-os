#include "../pch.h"

void process1() {
    char msg[] = "1";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process2() {
    char msg[] = "2";
    while (1) {

        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process3() {
    char msg[] = "3";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process4() {
    char msg[] = "4";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process5() {
    char msg[] = "5";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process6() {
    char msg[] = "6";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process7() {
    char msg[] = "7";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}
void process8() {
    char msg[] = "8";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}
void process9() {
    char msg[] = "9";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}

void process10() {
    char msg[] = "0";
    while (1) {
        int i = 0;
        int j = 0;
        while (i < 10000) {
            while (j < 10000)
                j++;
            i++;
            j = 0;
        }
        sys_write(msg, 1);
    }
}
