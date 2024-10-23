#include "server.h"
#include "parser.h"
#include <curl/curl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/unistd.h>

// 1MB
const int BUFFER_SIZE = 1000000;

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
    request_info_t *info = parse_request(buffer);
    char *endpoint = url_decode(info->endpoint);

    if (strcmp(info->method, "GET") == 0) {
      if (strcmp(endpoint, "/test") == 0) {
        char *response;
        FILE *file = fopen("test.txt", "r");
        char file_data[100000];

        if (file == NULL) {
          response = internal_server_error();
        } else {
          char data[1000];
          while (fgets(data, sizeof(data), file) != NULL) {
            strcat(file_data, data);
          }

          char formatted_data[100045];
          sprintf(formatted_data,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/plain\r\n"
                  "\r\n"
                  "%s",
                  file_data);
          response = strdup(formatted_data);
        }

        send(clientfd, response, strlen(response), 0);
        fclose(file);
      }
    } else if (strcmp(info->method, "POST") == 0) {
      if (strcmp(endpoint, "/test") == 0) {
        char *response;
        FILE *file = fopen("test.txt", "w");

        if (file == NULL) {
          response = internal_server_error();
        } else {
          fprintf(file, "%s", info->content);
          response = strdup("HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "\r\n"
                            "OK");
        }

        send(clientfd, response, strlen(response), 0);

        fclose(file);
      }
    } else {
      char *page = not_found_page();
      send(clientfd, page, strlen(page), 0);
    }

    free(info);
  } else {
    char *page = bad_request_page();
    send(clientfd, page, strlen(page), 0);
  }

  close(clientfd);
  free(arg);
  free(buffer);

  return NULL;
}

char *not_found_page() {
  return strdup("HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "404 Not Found");
}

char *bad_request_page() {
  return strdup("HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "400 Bad Request");
}

char *forbidden_request_page() {
  return strdup("HTTP/1.1 403 Forbidden\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "403 Forbidden");
}

char *internal_server_error() {
  return strdup("HTTP/1.1 500 Internal Server Error\n\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "500 Internal Server Error");
}

char *get_substring(char *str, int start, int end) {
  int sub_iteration = 0;
  int str_len = strlen(str);
  char *substring = malloc(str_len * sizeof(char) + 1);

  if (start > str_len)
    return strdup("");

  for (int i = 0; i < str_len; i++) {
    if (i == end) {
      substring[sub_iteration] = '\0';
      break;
    }

    if (i < start || i > end)
      continue;

    substring[sub_iteration] = str[i];
    sub_iteration++;
  }

  return substring;
}

char *url_decode(char *str) {
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
