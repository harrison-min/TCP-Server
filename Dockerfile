FROM alpine:latest

RUN apk add --no-cache openssl-dev
RUN apk add --no-cache g++
COPY src/ /src/
RUN g++ -o myapp src/server.cpp 
CMD ["/myapp"]
