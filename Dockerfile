FROM alpine:latest

RUN apk add --no-cache openssl-dev
RUN apk add --no-cache g++
RUN apk add --no-cache postgresql-dev
RUN apk add --no-cache chafa
COPY src/ /src/
COPY certs/ /certs/
COPY test/ /test/
RUN g++ /src/server.cpp /src/openSSLModule.cpp /src/postgresModule.cpp /src/requestHandler.cpp -o server -lssl -lcrypto -lpq
RUN g++ /src/client.cpp /src/openSSLModule.cpp /src/postgresModule.cpp /src/requestSender.cpp /src/clientUI.cpp -o client -lssl -lcrypto -lpq
RUN mkdir target
# Docker commands
# To run client: docker run -it  --network frontEndTest tcp-server sh