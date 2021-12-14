/*
 * 將 simple.c write 的 mutex 去掉, 運行這支程式 (當然, 先改成 ans.c 用 qemu 跑)
 * 流程:
 *        main thread: create 3 thread
 *        thread1: 不斷讀, 並判別讀到的是否全是 2 或 全是 3
 *        thread2: 不斷寫 2 進去
 *        thread3: 不斷寫 3 進去
 * 
 * 結果:
 *        去掉 mutex 會發現 2 跟 3 有機會同時出現
 *        加了 mutex 就要麼全是 2 要麼全是 3
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

const int cnt = 0x100;

void* child1(void* data) {
   char buf[cnt];
   while (1) {
        read(3, buf, 0x100);
        printf("child1: %s\n", buf); 
        int all2 = 1, all3 = 1;
        for (int i = 0; i<cnt; i++)
            if (buf[i] != '2') {
                all2 = 0;
                break;
            }
        for (int i = 0; i<cnt; i++)
            if (buf[i] != '3') {
                all3 = 0;
                break;
            }
        if (!all2 && !all3 && strlen(buf))
            break;
    }
  
    pthread_exit(NULL);
}

void *child2 (void* data) {
    char buf[cnt];
    for (int i = 0; i<cnt; i++)
        buf[i] = '2'; 

    while (1) {
        __asm__ volatile(
                "mov rdi, 3;"
                "mov rsi, %0;"
                "mov edx, dword ptr [%1];"
                "mov rax, 1;"
                "syscall;"
                ::"r"(buf), "r"(&cnt):"rax", "rdi", "rsi", "rdx"
       );
    }

    pthread_exit(NULL);
}

void *child3 (void* data) {
    char buf[cnt];
    for (int i = 0;i<cnt; i++)
        buf[i] = '3';

    while (1) {
        __asm__ volatile(
                "mov rdi, 3;"
                "mov rsi, %0;"
                "mov edx, dword ptr [%1];"
                "mov rax, 1;"
                "syscall;"
                ::"r"(buf), "r"(&cnt):"rax", "rdi", "rsi", "rdx"
       );
    }
    pthread_exit(NULL);
}


int main() {
    pthread_t t1, t2, t3;
    
    open("/dev/my_dev", O_RDWR);
    pthread_create(&t1, NULL, child1, NULL);
    pthread_create(&t2, NULL, child2, NULL);
    pthread_create(&t3, NULL, child3, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    return 0;
}
