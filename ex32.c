#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>

#define MAX_SIZE 150
#define NUM_OF_ARGS 3
#define CHAR_SIZE 1
#define NEW_LINE '\n'
#define END_OF_LINE '\0'
#define ENDING_FILE_SIZE 2
#define TIMEOUT 5
#define NO_C_FILE 1
#define COMPILATION_ERROR 2
#define TIMEOUT_ERROR 3
#define BAD_OUTPUT 4
#define SIMILAR_OUTPUT 5
#define GREAT_JOB 6
#define FILE_OUTPUT "output.txt"
#define RESULT_FILE "results.csv"
#define EXE_FILE "./program.out"

/*
 * write an error massage
 */
void error() {
    char * errorMassege ={"Error in system call\n"};
    write(2, errorMassege ,strlen(errorMassege));
    exit(1);
}

/*
 * write student result to file.
 */
void writeToFile(char *studentName, int status,int writenToFile) {
    // open result file.
    int fw = open(RESULT_FILE, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
    char buffer[MAX_SIZE];
    strcpy(buffer,studentName);
    if(writenToFile)
        write(fw,"\n",strlen("\n"));
    // write student grades depends on status.
    switch (status) {
        case 1:
            strcat(buffer,",0,NO_C_FILE");
            write(fw,buffer,strlen(buffer));
            break;
        case 2:
            strcat(buffer,",20,COMPILATION_ERROR");
            write(fw,buffer,strlen(buffer));
            break;
        case 3:
            strcat(buffer,",40,TIMEOUT");
            write(fw,buffer,strlen(buffer));
            break;
        case 4:
            strcat(buffer,",60,BAD_OUTPUT");
            write(fw,buffer,strlen(buffer));
            break;
        case 5:
            strcat(buffer,",80,SIMILAR_OUTPUT");
            write(fw,buffer,strlen(buffer));
            break;
        case 6:
            strcat(buffer,",100,GREAT_JOB");
            write(fw,buffer,strlen(buffer));
            break;
        default:
            break;
    }
    close(fw);
}
/*
 * check if file is c type.
 */
int isCProgram(char const *name) {
    size_t len = strlen(name);
    // check if the ending of a file is '.c'.
    return len > ENDING_FILE_SIZE && strcmp(name + len - ENDING_FILE_SIZE, ".c") == 0;
}
/*
 * get input of config file.
 */
void getInput(char *path, char buffer[][MAX_SIZE]) {
    char ch;
    int i = 0;
    int j = 0;
    int status = 0;
    int fdin = open(path, O_RDONLY);
    if (fdin < 0)
        error();
    // get first 3 lines.
    for (i = 0; i < NUM_OF_ARGS; i++) {
        status = read(fdin, &ch, 1);
        j = 0;
        while (status != 0 && ch != NEW_LINE) {
            buffer[i][j] = ch;
            status = read(fdin, &ch, CHAR_SIZE);
            j++;
        }
        // add \n in the ending of the line.
        buffer[i][j] = END_OF_LINE;
    }
}
/*
 * compare program output with output.txt
 */
int compareOutput(char *updatedPath, char *currOutput) {
    char buffer[MAX_SIZE];
    strcpy(buffer,updatedPath);
    strcat(buffer,"/output.txt");
    int status = 0;
    // define arguments for compare program input.
    char *args[] = {"./comp.out", buffer, currOutput, NULL};
    pid_t task_pid = fork();
    if (task_pid == -1) {
        error();
    } else if (task_pid == 0) {
        if (execvp(args[0], args) < 0)
            error();
        exit(0);
    } else {
        // wait for comp.out exit status and return it.
        waitpid(task_pid, &status, 0);
        if (WIFEXITED(status))
            status = WEXITSTATUS(status);
        return status;
    }
}

/*
 * compile a program.
 */
int compileProgram(char *cFile, char *path) {
    // create a full path that contains c file.
    char compilePath[MAX_SIZE];
    strcpy(compilePath,path);
    strcat(compilePath,"/");
    strcat(compilePath,cFile);

    // compile and save the program as program.out
    char *compile[] = {"gcc", "-o", "program.out", compilePath, NULL};
    int status = 0;

    pid_t task_pid = fork();
    if (task_pid == -1) {
        error();
    } else if (task_pid == 0) {
        //if (chdir(path) < 0) // here
        //    error();
        if (execvp(compile[0], compile) < 0) {
            error();
        }
        exit(0);
    } else {
        // wait until child will end.
        waitpid(task_pid, &status, 0);
        return status;
    }
}
/*
 * run c file and check the his running time.
 */
int executeProgram(char *path, char *input) {
    int status = 0;
    char outputPath[MAX_SIZE];
    strcpy(outputPath,path);
    strcat(outputPath,"/output.txt");

    pid_t exited_pid;
    pid_t pid = fork();
    if (pid == -1) {
        error();
    } else if (pid == 0) {
        //if (chdir(path) < 0)
        //    error();
        pid_t runProgram_pid = fork();
        if (runProgram_pid == 0) {
            // open output file as output.txt and input file as input.txt
            int fr = open(input, O_RDONLY);
            int fw = open(outputPath, O_WRONLY | O_CREAT,S_IRWXU | S_IRWXG | S_IRWXO);
            dup2(fr,STDIN_FILENO);
            dup2(fw, STDOUT_FILENO);
            close(fr);
            close(fw);
            // execute program.
            char *arg[] = {EXE_FILE, NULL};
            if (execvp(arg[0], arg) < 0)
                error();
            exit(0);
        }
        // run another program that sleep for 5 seconds.
        pid_t timeout_pid = fork();
        if (timeout_pid == 0) {
            sleep(TIMEOUT);
            exit(0);
        }
        // wait for one of the programs to return and kill the other one.
        do{
            exited_pid = wait(NULL);
        } while (exited_pid != timeout_pid && exited_pid != runProgram_pid);
        if (exited_pid == runProgram_pid) {
            kill(timeout_pid, SIGKILL);
            exit(0);
        } else {
            // in case the timeout program fhinished first, return timeout error.
            kill(runProgram_pid, SIGKILL);
            exit(TIMEOUT_ERROR);
        }
    } else {
        // wait until the program will end and return status.
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            status = WEXITSTATUS(status);
        return status;
    }
}

// merge two stings into one.
void mergePath(char *path, char *dirName) {
    strcat(path, "/");
    strcat(path, dirName);
}

/*
 * delete given file.
 */
void deleteFiles(char *path){
    char compilePath[MAX_SIZE];
    char outputPath[MAX_SIZE];

    //strcpy(compilePath,path);
    strcat(compilePath,EXE_FILE);
    unlink(compilePath);

    strcpy(outputPath,path);
    strcat(outputPath,"/output.txt");
    unlink(outputPath);


}
/*
 * scan student dir recursive, comeplile and run his program.
 */
int scanDirectory(char *path,char *input, char *outputFIle, char* studentName,int *writenToFIle) {
    char updatedPath[MAX_SIZE];
    DIR *pDir;
    int foundCFile = 0;
    struct dirent *pDirent;
    char *fileName;
    int status = 0;
    int tempResult = 0;
    if ((pDir = opendir(path)) == NULL)
        error();
    // search in student files.
    while ((pDirent = readdir(pDir)) != NULL) {
        if (strcmp(pDirent->d_name, "..") != 0 && strcmp(pDirent->d_name, ".") != 0) {
            strcpy(updatedPath, path);
            // in case of directory type, update path and call the function againn.
            if (pDirent->d_type == DT_DIR) {
                mergePath(updatedPath, pDirent->d_name);
                tempResult = scanDirectory(updatedPath, input,outputFIle,studentName, writenToFIle);
                if(!foundCFile)
                    foundCFile = tempResult;
            }
            // define file name.
            fileName = pDirent->d_name;
            // in case of c program type.
            if (isCProgram(fileName)) {
                foundCFile = 1;
                // compile program and return status, in case of invalid status, write student grade 20.
                status = compileProgram(fileName,path);
                if(status) {
                    writeToFile(studentName, COMPILATION_ERROR,*writenToFIle);
                    deleteFiles(updatedPath);
                    *writenToFIle = 1;
                    continue;
                }
                // execute student program, in case of timeout error, write student grade - 40.
                status = executeProgram(updatedPath,input);
                if (status) {
                    writeToFile(studentName, TIMEOUT_ERROR,*writenToFIle);
                    deleteFiles(updatedPath);
                    *writenToFIle = 1;
                    continue;
                }
                // compare student output to correct output and write grade according to the result.
                status = compareOutput(updatedPath,outputFIle);
                if(status == 1)
                    writeToFile(studentName, GREAT_JOB,*writenToFIle);
                else if(status == 2)
                    writeToFile(studentName, BAD_OUTPUT,*writenToFIle);
                else
                    writeToFile(studentName, SIMILAR_OUTPUT,*writenToFIle);
                deleteFiles(updatedPath);
                *writenToFIle = 1;
            }
        }
    }
    closedir(pDir);
    if (foundCFile)
        return 1;
    return 0;

}
/**
 * scan every student in the directory.
 */
void checkStudents(char *path,char *input, char* outputFile) {
    int writenToFIle = 0;
    char updatedPath[MAX_SIZE];
    DIR *pDir;
    struct dirent *pDirent;
    int fileFlag = 0;
    // open given directory.
    if ((pDir = opendir(path)) == NULL)
        error();
    // scan directory files.
    while ((pDirent = readdir(pDir)) != NULL) {
        // in case of file that named '..' or '.', skip to next file.
        if (strcmp(pDirent->d_name, "..") != 0 && strcmp(pDirent->d_name, ".") != 0) {
            strcpy(updatedPath, path);
            // in case of directory, define a updated path to directory.
            if (pDirent->d_type == DT_DIR) {
                mergePath(updatedPath, pDirent->d_name);
                // scan the directory, and search for c file, in case of no c file define grade to 0.
                fileFlag = scanDirectory(updatedPath, input,outputFile,pDirent->d_name, &writenToFIle);
                if (fileFlag == 0){
                    writeToFile(pDirent->d_name, NO_C_FILE,writenToFIle);
                    writenToFIle = 1;
                }
            }
        }
    }
    closedir(pDir);
}

int main(int argc, char *args[]) {
    char buffer[NUM_OF_ARGS][MAX_SIZE];
    DIR *pDir;
    if (argc != 2)
        error();
    getInput(args[1], buffer);
    checkStudents(buffer[0],buffer[1],buffer[2]);
    unlink(EXE_FILE);
    exit(0);
}
