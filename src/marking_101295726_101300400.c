#include "marking_101295726_101300400.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_EXAMS 20
// Random delay 
void random_delay(double min, double max) {
    double duration = min + ((double)rand() / RAND_MAX) * (max - min);
    usleep(duration * 1000000);
}

// Load rubric from file into shared memory
void load_rubric(const char *filename, Rubric *rubric) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open rubric file");
        for (int i = 0; i < NUM; i++)
            rubric->rubric[i] = 'A' + i; 
        return;
    }
    int num;
    char ans;
    for (int i = 0; i < NUM; i++) {
        if (fscanf(f, "%d, %c", &num, &ans) == 2) {
            rubric->rubric[i] = ans;
        else
        rubric->rubric[i] = 'A' + i;
    }
    fclose(f);
}

// Save rubric back to file
void save_rubric(const char *filename, Rubric *rubric) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Failed to open rubric file for writing");
        exit(1);
    }
    for (int i = 0; i < NUM; i++) {
        fprintf(f, "%d, %c\n", i + 1, rubric->rubric[i]);
    }
    fclose(f);
}

// Load exam into shared memory
bool load_exam(int exam_number, Exam *exam) {
    char filename[64];
    sprintf(filename, "exams/exam%d.txt", exam_number);

    FILE *f = fopen(filename, "r");
    if (!f) return false;

    char line[64];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return false; 
    }

    exam->student_number = atoi(line);  
    for (int i = 0; i < NUM_QUESTIONS; i++)
        exam->marked[i] = false;

    fclose(f);
    return true;  
}

// TA process function
void ta_work(int ta_id, Rubric *rubric, Exam *exam, sem_t *sem_rubric, sem_t *sem_exam) {
    int exam_number = 1;

    srand(time(NULL) ^ (getpid()<<16) ^ exam_number); 
    
    while (true) {
        bool valid = load_exam(exam_number, exam);
        if (!valid) break; 

        
        // Access rubric
        sem_wait(sem_rubric);
        printf("TA %d is reviewing rubric\n", ta_id);
        for (int i = 0; i < NUM; i++) {
            random_delay(0.5, 1.0);
            if (rubric->rubric[i] < 'Z') { 
                char old = rubric->rubric[i];
                rubric->rubric[i] = old + 1;
                save_rubric("Rubric/rubric.txt", rubric);
                printf("TA %d changed rubric Q%d: %c -> %c\n", ta_id, i + 1, old, rubric->rubric[i]);
            }
        }
        sem_post(sem_rubric);

        // Mark exam
        sem_wait(sem_exam);
        printf("TA %d is marking exam %d\n", ta_id, exam->student_number);
        for (int i = 0; i < NUM; i++) {
            if (!exam->marked[i]) {
                random_delay(1.0, 2.0);
                exam->marked[i] = true;
                printf("TA %d marked Q%d of exam %d\n", ta_id, i + 1, exam->student_number);
            }
        }
        sem_post(sem_exam);
        if (exam->student_number == 9999) {
            printf("TA %d finished exam 9999, stopping.\n", ta_id);
            break;
        }
        exam_number++;
    }
    printf("TA %d finished all exams.\n", ta_id);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number_of_TAs>\n", argv[0]);
        return 1;
    }

    int num_TAs = atoi(argv[1]);
    if (num_TAs < 2) {
        printf("Number of TAs must be at least 2.\n");
        return 1;
    }

    // Shared memory
    Rubric *shared_rubric = mmap(NULL, sizeof(Rubric), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    Exam *shared_exam = mmap(NULL, sizeof(Exam), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Semaphores
    sem_unlink("/sem_rubric");
    sem_t *sem_rubric = sem_open("/sem_rubric", O_CREAT, 0644, 1);
    sem_unlink("/sem_exam");
    sem_t *sem_exam = sem_open("/sem_exam", O_CREAT, 0644, 1);

    // Load rubric
    load_rubric("Rubric/rubric.txt", shared_rubric);

    // Fork TA processes
    pid_t pids[num_TAs];
    for (int i = 0; i < num_TAs; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            ta_work(i + 1, shared_rubric, shared_exam, sem_rubric, sem_exam);
            exit(0);
        }
    }

    // Parent waits
    for (int i = 0; i < num_TAs; i++)
        waitpid(pids[i], NULL, 0);

    // Cleanup
    sem_close(sem_rubric);
    sem_unlink("/sem_rubric");
    sem_close(sem_exam);
    sem_unlink("/sem_exam");
    munmap(shared_rubric, sizeof(Rubric));
    munmap(shared_exam, sizeof(Exam));

    printf("All exams completed.\n");
    return 0;
}
