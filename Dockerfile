FROM alpine:latest

RUN apk add --no-cache openssl-dev
RUN apk add --no-cache g++
COPY src/ /src/
RUN g++ -o server src/server.cpp 
RUN g++ -o client src/client.cpp 

