FROM alpine:latest

RUN apk add --no-cache openssl-dev
RUN apk add --no-cache g++
RUN apk add --no-cache postgresql-dev
RUN apk add --no-cache chafa
COPY src/ /src/
COPY certs/ /certs/
COPY test/ /test/
RUN g++ /src/server.cpp /src/openSSLModule.cpp /src/postgresModule.cpp /src/requestHandler.cpp -o server -lssl -lcrypto -lpq
RUN g++ /src/client.cpp /src/openSSLModule.cpp /src/postgresModule.cpp /src/requestSender.cpp -o client -lssl -lcrypto -lpq
RUN chafa test/test.png
# Docker commands
# To build: docker build -t tcp-server .
# To run/test: docker run -it  --network test tcp-server sh