#include <curl/curl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

// 1MB
const int BUFFER_SIZE = 1000000;

void *handle_request(void *arg);
char *url_decode(char *str, int len);
char *get_file_extension(char *text);
char *get_substring(char *str, int pos);
void build_response(char *file_name, char *file_ext, char *response,
                    size_t *response_len);
char *get_mime_type(char *ext);

int main() {
  int sockfd, *clientfd = 0;
  socklen_t peerAddressSize;
  struct sockaddr_in address, peerAddress;

  // fd = File Descriptor
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    fprintf(stderr, "Failed to create socket.");
    return 1;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(9300);
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1)
    handle_error("bind");

  // 50 is the number of backlog for listening
  if (listen(sockfd, 50) == -1)
    handle_error("listen");

  peerAddressSize = sizeof(peerAddress);

  for (;;) {
    clientfd = malloc(sizeof(int));

    *clientfd =
        accept(sockfd, (struct sockaddr *)&peerAddress, &peerAddressSize);

    if (*clientfd == -1) {
      perror("accept failed");
      continue;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_request, (void *)clientfd);
    pthread_detach(thread_id);
  }

  close(sockfd);

  return 0;
}

void *handle_request(void *arg) {
  int clientfd = *(int *)arg;
  char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t received = recv(clientfd, buffer, BUFFER_SIZE, 0);
  if (received > 0) {
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];

    int match = regexec(&regex, buffer, 2, matches, 0);

    if (match == 0) {
      buffer[matches[1].rm_eo] = '\0';
      char *file_name = buffer + matches[1].rm_so;
      char *decoded_file_name = url_decode(file_name, strlen(file_name));

      char *file_ext = get_file_extension(decoded_file_name);

      if (file_ext != NULL) {
        char *response = malloc(BUFFER_SIZE * 2 * sizeof(char));
        size_t response_len;
        build_response(decoded_file_name, file_ext, response, &response_len);

        send(clientfd, response, response_len, 0);
      }

      free(file_ext);
    }

    regfree(&regex);
  }

  close(clientfd);
  free(arg);
  free(buffer);

  return NULL;
}

char *url_decode(char *str, int len) {
  int str_len = strlen(str);
  char *decoded_string = str;

  for (int i = 0; i < str_len; i++) {
    if (str[i] == '%') {
      if (i + 3 <= str_len) {
        char hex[3];
        hex[0] = str[i + 1];
        hex[1] = str[i + 2];
        hex[2] = '\0';

        int value = strtol(hex, NULL, 16);

        if (value >= 32 && value <= 126) {
          decoded_string[i] = value;
          memmove(&str[i + 1], &str[i + 3], str_len - i - 2);

          str_len -= 2;

          i += 2;
        }
      }
    }
  }

  return decoded_string;
}

char *get_file_extension(char *text) {
  regex_t regex;
  regmatch_t matches[2];

  regcomp(&regex, "\\.([A-Za-z]+)$", REG_EXTENDED);
  int match = regexec(&regex, text, 2, matches, 0);

  if (match == 0)
    return get_substring(text, matches[1].rm_so);

  return NULL;
}

char *get_substring(char *str, int pos) {
  int sub_iteration = 0;
  int str_len = strlen(str);
  char *substring = malloc(str_len * sizeof(char) + 1);

  if (pos > str_len)
    return strdup("");

  for (int i = 0; i < str_len; i++) {
    if (i < pos)
      continue;

    substring[sub_iteration] = str[i];
    sub_iteration++;

    if (i == strlen(str) - 1) {
      substring[sub_iteration] = '\0';
      break;
    }
  }

  return substring;
}

void build_response(char *file_name, char *file_ext, char *response,
                    size_t *response_len) {
  const char *mime_type = get_mime_type(file_ext);
  char *header = malloc(BUFFER_SIZE * sizeof(char));
  snprintf(header, BUFFER_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           mime_type);

  int filefd = open(file_name, O_RDONLY);
  if (filefd == -1) {
    snprintf(response, BUFFER_SIZE,
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/plain\r\n"
             "\r\n"
             "404 Not Found");
    *response_len = strlen(response);
    return;
  }

  struct stat file_stat;
  fstat(filefd, &file_stat);
  off_t file_size = file_stat.st_size;

  *response_len = 0;
  memcpy(response, header, strlen(header) * sizeof(char));
  *response_len += strlen(header);

  ssize_t bytes_read;
  while ((bytes_read = read(filefd, response + *response_len,
                            BUFFER_SIZE - *response_len)) > 0) {
    *response_len += bytes_read;
  }

  free(header);
  close(filefd);
}

char *get_mime_type(char *ext) {
  if (strcmp(ext, "html") == 0) {
    return strdup("text/html");
  } else if (strcmp(ext, "md") == 0) {
    return strdup("text/markdown");
  } else if (strcmp(ext, "txt") == 0) {
    return strdup("text/plain");
  }

  return strdup("");
}
