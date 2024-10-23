# Simple endpoint based HTTP Server

This is a project based off of [Jeffrey Yu's Medium post](https://jeffreyzepengyu.medium.com/how-i-built-a-simple-http-server-from-scratch-using-c-e279569464ca) on how they built a simple HTTP Server.

Due to my lack of C development experience, I was not too familiar with how file descriptors work and their functions,
so it was a great reference to have to understand how to work with them.

## HTTP request parser

I ended up writing a custom parser for HTTP requests, it is based off of the [MDN page on HTTP Messages](https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages), it will correctly parse headers, although I didn't end up using them in this project.

It brutely goes through the first line of text, separates the method and endpoint and ignores the HTTP version,
implementing it would not be too hard, but for my purposes I chose to ignore it. For the headers it parses based off of
the delimeter and decides it's end based off of the carriage return. Whenever it detects two carriage returns have been
made, it breaks off of it's logic and returns the rest as the content of the request, as per spec.

## Compiling

This was made entirely in Linux with Linux in mind, probably all of the libraries I used have a Windows counterpart (or
are the exact same) but I haven't bothered porting due to the project just being to test myself.

A simple `make` is enough to compile this project, which it's binary will live under `bin/server`.

## Usage

This is an HTTP server which listens for two requests only:

### `GET /test`

This will return the contents of the `test.txt` file to you.

### `POST /test`

And this, write onto `test.txt` the data you sent.
