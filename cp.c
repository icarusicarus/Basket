#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int arg_num = argc-1;		// number of arguments
	int file_num = argc-2;		// number of files
	printf("ARGC: %d\n", argc);
	
	struct stat statbuf;		// stat struct to check whether the argument is file or directory 
	
	if(stat(argv[arg_num], &statbuf)<0) {	// put last argument in statbuf
		perror("STAT ERROR\n");		// if stat function return negative value, that means error occured. process terminate
		return -1;
	}
	if(!(S_ISDIR(statbuf.st_mode))) {	// if last argument is not directory, taht means invalid command line. process terminate
		printf("Last argument must be directory!\n");
		printf("Usage : %s <filename1> <filename2> <filename3> ... <directory>\n", argv[0]);
		return 0;
	}
	if (arg_num < 4) {		// number of arguments must be larger than 4. (at least 3 file and one directory needed)
		printf("At least 3 files must exist!\n");
		printf("Usage : %s <filename1> <filename2> <filename3> ... <directory>\n", argv[0]);
		return 0;
	}
	
	FILE *fd[file_num];		// array of source files
	FILE *dest[file_num];		// array of destination files
	char *destPath[file_num];	// In case of this assignment, we get files and copy them into directory. 
					// so, destination path should "Directory_name"+"/"+"File_name". create and store destination path into this array
	char buf[257];
	int len;
	
	for(int i = 0; i < file_num; i++) {		// repeat this process as file number
		destPath[i] = (char*)malloc(256);	// allocate destPath size. actually linux maximum path length is 4096 but I allocate 256(for small program)
		sprintf(destPath[i], "%s%s%s", argv[arg_num], "/", argv[i+1]);	// create and store destination path 
		fd[i] = fopen(argv[i+1], "r");		// open source file as read
		dest[i] = fopen(destPath[i], "w");	// open destination as write

		while(len = fread(buf, 1, 256, fd[i])) {	// read source file to buf. reapeat as length of file cotents
			buf[len] = '\0';			
			fwrite(buf, 1, len, dest[i]);		// write buf to destination file.
			printf("Length: %d\n", len);
			printf("%s\n", buf);
		}

		fclose(dest[i]);	// close destination file
		fclose(fd[i]);		// close source file
		printf("%dth file copyed\n", i+1);
	}

	return 0;
}
