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
			perror("File read error");
			return -1;
		}
		stream->stdin_buflen = rc;
		stream->stdin_buf_cursor = 0;
	}
	return ((int) (stream->stdin_buffer[stream->stdin_buf_cursor++]));
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream) {
	int rc;
	if (stream->stdout_buf_cursor == BUF_SIZE - 1 || ((char) c == '\n')) {
		/* so_fflush(stream);*/
	}
	rc = write(stream->fd, stream->stdout_buffer + stream->stdout_buf_cursor, 1);
	if (rc < 0) {
		perror("File write error (so_fputc)");
		return -1;
	}

	return c;
}

FUNC_DECL_PREFIX
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	size_t nr_bytes;
	size_t total_bytes = size * nmemb;
	unsigned char read_character;
	int cr;
	for (nr_bytes = 0; nr_bytes < total_bytes; nr_bytes++) {
		read_character = (unsigned char) so_fgetc(stream);
		if (read_character < 0) {
			perror("File read error (so_fread)");
			return -1;
		}
		memcpy(ptr + nr_bytes, &read_character, 1);
	}
}

FUNC_DECL_PREFIX
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	
}