# HTTP_PROXY

1. Overview:

-Server:
The program is a concurrent HTTP server acting as an HTTP Proxy, sending DNS queries on behalf of clients and replying with the DNS answer(s). The program responds to POST requests containing the website's hostname that the client wants to translate to IPV4 addresses (as the extention supports only A queries) , then the server sends a DNS query (Type A) and accordingly relay the DNS Answer (containing IPV4 address(es) list) back to the client. The server as in phase 2 is a concurrent server that opens a new thread for each new client. And it runs as a Daemon process in the background if the user wants to.

-Client:
The client is used to send GET, PUT or POST requests to the HTTP Proxy and print the result to the user.

2. User instructions:

-Server:
The user has to pass two mandatory argument (a port number which the server will accept client requests on and a number indicating whether he wants the server to run as a Deamon process or not) and an
optional 'host' argument which takes a hostname or an IP address that clients would send their requests to .
The command line arguments are: [ <host> ] <port#> <1: Daemon-ON, any other value:Daemon-OFF>\n");
If no host is given , the default IP address will be chosen as the address of the server (usually the local loopback address 127.0.0.1 ) .
To run the server as a Daemon use type 1 in the command line argument, else type anything else to not run as Daemon process (but you shouldn't leave it empty).

-Client:
In case of POST the command line arguments are: POST <Website><URL of Proxy Server> <service name or number of Proxy>
Ex. POST comnet.aalto.fi http://nwprog1.netlab.hut.fi 3000
<Website> is the hostname required by the DNS to resolve.

3. Testing and known limitations:

The server was tested on nwprog1.netlab.hut.fi using port 2002 through having multiple clients(10) sending POST requests in parallel, and the Server responded to each request. 
The client was tested on a normal intel core i3 PC on an ubuntu distribution. It was tested with different DNS servers and it was successful in printing the correct messages, whether the DNS
server was available or not,etc,...
All messages sent contain an 'Iam' field of 'aymannedaa' to make testing more easier.

The server supports only type A POST requests as it only sends type A DNS requests.
The DNS server used (8.8.8.8) is hard coded in the server so that
limits the ability of some one who is not a programer to change that.
Also there is no automatic way to support IPV6 DNS servers addresses.
The server is intended to provide name to address translation service to clients so it doesn't show authoritative answers, or other information, it only shows the IPV4 address of the hostname
provided by the client.
The server only supports 2 header fields Content-type and Content-lenght fields.
For the client it also supports only type A as it was inteded only to be used with the server who also supports only type A DNS record. The client also supports only the compulsory fields of Content-type and Content-lenght in the post header.

