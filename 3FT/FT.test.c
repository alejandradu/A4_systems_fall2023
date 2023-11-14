#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "FT.h"

#define MAX_PATH_LENGTH 100

void test_FT_insertFile() {
    int status;
    char path[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test.txt";
    char contents[] = "This is a test file.";
    size_t length = strlen(contents) + 1;

    status = FT_insertFile(path, contents, length);
    assert(status == SUCCESS);

    // Try inserting the same file again
    status = FT_insertFile(path, contents, length);
    assert(status == ALREADY_IN_TREE);

    // Try inserting a file with a bad path
    char badPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test";
    status = FT_insertFile(badPath, contents, length);
    assert(status == BAD_PATH);

    // Try inserting a file with a conflicting path
    char conflictingPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT";
    status = FT_insertFile(conflictingPath, contents, length);
    assert(status == CONFLICTING_PATH);
}

void test_FT_containsFile() {
    int status;
    char path[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test.txt";
    char badPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test";
    char nonexistentPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/nonexistent.txt";

    // Insert a file and check if it exists
    char contents[] = "This is a test file.";
    size_t length = strlen(contents) + 1;
    status = FT_insertFile(path, contents, length);
    assert(status == SUCCESS);
    assert(FT_containsFile(path) == TRUE);

    // Check if a file with a bad path exists
    assert(FT_containsFile(badPath) == FALSE);

    // Check if a nonexistent file exists
    assert(FT_containsFile(nonexistentPath) == FALSE);
}

void test_FT_rmFile() {
    int status;
    char path[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test.txt";
    char badPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test";
    char nonexistentPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/nonexistent.txt";

    // Insert a file and remove it
    char contents[] = "This is a test file.";
    size_t length = strlen(contents) + 1;
    status = FT_insertFile(path, contents, length);
    assert(status == SUCCESS);
    assert(FT_rmFile(path) == SUCCESS);
    assert(FT_containsFile(path) == FALSE);

    // Try removing a nonexistent file
    assert(FT_rmFile(nonexistentPath) == NO_SUCH_PATH);

    // Try removing a file with a bad path
    assert(FT_rmFile(badPath) == BAD_PATH);
}

void test_FT_getFileContents() {
    int status;
    char path[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test.txt";
    char badPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/test";
    char nonexistentPath[MAX_PATH_LENGTH] = "/Users/alejandraduran/Documents/Pton_courses/COS217/A4/COS217_A4/3FT/nonexistent.txt";

    // Insert a file and get its contents
    char contents[] = "This is a test file.";
    size_t length = strlen(contents) + 1;
    status = FT_insertFile(path, contents, length);
    assert(status == SUCCESS);
    assert(strcmp(FT_getFileContents(path), contents) == 0);

    // Try getting the contents of a nonexistent file
    assert(FT_getFileContents(nonexistentPath) == NULL);

    // Try getting the contents of a file with a bad path
    assert(FT_getFileContents(badPath) == NULL);
}

int main() {
    test_FT_insertFile();
    test_FT_containsFile();
    test_FT_rmFile();
    test_FT_getFileContents();
    printf("All tests passed!\n");
    return 0;
}