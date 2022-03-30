#include "./util/so_stdio.h"

SO_FILE *so_fopen(const char *pathname, const char *mode) {
	SO_FILE *file = malloc(sizeof(SO_FILE));
	file->buffer = malloc(BUF_SIZE * sizeof(char));
	int fd;
	#if defined(__linux__)
	
	if (mode[0] == 'r') {
		/* 
		read only mode
		*/
		if (mode[1] == '+') {
			/* 
			read & write mode
			*/
			fd = open(pathname, O_RDWR);
			if (fd < 0) {
				perror("File open error: \"r+\" mode");
				return -1;
			}
		} else {
			fd = open(pathname, O_RDONLY);
			if (fd < 0) {
				perror("File open error: \"r\" mode");
				return -1;
			}
		}
	}
	if (mode[0] == 'w') {
		/* 
		write only mode with implicit creation and truncation
		*/
		if (mode[1] == '+') {
			/* 
			read & write mode with implicit creation and truncation
			*/
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC);
			if (fd < 0) {
				perror("File open error: \"w+\" mode");
				return -1;
			}
		} else {
			fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
			if (fd < 0) {
				perror("File open error: \"w\" mode");
				return -1;
			}
		}
	}
	if (mode[0] == 'a') {
		/* 
		append mode
		*/
		if (mode[1] == '+') {
			/* 
			read & append mode
			*/
			fd = open(pathname, O_APPEND | O_CREAT | O_RDWR);
			if (fd < 0) {
				perror("File open error: \"a+\" mode");
				return -1;
			}
		} else {
			fd = open(pathname, O_APPEND | O_CREAT);
			if (fd < 0) {
				perror("File open error: \"a\" mode");
				return -1;
			}
		}
	}
	#elif defined(_WIN32)

	#endif
}

int main(int argc, char *argv[]) {
	return 0;
}
