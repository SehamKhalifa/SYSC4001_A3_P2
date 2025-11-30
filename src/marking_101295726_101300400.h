#ifndef MARKING_H_
#define MARKING_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/mman.h>

#define NUM 5

//struct fot exam
typedef struct{             
    int student_number;   
    bool marked[NUM];
} Exam;

//struct for rubric
typedef struct {
    char rubric[NUM];      
} Rubric;

#endif