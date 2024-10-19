# Simple HTTP Server

This is a project based entirely off of [Jeffrey Yu's Medium post](https://jeffreyzepengyu.medium.com/how-i-built-a-simple-http-server-from-scratch-using-c-e279569464ca) on how they built a simple HTTP Server.

Due to my lack of C development experience, I was not too familiar with how file descriptors work and their functions,
so it was a great reference to have to understand how to work with them.

## Differences from the post

The post lacks explanation on certain user functions, of which I had to manually implement, those being:

- `url_decode`
- `get_file_extension`
  - `get_substring`
- `get_mime_type`

There might be other major differences as I initially started this project purely reading manual pages, but without
pointers it became quite convoluted to understand how to use them in the contexts I wanted to.

## Compiling

This was made entirely in Linux with Linux in mind, probably all of the libraries I used have a Windows counterpart (or
are the exact same) but I haven't bothered porting due to the project just being to test myself.

A simple `make` is enough to compile this project, which it's binary will live under `bin/server`.

## Usage

This is an HTTP server which serves local files, currently it fully supports three mime types: `text/plain`,
`text/markdown` and `text/html`, but browsers will be ok with a faulty `Content-Type`, so you can technically supply any
local file.

Opening `localhost:9300/test page.html` should give you a blank page with a `Hello There` written on it, that's what I
used to test and make sure `url_decode` was working as intended.
