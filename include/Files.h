#ifndef FILES_H
#define FILES_H

#define INITIAL_BUFFER_SIZE 1024 // Define an initial buffer size

void append_to_buffer(char **buffer, const char *data);
void write_log_to_file(const char *buffer); 

#endif
