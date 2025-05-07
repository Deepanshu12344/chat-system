# Peer-to-Peer Chat System in C
This is a simple peer-to-peer chat system written in C that allows multiple clients to register and send direct messages to each other via a central server. It uses TCP sockets, POSIX threads, and a custom protocol for communication.

## Features
Register clients using IP address and name

Send direct messages from one client to another

Multithreaded server handling multiple clients concurrently

Linked list for managing connected clients

Thread-safe operations using mutexes

Simple custom command-based protocol


## Technologies Used 
C (POSIX sockets, threads)

TCP networking (IPv4)

pthread for concurrency

Linux (tested on Ubuntu)