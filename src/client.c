#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
  // copy the input URL so as not to mutate the original
  char *hostname = strdup(url);
  char *port;
  char *path;

  urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));

  // couldn't we just do this?
  /* sscanf(url, "%s:%s/%s", hostname, port, path); */

  /*
    We can parse the input URL by doing the following:

    1. Use strchr to find the first slash in the URL (this is assuming there is no http:// or https:// in the URL).
    2. Set the path pointer to 1 character after the spot returned by strchr.
    3. Overwrite the slash with a '\0' so that we are no longer considering anything after the slash.
    4. Use strchr to find the first colon in the URL.
    5. Set the port pointer to 1 character after the spot returned by strchr.
    6. Overwrite the colon with a '\0' so that we are just left with the hostname.
  */
  char *p;
  // in case of leading prefix, move the hostname pointer up
  if (strstr(hostname, "https://")) {
    hostname += strlen("https://");
  } 
  if (strstr(hostname, "http://")) {
    hostname += strlen("http://");
  } 
  p = strchr(hostname, '/');
  // in case there's no trailing "/", make it the default path
  if (p == NULL) {
    path = "/";
  }
  else  {
    path = p + 1;
    *p = '\0';
  }

  p = strchr(hostname, ':');
  // add default port if no ":" found
  if (p == NULL) {
    port = "80";
  } else {
    port = p + 1;
    *p = '\0';
  }

  urlinfo->hostname = hostname;
  urlinfo->port = port;
  urlinfo->path = path;

  /* printf("hostname: %s\n", hostname); */
  /* printf("port: %s\n", port); */
  /* printf("path: %s\n", path); */

  return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
  const int max_request_size = 16384;
  char request[max_request_size];
  int rv;
  int request_length = sprintf(request, "GET /%s HTTP/1.1\n"
      "Host: %s:%s\n"
      "Connection: close\n\n",
      path, hostname, port
      );
  /* printf("request:\n%s", request); */

  rv = send(fd, request, request_length, 0);
  return 0;
}

char *get_body(char *buffer) {
  /* printf("header:\n%s\n", buffer); */
  char *body; 
  if (strstr(buffer, "\r\n\r\n")) {
    body = (strstr(buffer, "\r\n\r\n"));
  } else if (strstr(buffer, "\n\n")) {
    body = (strstr(buffer, "\n\n"));
  }
  else body = buffer;
  /* printf("body:\n%s\n", body); */
  /* printf("before body:\n%s", (body - 1)); */
  *body = '\0';
  body++;
  return body;
}

int main(int argc, char *argv[])
{  
  int sockfd, numbytes;  
  char buf[BUFSIZE];
  int print_header = 0;

  if (argc < 2 || argc > 3) {
    fprintf(stderr,"usage: client HOSTNAME:PORT/PATH [-h]\n");
    exit(1);
  }

  if (argc == 3 && strcmp(argv[2], "-h") != 0) {
    fprintf(stderr, "unkown flag: %s\n", argv[2]);
    exit(1);
  }

  if (argc == 3 && strcmp(argv[2], "-h") == 0) {
    print_header = 1;
  }

  urlinfo_t *urlinfo = parse_url(argv[1]);
  sockfd = get_socket(urlinfo->hostname, urlinfo->port);
  /* sockfd = get_socket("localhost", "3495"); */
  send_request(sockfd, urlinfo->hostname, urlinfo->port, urlinfo->path);
  while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0) {
  }
  /* printf("%s\n", buf); */
  char *body = get_body(buf);
  if (print_header) {
    printf("%s\n", buf);
  }
  printf("%s", body);
  
  /* free(body); */
  free(urlinfo);
  close(sockfd);

  /*
    1. Parse the input URL
    2. Initialize a socket by calling the `get_socket` function from lib.c
    3. Call `send_request` to construct the request and send it
    4. Call `recv` in a loop until there is no more data to receive from the server. Print the received response to stdout.
    5. Clean up any allocated memory and open file descriptors.
  */

  return 0;
}
