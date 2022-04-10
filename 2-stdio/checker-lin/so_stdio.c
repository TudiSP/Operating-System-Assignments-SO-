#include "../util/so_stdio.h"
#include <stdio.h>

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode) {
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
				perror("File open error \"r+\" mode");
				return NULL;
			}
		} else if (mode[1] == 0) {
			fd = open(pathname, O_RDONLY);
			if (fd < 0) {
				perror("File open error \"r\" mode");
				return NULL;
			} 
		} else {
			perror("Invalid arg");
			return NULL;
		}
	} else if (mode[0] == 'w') {
		/* 
		write only mode with implicit creation and truncation
		*/
		if (mode[1] == '+') {
			/* 
			read & write mode with implicit creation and truncation
			*/
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0666);
			if (fd < 0) {
				perror("File open error \"w+\" mode");
				return NULL;
			}
		} else if (mode[1] == 0) {
			fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (fd < 0) {
				perror("File open error \"w\" mode");
				return NULL;
			} 
		} else {
			perror("Invalid arg");
			return NULL;
		}
	} else if (mode[0] == 'a') {
		/* 
		append mode
		*/
		if (mode[1] == '+') {
			/* 
			read & append mode
			*/
			fd = open(pathname, O_RDWR | O_CREAT | O_APPEND, 0666);
			if (fd < 0) {
				perror("File open error \"a+\" mode");
				return NULL;
			}
		} else if (mode[1] == 0) {
			fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0666);
			if (fd < 0) {
				perror("File open error \"a\" mode");
				return NULL;
			}
		} else {
			perror("Invalid arg");
			return NULL;
		}
	} else {
		perror("Invalid arg");
		return NULL;
	}
	#elif defined(_WIN32)

	#endif

	SO_FILE *file = malloc(sizeof(SO_FILE));
	file->stdin_buffer = malloc(BUF_SIZE * sizeof(char));
	file->stdout_buffer = malloc(BUF_SIZE * sizeof(char));
	file->stdin_buf_cursor = 0;
	file->stdout_buf_cursor = 0;
	file->stdin_buflen = 0;
	file->cursor = 0;
	file->err_flag = 0;
	file->write_flag = 0;
	file->popen_flag = 0;
	file->fd = fd;
	return file;
}

FUNC_DECL_PREFIX int so_fclose(SO_FILE *stream) {
	int rc;
	stream->write_flag = 1;
	so_fflush(stream);

	free(stream->stdin_buffer);
	free(stream->stdout_buffer);
	
	rc = close(stream->fd);
	if (rc < 0) {
		perror("File close error");
		stream->err_flag = 1;
		return SO_EOF;
	}
	free(stream);
	return 0;
}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE *stream) {
	/* reached end of stdin buffer */
	if (stream->stdin_buf_cursor == stream->stdin_buflen) {
		int rc = read(stream->fd, stream->stdin_buffer, BUF_SIZE);
		if (rc < 0) {
			return SO_EOF;
		}
		stream->cursor += rc;
		stream->stdin_buflen = rc;
		stream->stdin_buf_cursor = 0;
	}
	return ((int) (stream->stdin_buffer[stream->stdin_buf_cursor++]));
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream) {
	int status = 0;
	if (stream->stdout_buf_cursor == BUF_SIZE || c == 0) {
		status = so_fflush(stream);
	}
	
	if (status < 0) {
		perror("File write error (so_fputc)");
		stream->err_flag = 1;
		return SO_EOF;
	}
	stream->stdout_buffer[stream->stdout_buf_cursor] = (unsigned char) c;
	stream->cursor++;
	stream->write_flag = 1;
	return ((int) (stream->stdout_buffer[stream->stdout_buf_cursor++]));
}

FUNC_DECL_PREFIX
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	size_t nr_bytes_read = 0;
	size_t total_bytes = size * nmemb;
	unsigned char read_character;

	while (nr_bytes_read < total_bytes) {
		read_character = (unsigned char) so_fgetc(stream);
		if (read_character < 0) {
			return SO_EOF;
		}
		memcpy(ptr + nr_bytes_read, &read_character, 1);
		nr_bytes_read++;
	}
	return nr_bytes_read / size;
}

FUNC_DECL_PREFIX
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	size_t nr_bytes_written = 0;
	size_t total_bytes = size * nmemb;
	int write_character;
	int status;

	while (nr_bytes_written < total_bytes) {
		write_character = (int) *(((unsigned char *) ptr) + nr_bytes_written);
		status = so_fputc(write_character, stream);
		if (status < 0) {
			perror("File read error (so_fread)");
			stream->err_flag = 1;
			return SO_EOF;
		}
		nr_bytes_written++;
	}
	
	return nr_bytes_written / size;
}

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence) {
	off_t off_result;
	#if defined(__linux__)
	off_result = lseek(stream->fd, offset, whence);
	#elif defined(_WIN32)

	#endif

	if (off_result < 0) {
		#if defined(__linux__)
		perror("lseek error");
		stream->err_flag = 1;
		#elif defined(_WIN32)

		#endif
		
		return SO_EOF;
	}
	stream->cursor = off_result;
	return 0;
}

FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream) {
	return stream->cursor;
}

FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream) {
	int wc = 0;
	if (!stream->stdout_buf_cursor) return 0;
	if (stream->write_flag) {
		wc = write(stream->fd, stream->stdout_buffer, stream->stdout_buf_cursor);
		if (wc < 0) {
			perror("File write error");
			stream->err_flag = 1;
			return SO_EOF;
		}
	} else {
		perror("No prior write operations executed to justify flushing");
		stream->err_flag = 1;
		return SO_EOF;
	}

	stream->stdout_buf_cursor = 0;
	stream->cursor += wc;
	return 0;
}

#if defined(__linux__)
FUNC_DECL_PREFIX int so_fileno(SO_FILE *stream) {
	return stream->fd;
}
#elif defined(_WIN32)
FUNC_DECL_PREFIX HANDLE so_fileno(SO_FILE *stream);
#else
#error "Unknown platform"
#endif

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream) {
	char c;
	int eof = read(stream->fd, &c, 1);
	lseek(stream->fd, -1, SEEK_CUR);
	return eof;
}

FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream) {
	return stream->err_flag;
}

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type) {


	return NULL;



	SO_FILE *p_stream;
	int pdes[2], fd;
	char const *argvec[] = {"sh","-c", NULL, NULL};
	argvec[2] = command;

	#if defined(__linux__)
	/* opening pipe */
	int status = pipe(pdes);
	if (status < 0) {
		perror("Piping error (so_popen)");
		return NULL;
	}


	pid_t pid;
	pid = fork();
	

	switch (pid) {
	case -1:
		/* error forking */
		/* closing pipes */
		close(pdes[0]);
		close(pdes[1]);
		perror("Forking error (so_popen)");
		return NULL;
	case 0:
		/* child process */
		if (type[0] == 'r') {
			close(pdes[0]);
			/* redirecting output */
			dup2(pdes[1], STDOUT_FILENO);
		} else {
			close(pdes[1]);
			/* redirecting input */
			dup2(pdes[0], STDIN_FILENO);
		}

		execvp("/bin/sh", (char *const *) argvec);
	}

	if (type[0] == 'r') {
		close(pdes[1]);
		fd = pdes[0];
	} else {
		close(pdes[0]);
		fd = pdes[1];
	}
	
	
	#elif defined(_WIN32)
	
	#else
	#error "Unknown platform"
	#endif
	
	p_stream = malloc(sizeof(SO_FILE));
	p_stream->stdin_buffer = malloc(BUF_SIZE * sizeof(char));
	p_stream->stdout_buffer = malloc(BUF_SIZE * sizeof(char));
	p_stream->stdin_buf_cursor = p_stream->stdout_buf_cursor = p_stream->stdin_buflen = 0;
	p_stream->cursor = 0;
	p_stream->err_flag = p_stream->write_flag = p_stream->popen_flag = 0;
	p_stream->fd = fd;
	p_stream->pid = pid;
	return p_stream;

}

FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream) {
	return -1;
	int status;

	waitpid(stream->pid, &status, 0);
	so_fclose(stream);

	return status;
}
