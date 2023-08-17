#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define SHM_SIZE 10

pthread_mutex_t mutex;

int main() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    pthread_mutex_init(&mutex, NULL); // Mutex başlatılması

    pid_t producer_pid = fork();

    if (producer_pid == 0) { // Child process (Producer)
        close(pipe_fd[0]); // Close read end of the pipe

        for (int i = 1; i <= SHM_SIZE; i++) {
            pthread_mutex_lock(&mutex); // Kritik bölge başlangıcı
            write(pipe_fd[1], &i, sizeof(int));
            pthread_mutex_unlock(&mutex); // Kritik bölge sonu
            sleep(1);
        }

        close(pipe_fd[1]);
        exit(0);
    } else if (producer_pid > 0) { // Parent process
        close(pipe_fd[1]); // Close write end of the pipe

        pid_t consumer_pid = fork();

        if (consumer_pid == 0) { // Child process (Consumer)
            close(pipe_fd[1]); // Close write end of the pipe

            int data;
            close(pipe_fd[1]); // Close the write end of the pipe
            while (1) { // Sürekli döngü
                int bytes_read = read(pipe_fd[0], &data, sizeof(int));
                if (bytes_read <= 0) {
                    break; // Veri gelmezse döngüden çık
                }
                pthread_mutex_lock(&mutex); // Kritik bölge başlangıcı
                printf("Consumer read: %d\n", data);
                pthread_mutex_unlock(&mutex); // Kritik bölge sonu
                sleep(1);
            }

            close(pipe_fd[0]);
            exit(0);
        } else if (consumer_pid > 0) { // Parent process
            close(pipe_fd[0]); // Close read end of the pipe
            waitpid(producer_pid, NULL, 0); // Üretici işlemin bitmesini bekle
            waitpid(consumer_pid, NULL, 0); // Tüketici işlemin bitmesini bekle
        } else {
            perror("Fork failed for consumer process");
            exit(1);
        }
    } else {
        perror("Fork failed for producer process");
        exit(1);
    }

    pthread_mutex_destroy(&mutex); // Mutex sonlandırılması

    return 0;
}
