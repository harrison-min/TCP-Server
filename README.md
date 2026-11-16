# mTLS TCP Server for file storage

A simple file storage system with mutual TLS encryption via OpenSSL and a custom message-passing protocol. Client communicates via mTLS channel to Server which manages the isolated PostgreSQL container.

## Features

* **OpenSSL Integration:** Client and Server are verified for mTLS with self signed certificates
* **Custom Message Passing:** Custom message parsing system with simple format: "PayloadBytes|OperationCode|OperationMetadata|".
* **PostgreSQL Persistence:** Files are stored via PostgreSQL Large Objects within folders of the filesystem
* **Docker Compose Setup:** Containerized deployment of the TCP server and the database instance to isolate database from public traffic.
