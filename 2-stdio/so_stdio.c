#include "./util/so_stdio.h"

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
				perror("File open error: \"r+\" mode");
				return NULL;
			}
		} else {
			fd = open(pathname, O_RDONLY);
			if (fd < 0) {
				perror("File open error: \"r\" mode");
				return NULL;
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
				return NULL;
			}
		} else {
			fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
			if (fd < 0) {
				perror("File open error: \"w\" mode");
				return NULL;
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
				return NULL;
			}
		} else {
			fd = open(pathname, O_APPEND | O_CREAT);
			if (fd < 0) {
				perror("File open error: \"a\" mode");
				return NULL;
			}
		}
	}
	#elif defined(_WIN32)

	#endif
	SO_FILE *file = malloc(sizeof(SO_FILE));
	file->stdin_buffer = malloc(BUF_SIZE * sizeof(char));
	file->stdout_buffer = malloc(BUF_SIZE * sizeof(char));
	file->stdin_buf_cursor = file->stdout_buf_cursor = file->stdin_buflen = 0;
	file->cursor = 0;
	file->err_flag = 0;
	file->fd = fd;
	return file;
}

FUNC_DECL_PREFIX int so_fclose(SO_FILE *stream) {
	int rc;
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
		if (rc == 0) {
			return 0;
		} else if (rc < 0) {
			perror("File read error");
			stream->err_flag = 1;
			return SO_EOF;
		}
		stream->cursor += rc;
		stream->stdin_buflen = rc;
		stream->stdin_buf_cursor = 0;
	}
	return ((int) (stream->stdin_buffer[stream->stdin_buf_cursor++]));
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream) {
	int status;
	if (stream->stdout_buf_cursor == BUF_SIZE - 1 || ((char) c == '\n')) {
		status = so_fflush(stream);
	}
	
	if (status < 0) {
		perror("File write error (so_fputc)");
		stream->err_flag = 1;
		return SO_EOF;
	}
	stream->stdout_buffer[stream->stdout_buf_cursor++] = (unsigned char) c;
	stream->cursor++;

	return c;
}

FUNC_DECL_PREFIX
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	size_t nr_bytes_read = 0;
	size_t total_bytes = size * nmemb;
	unsigned char read_character;

	while (nr_bytes_read < total_bytes) {
		read_character = (unsigned char) so_fgetc(stream);
		if (read_character == 0) {
			return nr_bytes_read;
		} else if (read_character < 0) {
			perror("File read error (so_fread)");
			stream->err_flag = 1;
			return SO_EOF;
		}
		memcpy(ptr + nr_bytes_read, &read_character, 1);
		nr_bytes_read++;
	}
	return nr_bytes_read;
}

FUNC_DECL_PREFIX
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	size_t nr_bytes_written = 0;
	size_t total_bytes = size * nmemb;
	int write_character;
	int status;

	while (nr_bytes_written < total_bytes) {
		write_character = (int) *((unsigned char *) ptr);
		status = so_fputc(write_character, stream);
		if (status < 0) {
			perror("File read error (so_fread)");
			stream->err_flag = 1;
			return SO_EOF;
		}
		nr_bytes_written++;
	}
	
	return nr_bytes_written;
}

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence) {
	off_t offset;
	#if defined(__linux__)
	offset = lseek(stream->fd, offset, whence);
	#elif defined(_WIN32)

	#endif

	if (offset < 0) {
		#if defined(__linux__)
		perror("lseek error");
		stream->err_flag = 1;
		#elif defined(_WIN32)

		#endif
		
		return SO_EOF;
	}
	stream->cursor = offset;
	return offset;
}

FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream) {
	return stream->cursor;
}

FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream) {
	int wc;

	wc = write(stream->fd, stream->stdout_buffer, stream->stdout_buf_cursor);
	if (wc < 0) {
		perror("File write error");
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

}