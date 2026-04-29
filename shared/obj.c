#include "obj.h"
#include "string.h"

int next_line(const char *buf, char *line) {
    const char *p = buf;
    int size = 0;
    while (*p != '\n' && *p != '\0') {
        line[size] = *p;
        p++;
        size++;
    }

    line[size] = '\0';

    if (*p == '\n')
        return size + 1;
    return size;
}

void parse_obj(const char *buf, obj_vertex_t *vertices, int *vertex_count, obj_face_t *faces, int *face_count) {
    char temp[32];
    char line[128];
    const char *p = buf;

    while (*p != '\0') {
        int consumed = next_line(p, line);
        if (consumed == 0)
            break;

        if (line[0] == 'v' && line[1] == ' ') {
            strtok(temp, line, ' ');

            strtok(temp, NULL, ' ');
            vertices[*vertex_count].x = stof(temp);

            strtok(temp, NULL, ' ');
            vertices[*vertex_count].y = stof(temp);

            strtok(temp, NULL, ' ');
            vertices[*vertex_count].z = stof(temp);

            (*vertex_count)++;

        } else if (line[0] == 'f' && line[1] == ' ') {
            strtok(temp, line, ' ');

            strtok(temp, NULL, ' ');
            faces[*face_count].a = stoi(temp) - 1;

            strtok(temp, NULL, ' ');
            faces[*face_count].b = stoi(temp) - 1;

            strtok(temp, NULL, ' ');
            faces[*face_count].c = stoi(temp) - 1;

            (*face_count)++;
        }

        p += consumed;
    }
}
