#ifndef SERVER_H
#define SERVER_H

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

void *handle_request(void *arg);

char *not_found_page();

char *bad_request_page();

char *internal_server_error();

char *get_substring(char *str, int start, int end);

char *url_decode(char *str);

#endif // !SERVER_H
