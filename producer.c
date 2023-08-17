#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>

#define SHM_SIZE 10
#define NUM_THREADS 10

pthread_mutex_t mutex;

int main() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    int* shared_sayac = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *shared_sayac = 1; // Ortak global sayac değişkeni

    while (1) {
        pid_t producer_pid = fork();

        if (producer_pid == 0) { // Child process (Producer)
            close(pipe_fd[0]); // Close read end of the pipe

            for (int i = 1; i <= SHM_SIZE; i++) {
                pthread_mutex_lock(&mutex); // Kritik bölge başlangıcı
                write(pipe_fd[1], &i, sizeof(int));
                printf("%d.Thread  : %d - Sayac: %d\n", i,i, *shared_sayac);
                (*shared_sayac)++;
                pthread_mutex_unlock(&mutex); // Kritik bölge sonu
                sleep(1);
            }

            close(pipe_fd[1]);
            exit(0);
        } else if (producer_pid > 0) { // Parent process
            wait(NULL);
        } else {
            perror("Fork failed");
            exit(1);
        }

        // Tüketici işlemi
        pid_t consumer_pid = fork();

        if (consumer_pid == 0) { // Child process (Consumer)
            execvp("./consumer", NULL); // Başka bir programı çalıştır

            perror("Consumer process execvp failed");
            exit(1);
        } else if (consumer_pid > 0) { // Parent process
            close(pipe_fd[1]); // Close write end of the pipe
            close(pipe_fd[0]); // Close read end of the pipe
            waitpid(consumer_pid, NULL, 0); // Tüketici işlemin bitmesini bekle
        } else {
            perror("Fork failed for consumer process");
            exit(1);
        }
    }

    munmap(shared_sayac, sizeof(int));
    return 0;
}
