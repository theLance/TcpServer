# Simple TCP Echo Server

Run a local TCP server to echo back reversed text.

## Installation

To create the binaries, just type:
```bash
make all
```

## Usage

First create the server. Use the command:
```bash
./srv
```
Then create the desired amount of clients, using the command:
```bash
./clt
```

The clients and the server shut down once all clients sent
```
q
```
Ie. a letter 'q' and a newline.

## Notes

The server will try to launch on port 8080, but will try several others before
 calling it quits. The same goes for the clients.

The server in the current implementation cannot just quit. You can stop it with
 CTRL+C (SIGINT) if you wish, it will only have the socket open at that point, so
 it should not be that bad.
