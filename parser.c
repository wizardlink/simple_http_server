#include "parser.h"
#include <stdlib.h>
#include <string.h>

// 32 = ASCII code for space
const int DELIMETER = 32;
// 58 = ASCII for :
const int HEADER_DELIMETER = 58;

const int CARRIAGE = 13;
const int LINE_FEED = 10;

request_info_t *parse_request(char *request) {
  request_info_t *info = malloc(sizeof(request_info_t));
  int request_length = strlen(request);

  // This is how many times we found a space character,
  // it allows us to determine how far we are in the
  // request, such as after the first step, we are in the
  // endpoint, third would be the http version.
  // The value can be negative as soon as we are done parsing the
  // first section of data.
  int step = 0;

  // What we will capture until a delimeter.
  char capture[100000];
  int capture_length = 0;

  for (int i = 0; i < request_length; i++) {
    char current = request[i];

    if (current == DELIMETER && step >= 0) {
      capture[capture_length] = '\0';

      if (step == 1) {
        info->endpoint = strdup(capture);
        step = -1;
      } else if (step == 0) {
        info->method = strdup(capture);
        step++;
      }

      memset(capture, 0, sizeof(capture));
      capture_length = 0;
      continue;
    }

    if (current == LINE_FEED && (strcmp(capture, "HTTP/1.1") == 0 ||
                                 strcmp(capture, "HTTP/1.1\r") == 0)) {
      memset(capture, 0, sizeof(capture));
      capture_length = 0;
      continue;
    }

    capture[capture_length] = current;
    capture_length++;
  }

  parse_header_and_content(info, capture);

  return info;
}

void parse_header_and_content(request_info_t *info, char capture[100000]) {
  int capture_length = strlen(capture);

  // -1 = We didn't find more headers and are now reading content
  // 0 = We are still looking for a `:`
  // 1 = We found one, now we parse the value
  short found_delimeter = 0;

  // Amount of line feeds found consectively
  short line_feeds = 0;

  int current_header = 0;

  // I know... But it's kind of funny so I'll use this.
  char capture_capture[100000];
  int capture_capture_length = 0;

  for (int i = 0; i < capture_length; i++) {
    char current = capture[i];

    if (current == LINE_FEED) {
      line_feeds++;
    }

    if (current != LINE_FEED) {
      if (line_feeds == 1 && current != LINE_FEED)
        line_feeds--;
      else if (line_feeds == 2)
        break;
    }

    if (current == HEADER_DELIMETER && found_delimeter != 1) {
      found_delimeter = 1;

      capture_capture[capture_capture_length] = '\0';

      info->headers[current_header].name = strdup(capture_capture);

      memset(capture_capture, 0, sizeof(capture_capture));
      capture_capture_length = 0;

      continue;
    }

    if (current == LINE_FEED && found_delimeter == 1) {
      // -1 because of the carriage return before the line feed.
      capture_capture[capture_capture_length - 1] = '\0';

      info->headers[current_header].value = strdup(capture_capture + 1);
      found_delimeter = 0;

      current_header++;

      memset(capture_capture, 0, sizeof(capture_capture));
      capture_capture_length = 0;
      continue;
    }

    capture_capture[capture_capture_length] = current;
    capture_capture_length++;
  }

  // Skip the line feed and carriage before us.
  info->content = strdup(capture_capture + 2);
}
