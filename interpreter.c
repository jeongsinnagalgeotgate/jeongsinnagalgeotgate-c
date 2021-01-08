#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

typedef void(*voidfuncptr_t)();
typedef struct __parsed_data_t {
    voidfuncptr_t *fs;
    int *ltofpos;
} parsed_data_t;

void read_options(int argc, char **argv);
FILE* validate_file(char *path);
wchar_t* read_file(FILE *fp);
parsed_data_t* parse(wchar_t *str);
void run();
void __read();
void __write();
void __push();
void __pop();
void __enqueue();
void __dequeue();
void __move_line();
void __next_line();
void __tmp_inc();
void __tmp_dec();
void __tmp_reset();

parsed_data_t *data = NULL;
char *path = NULL;
int   tmp = 0;
int   pos = 0;
int   line = 0;
int  *__stack = NULL;
int  *__queue = NULL;
int   __stack_top = 0;
int   __queue_rear = 0;
int   __queue_front = 0;
int   __stack_size = 0;
int   __queue_size = 0;

#ifdef DEBUG
int debugfd;
#endif

void debug(char* s) {
#ifdef DEBUG
    printf("%s", s);
#endif
}

int main(int argc, char** argv) {

    char *path = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp("-s", argv[i])) {
            __stack_size = atoi(argv[++i]);
        } else if (!strcmp("-q", argv[i])) {
            __queue_size = atoi(argv[++i]);
        } else {
            path = argv[i];
        }
    }

    if (path == NULL) {
        fprintf(stderr, "파일안줘서정신나갈것같애\n");
        exit(1);
    }

    __stack_size = __stack_size ? __stack_size : 256;
    __queue_size = __queue_size ? __queue_size + 1 : 256 + 1;
    __stack = calloc(__stack_size, sizeof(int));
    __queue = calloc(__queue_size, sizeof(int));

#ifdef DEBUG
    printf("Using stack size: %d, queue size: %d\n", __stack_size, __queue_size);

    printf("v %p\n", __push);
    printf("^ %p\n", __pop);
    printf("< %p\n", __enqueue);
    printf("> %p\n", __dequeue);
    printf("l %p\n", __move_line);
    printf("\\ %p\n", __next_line);
    printf("+ %p\n", __tmp_inc);
    printf("- %p\n", __tmp_dec);
    printf(". %p\n", __tmp_reset);
#endif

    setlocale(LC_ALL, "");

    FILE *fp = validate_file(path);
    wchar_t *str = read_file(fp);

    fclose(fp);

#ifdef DEBUG
    puts("");
    for (int i = 0; ~str[i];) {
        printf("%d ", str[i++]);
    }

    printf("-1\n");
#endif

    data = parse(str);

#ifdef DEBUG
    for (int i=0;data->fs[i] != NULL;i++) {
        printf("%p ", data->fs[i]);
    }
    puts("");
    for (int i=0;data->ltofpos[i] || !i;i++) {
        printf("%d ", data->ltofpos[i]);
    }
    puts("");
#endif

    run();

    return 0;
}

void __write() {
#ifdef DEBUG
    printf("write %d -> ", tmp);
#endif
    if (tmp > 127)
        printf("%c%c%c", 0xe0 + ((tmp & 0xf000) >> 12), 0x80 + ((tmp & 0x0fc0) >> 6), 0x80 + (tmp & 0x003f));
    else
        printf("%c", tmp);
    fflush(stdout);
}

void __read() {
#ifdef DEBUG
    printf("read");
#endif
    tmp = fgetwc(stdin);
}

void __push() {
#ifdef DEBUG
    printf("push");
#endif
    __stack[__stack_top++] = tmp;
}

void __pop() {
#ifdef DEBUG
    printf("pop");
#endif
    tmp = __stack[--__stack_top];
}

void __enqueue() {
#ifdef DEBUG
    printf("enqueue");
#endif
    __queue[__queue_rear++] = tmp;
    __queue_rear %= __queue_size;
}

void __dequeue() {
#ifdef DEBUG
    printf("dequeue");
#endif
    tmp = __queue[__queue_front++];
    __queue_front %= __queue_size;
}

void __tmp_inc() {
#ifdef DEBUG
    printf("inc");
#endif
    tmp++;
}

void __tmp_dec() {
#ifdef DEBUG
    printf("dec");
#endif
    tmp--;
}

void __tmp_reset() {
#ifdef DEBUG
    printf("reset");
#endif
    tmp = 0;
}

void __next_line() {
#ifdef DEBUG
    printf("next line");
#endif
    line++;
}

void __move_line() {
    pos = data->ltofpos[line += __stack[--__stack_top]];
#ifdef DEBUG
    printf("move line %d\n", line);
#endif
}

FILE* validate_file(char *path) {

    FILE *fp = NULL;

    if ((fp = fopen(path, "rt")) == NULL) {
        fprintf(stderr, "파일이없어정신나갈것같애\n");
        exit(1);
    }

    return fp;
}

wchar_t* read_file(FILE *fp) {
    wchar_t *wcs = calloc(512, sizeof(wchar_t));
    long long idx = 0, alloced = 512;
    
    errno = 0;
    while (WEOF != (wcs[idx++] = fgetwc(fp))) {
        if (idx >= alloced) {
            wcs = (wchar_t *)realloc(wcs, (alloced += 512) * sizeof(wchar_t));
        }
    }

    if (errno == EILSEQ) {
        fprintf(stderr, "파일읽는데정신나갈것같애: got illegal sequence (EILSEQ) while reading file.\n");
        exit(1);
    }

    return wcs;
}

parsed_data_t* parse(wchar_t *str) {

    int i = 0;
    int line = 0;
    int pos = 0;
    int tmp = 0;
    parsed_data_t *data = calloc(1, sizeof(parsed_data_t));
    data->fs = calloc(512, sizeof(voidfuncptr_t));
    data->ltofpos = calloc(512, sizeof(int));

    data->ltofpos[0] = 0;

    while (~str[i] || tmp > 0) {
#ifdef DEBUG
        printf("i = %6d, str[i] = %2lc, tmp = %07x, line = %3d, pos = %4d\n", 
                i, str[i] == 10 ? '\\' : str[i], tmp, line, pos);
#endif
        if (!wcsncmp(str+i, L"정신", 2) && (0xf & tmp) == 0) {
            tmp |= 0x1000000;
            i += 2;
        } else if (!wcsncmp(str+i, L"점심", 2) && (0xf & tmp) == 0) {
            tmp |= 0x2000000;
            i += 2;
        } else if (!wcsncmp(str+i, L"나갈", 2) && (0xf & tmp) == 1) {
            tmp |= 0x0100000;
            i += 2;
        } else if (!wcsncmp(str+i, L"나가서먹을", 5) && (0xf & tmp) == 1) {
            tmp |= 0x0200000;
            i += 5;
        } else if (!wcsncmp(str+i, L"것", 1) && (0xf & tmp) == 2) {
            tmp |= 0x0010000;
            i += 1;
        } else if (!wcsncmp(str+i, L"거", 1) && (0xf & tmp) == 2) {
            tmp |= 0x0020000;
            i += 1;
        } else if (!wcsncmp(str+i, L"같애", 2) && (0xf & tmp) == 3) {
            tmp |= 0x0001000;
            i += 2;
        } else if (!wcsncmp(str+i, L"같아", 2) && (0xf & tmp) == 3) {
            tmp |= 0x0002000;
            i += 2;
        } else if (!wcsncmp(str+i, L"?", 1) && (0xf & tmp) >= 4 && !(0x0000f00 & tmp)) {
            tmp |= (tmp & 0xf) << 8;
            i += 1;
        } else if (!wcsncmp(str+i, L".", 1) && (0xf & tmp) >= 4 && !(0x00000f0 & tmp)) {
            tmp |= (tmp & 0xf) << 4;
            i += 1;
        } else if ((0xf & tmp) >= 4) {
            // switch-case
            switch (tmp >> 16) {
                case 0x111:
                    data->fs[pos++] = __push;
                    break;
                case 0x121:
                    data->fs[pos++] = __pop;
                    break;
                case 0x211:
                    data->fs[pos++] = __enqueue;
                    break;
                case 0x221:
                    data->fs[pos++] = __dequeue;
                    break;
                case 0x112:
                    data->fs[pos++] = __read;
                    break;
                case 0x122:
                    data->fs[pos++] = __write;
                    break;
                case 0x212:
                    data->fs[pos++] = __tmp_dec;
                    break;
                case 0x222:
                    data->fs[pos++] = __tmp_inc;
                    break;
                default:
                    fprintf(stderr, "E: Invalid syntax at %d: %lc.\n", i, str[i]);
                    exit(1);
            }

            if ((0x000f000 & tmp) >> 12 == 2)
                data->fs[pos++] = __move_line;

            int t = (tmp >> 4) & 0x0000ff;
            
            if ((t & 0xf0) == 4);
            else if ((t & 0x0f) == 4) {
                data->fs[pos++] = __tmp_reset;
            }

            if ((t & 0xf0) == 5);
            else if ((t & 0x0f) == 5) {
                data->fs[pos++] = __tmp_reset;
            }

            if (!wcsncmp(str+i, L"\n", 1)) {
                data->fs[pos++] = __next_line;
                data->ltofpos[++line] = pos;
                i++;
            }

            tmp = -1;
        } else {
            fprintf(stderr, "E: Invalid syntax at %d: %lc.\n", i, str[i]);
            exit(1);
        }
        tmp++;
    }

    data->fs[pos] = NULL;

    return data;
}

void run() {
    for (;data->fs[pos] != NULL;) {
        (*data->fs[pos++])();
#ifdef DEBUG
        printf("\n%d\nstack", pos);
        for (int i=0;i<__stack_top;i++) {
            printf(" %d", __stack[i]);
        }
        printf("\nqueue");

        if (__queue_front <= __queue_rear)
            for (int i=__queue_front;i<__queue_rear;i++) {
                printf(" %d", __queue[i]);
            }
        else
            for (int i=__queue_front;i<__queue_rear + __queue_size;i++) {
                printf(" %d", __queue[(i + __queue_size) % __queue_size]);
            }
        printf("\ntmp %d\n\n", tmp);
#endif
    }
}
