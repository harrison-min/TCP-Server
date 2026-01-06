FROM alpine:latest

RUN apk add --no-cache openssl-dev
RUN apk add --no-cache g++
COPY src/ /src/
COPY certs/ /certs/
RUN g++ /src/server.cpp /src/OpenSSLModule.cpp -o /server -lssl -lcrypto
RUN g++ src/client.cpp /src/OpenSSLModule.cpp -o client -lssl -lcrypto

# Docker commands
# To build: docker build -t tcp-server .
# To run/test: docker run -it  --network host tcp-server sh