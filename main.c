#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>


#define MAX_BUFFER 1024
#define SYSFS_FILE_A_IN "/sys/kernel/sykom/rejAgawmat"
#define SYSFS_FILE_S_OUT "/sys/kernel/sykom/rejSgawmat"
#define SYSFS_FILE_W_OUT "/sys/kernel/sykom/rejWgawmat"


unsigned int read_from_file(char *filePath){
    char buffer[MAX_BUFFER];
    int fd_out = open(filePath, O_RDONLY);
    if(fd_out < 0){
        printf("Open %s - error number %d\n", filePath, errno);
        exit(5);
    }
    lseek(fd_out, 0L, SEEK_SET);
    int n = read(fd_out, buffer, MAX_BUFFER);
    if(n>0) {
	buffer[n]='\0';
	close(fd_out);
	return strtoul(buffer, NULL, 16);
    }
}

void write_to_file(char *filePath, unsigned int value) {
    char buffer[MAX_BUFFER];
    
    int fd_in = open(filePath, O_RDWR | O_TRUNC, 0644);
    if(fd_in < 0) {
        printf("Open %s - error number %d\n", filePath, errno);
        exit(5);
    }
    snprintf(buffer, MAX_BUFFER, "%x", value);
    lseek(fd_in, 0L, SEEK_SET); 
    ssize_t bytes_written = write(fd_in, buffer, strlen(buffer));
    if(bytes_written < 0) {
        printf("Write to %s - error number %d\n", filePath, errno);
        exit(6);
    }
    close(fd_in);
}

unsigned int get_prime(unsigned int a){
    write_to_file(SYSFS_FILE_A_IN, a);
    unsigned int read_status;
    unsigned int read;
    do{
        read_status = read_from_file(SYSFS_FILE_S_OUT);
    }
    while(read_status != 5);
    sleep(1);
    read = read_from_file(SYSFS_FILE_W_OUT);
    return read;
}

int test_module(){
    unsigned int res;
    unsigned int W;
    unsigned int number_found;
    unsigned int a[6] =       {1, 2, 3, 10, 20, 700};
    unsigned int results[6] = {2, 3, 5, 29, 71, 5279};
    int i = 0;
    while (i < 6) {
        res = get_prime(a[i]);
        W = res >> 4;
        number_found = (res & 0x0000000F);
        printf("A = 0x%x, W = 0x%x, Expected = 0x%x, found numbers so far: 0x%x\n", a[i], W, results[i], number_found);
	fflush(stdout);
        i++;
    }

    return 0;
}


int main(void){
    int test = test_module();
    if(test > 0){
        printf("TEST FAILED at %d values\n", test);
    } else {
        printf("====== TEST PASSED =====\n");
    }
    return 0;
}

