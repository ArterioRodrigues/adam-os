#include "../lib/lib.h"
#include "../lib/string.h"


void main() {
    print("Adam OS\n");
    sys_exec("shell");
    print("Error: exec failed");

    int fd = sys_open("/");
    char buf[100];
    int size = sys_read(fd, buf, 100);
    sys_write(0, buf, size);

    // int fd = sys_open("docs");
    // while(1);
    // uint8_t buf[96];
    // int size = sys_read(fd, buf, 96);

    // fat16_entry_t entry[10];
    // int index = 0;

    // for (int i = 0; i < 96; i += sizeof(fat16_entry_t)) {
    //     entry[index].name[0] = buf[i];
    //     entry[index].name[1] = buf[i + 1];
    //     entry[index].name[2] = buf[i + 2];
    //     entry[index].name[3] = buf[i + 3];
    //     entry[index].name[4] = buf[i + 4];
    //     entry[index].name[5] = buf[i + 5];
    //     entry[index].name[6] = buf[i + 6];
    //     entry[index].name[7] = buf[i + 7];

    //    entry[index].extension[0] = buf[i + 8];
    //    entry[index].extension[1] = buf[i + 9];
    //    entry[index].extension[2] = buf[i + 10];

    //    entry[index].attributes = buf[i + 11];

    //    entry[index].start_cluster = (uint16_t)buf[i + 25];
    //    entry[index].file_size = (uint32_t)buf[i + 27];

    //    index++;
    //}
    //
    // print(entry[0].name);
    // print(entry[1].name);
    // print(entry[2].name);
    while (1)
        ;
}
