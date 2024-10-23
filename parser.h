#ifndef PARSER_H
#define PARSER_H

typedef struct request_header {
  char *name;
  char *value;
} request_header_t;

typedef struct request_info {
  char *content;
  char *endpoint;
  char *method;
  request_header_t headers[50];
} request_info_t;

request_info_t *parse_request(char *request);

void parse_header_and_content(request_info_t *info, char capture[100000]);
#endif // !PARSER_H
