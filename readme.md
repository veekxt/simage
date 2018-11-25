A simple encryption proxy tool

usage:

In client: `simage -c -p 10001 -t 12.34.56.78:8088 -k some-random-string`

In server: `simage -s -p 8088 -k some-random-string`

now you get a socks5 proxy on `127.0.0.1:10001`, `-t` designate the server( ip or domain ). `-k` is used for authentication and must be the same.

and you will get a proxy:
```
localhost  <=====>  12.34.56.78
```

All data is transmitted by aes-256-gcm encryption (supported by sodium).The byte stream is completely random.

How to build:
```
mkdir tmp
cd tmp
cmake ..
make
```
