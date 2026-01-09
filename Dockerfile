FROM alpine:latest

RUN apk add --no-cache openssl-dev
RUN apk add --no-cache g++
RUN apk add --no-cache postgresql-dev
COPY src/ /src/
COPY certs/ /certs/
RUN g++ /src/server.cpp /src/openSSLModule.cpp /src/postgresModule.cpp -o server -lssl -lcrypto -lpq
RUN g++ /src/client.cpp /src/openSSLModule.cpp /src/postgresModule.cpp -o client -lssl -lcrypto -lpq

# Docker commands
# To build: docker build -t tcp-server .
# To run/test: docker run -it  --network test tcp-server sh