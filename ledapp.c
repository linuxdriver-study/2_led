#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
        int ret = 0;
        int fd;
        unsigned char buf[1] = {0};
        char *filename = NULL;

        if (argc != 3) {
                printf("Error Usage!\n"
                       "Usage %s filename 0:1\n"
                       "0:close led\n"
                       "1:open led\n"
                       ,argv[0]);
                ret = -1;
                goto error;
        }

        filename = argv[1];
        fd = open(filename, O_RDWR);
        if (fd == -1) {
                perror("open failed!\n");
                ret = -1;
                goto error;
        }

        buf[0] = atoi(argv[2]);
        ret = write(fd, buf, 1);
        if (ret < 0) {
                perror("write error");
                goto error;
        }

error:
        close(fd);
        return ret;
}
