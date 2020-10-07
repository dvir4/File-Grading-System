#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>

#define NUM_OF_ARGS 3
#define CHAR_SIZE 1
#define ENTER '\n'
#define SPACE ' '
#define TRUE 1
#define FALSE 0
#define DIFF  32

/*
 * print error massage.
 */
void error() {
    char * errorMassege ={"Error in function\n"};
    write(2, errorMassege ,strlen(errorMassege));
    exit(1);
}
/*
 * check if char is a letter.
 */
int isChar(char ch) {
    if ((65 <= ch && ch <= 90) || (97 <= ch && ch <= 122))
        return TRUE;
    return FALSE;
}
/*
 * check if char is space or \n type.
 */
int canContinue(char ch) {
    if (ch == SPACE || ch == ENTER)
        return TRUE;
    return FALSE;

}
/*
 * in case of similar letters, return true.
 */
int isSimilar(char charOne, char charTwo) {
    if (charOne - charTwo == DIFF || charTwo - charOne == DIFF)
        return TRUE;
    return FALSE;
}
/*
 * open two files and comapare their content.
 */
int compareFiles(char *pathFileOne, char *pathFIleTwo) {
    char charFileOne;
    char charFileTwo;
    int similarFlag = 0;
    // open two files, in case of error write a massage.
    int fileOne = open(pathFileOne, O_RDONLY);
    int fileTwo = open(pathFIleTwo, O_RDONLY);
    if (fileOne < 0 || fileTwo < 0)
        error();

    // read the first two letters.
    int readCharFileOne = read(fileOne, &charFileOne, CHAR_SIZE);
    int readCharFileTwo =read(fileTwo, &charFileTwo, CHAR_SIZE);

    // while there is content on at least one of this two files, read letters.
    while (readCharFileOne || readCharFileTwo) {
        // in case the letters are not the same.
        if (charFileOne != charFileTwo) {
            // in case there are both letters, check if they are similar.
            if (isChar(charFileOne) && isChar(charFileTwo)) {
                if (isSimilar(charFileOne, charFileTwo)) {
                    similarFlag = TRUE;
                    readCharFileOne = read(fileOne, &charFileOne, CHAR_SIZE);
                    readCharFileTwo = read(fileTwo, &charFileTwo, CHAR_SIZE);
                    continue;
                }
                // in case they are not similar, return 2.
                return 2;
            //  check if at least one of the letters are space or \n, in case it is, skip the next letter.
            } else if (canContinue(charFileOne) || canContinue(charFileTwo)) {
                if (canContinue(charFileOne)) {
                    readCharFileOne = read(fileOne, &charFileOne, CHAR_SIZE);
                    continue;
                }
                readCharFileTwo = read(fileTwo, &charFileTwo, CHAR_SIZE);
                continue;
            } else
                return 2;
        }
        readCharFileOne = read(fileOne, &charFileOne, CHAR_SIZE);
        readCharFileTwo = read(fileTwo, &charFileTwo, CHAR_SIZE);
    }
    close(fileOne);
    close(fileTwo);
    // in case of similar flag is on, return 3.
    if(similarFlag)
        return 3;
    return 1;
}


int main(int argc, char *args[]) {
    if (argc != NUM_OF_ARGS)
        error();
    int result = compareFiles(args[1],args[2]);
    exit(result);
}
