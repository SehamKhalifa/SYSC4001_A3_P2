SYSC4001_A3_P2 README:
To compile the program run this in the terminal:

Step 1: gcc -o marking_program marking_101295726_101300400.c -Wall -pthread -lrt

Step 2: ./src/marking_program 

So to run the program, it requires: Number of TAs (n ≥ 2) Rubric file (e.g., rubric.txt) At least 20 exam files (last file must contain student number 9999 to trigger termination)

Test Cases:
Run with 2 tas using : ./src/marking_program 2  and as a result Program should run as expected

Run with 4 tas using : ./src/marking_program 4  and as a result Program should run as expected

Run with 1 ta using : ./src/marking_program 1  and as a result Program will fail and require you to have number of tas ≥ 2


Contributers:
Seham Khalifa 101295726
Pardis Ehsani 101300400
