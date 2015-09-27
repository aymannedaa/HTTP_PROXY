/*
 ============================================================================
 Name        : HTTP_Proxy.c
 Author      : Ayman Elkadi
 Version     :
 Copyright   : Your copyright notice
 Description : HTTP_PROXY in C, Ansi-style
 ============================================================================
 */

//References for the code:
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms682016%28v=vs.85%29.aspx
//Lecture notes
//http://www.uow.edu.au/~markus/teaching/CSCI319/resolver.c
//http://www.binarytides.com/dns-query-code-in-c-with-winsock

#include <stdio.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include <netdb.h>


#define MAXBUF 20000
int dns_type;

// ID: identification number
// RD: recursion desired - 1 bit
// TC: truncated message - 1 bit
// AA: authoritative answer - 1 bit
// OPCODE: what is the message -4 bits
// QR: is it a DNS query or a DNS response - 1 bit
// RCODE: response code - 4 bits
// Z: authentication and checking - 3 bits
// RA: recursion available - 1 bit
// QDCOUNT: Questions count
// ANCOUNT: Answers count
// NSCOUNT: Authoritative count
// ARCOUNT: Resource count

// DNS header structure
struct Header_dns {
     short ID;
     char RD:1, TC:1 , AA:1 ,OPCODE :4, QR :1, RCODE: 4, Z :3, RA :1;
     short QDCOUNT,ANCOUNT,NSCOUNT, ARCOUNT;
};

// QTYPE: Type of question, type A, AAAA, etc,...
// QCLASS: CLASS of question

//DNS question structure
struct Question_dns
{
     short QTYPE,QCLASS;
};

#pragma pack(push, 1) // used to alter values of Resource Records
// DNS Resource record structure
struct ResourceRecord_dns
{
     short RRTYPE, RRCLASS;
     int RRTTL;
     short RRSIZE;
};
#pragma pack(pop)

// Answer record structure
struct Answer_dns
{
    char *name;
    struct ResourceRecord_dns *resource;
    char *rdata;
};

char *dns_server;
char* proxy(char *, short);


//***Source of function : http://www.binarytides.com/dns-query-code-in-c-with-winsock***///
// function to read  3www6google3com format from buffer then convert it to dotted format (www.google.com)
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
    name = (unsigned char*)malloc(256);

    name[0]='\0';

    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }

        reader = reader+1;

        if(jumped==0)
        {
            *count = *count + 1; //if we haven't jumped to another location then we can count up
        }
    }

    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }

    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++)
    {
        p=name[i];
        for(j=0;j<(int)p;j++)
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot

    printf("inside readname fun: %s\n",name);
    return name;
}



//*** source of function : http://www.uow.edu.au/~markus/teaching/CSCI319/resolver.c
//It is used for changing the dotted format of normal website which is for google www.google.com to the format sent in the DNS query which is 3www6google3com .
//This is done by counting the number of charachters before a '.' charachter then writing this number at the beginning of those charachters, then starting over from the next character after the '.'.
void ChangetoDnsNameFormat(char* dns, char* host)
{
    int lock = 0 , i;
    strcat(host,"."); // required as charecters are counted until a dot is found.

    for(i = 0 ; i < strlen(host) ; i++)
    {
        if(host[i]=='.')
        {
            *dns++ = i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host[lock];
            }
            lock++;
        }
    }
    *dns++='\0';
}


char* proxy(char *dns_name, short dns_type)
{

//	printf("entered proxy\n");

	char buf[MAXBUF];char *address;

	char* output=(char *)malloc(1024); //enough space for answer
	int i=0,j=0,sizeHeader,sizeQuestion,addressSize,end=0;

	// create new queries for DNS Header and Question structures
	struct Header_dns *query_header;
	struct Question_dns *query_question;
	query_header=(struct Header_dns *)&buf; // attach the DNS header to the buffer

	// fill DNS header with the following:
	//Query ID is chosen to be the process ID of the calling process
	//Recursion is desired
	// We have one question in the query
	//All the other fields are chosen to be zeros
	query_header->ID= (unsigned short) htons(getpid());query_header->RD= 1;query_header->TC=0;query_header->AA=0;query_header->OPCODE=0;query_header->QR=0;query_header->RCODE=0;query_header->Z=0;query_header->RA=0;query_header->QDCOUNT=htons(1);query_header->ANCOUNT=0;query_header->NSCOUNT=0;query_header->ARCOUNT=0;

	//size variables
	sizeHeader   =sizeof(struct Header_dns);
	sizeQuestion =sizeof(struct Question_dns);

    //As mentioned in the assignment guidelines and lecture we could use a public dns_Server or get the local DNS server from the system
	// For this implementation we have chosen to use a public DNS server which is google's DNS_server(8.8.8.8).
	dns_server= "8.8.8.8";
	// other DNS servers used for testing:
	//dns_server="195.148.124.10";
	//dns_server="2001:4860:4860::8888";
	//dns_server="4.85.23.113";

	// set timout for socket to be 40 seconds
    struct timeval timeout;
    timeout.tv_sec = 40;
    timeout.tv_usec = 0;  // needed to avoid errors

	//printf("%s\n",dns_name);

    // attaching buffer to address with size of the header
	address =(char*)&buf[sizeHeader];
	// calling a function to change the format of the hostname we want to query from www.google.com => 3www6google3com which is required in the DNS query
	ChangetoDnsNameFormat(address , dns_name);

	//printf("ChangetoDnsNameFormat succeeded\n");

	// calculating the size of the address after format change
	 addressSize = strlen((const char*)address) +1;

	//question query should be added in buffer after a size of 'Qsize'
	int Qsize = sizeHeader + addressSize;

	//attaching Question structure to buffer
	query_question =(struct Question_dns*)&buf[Qsize];
	//fill Question sturcture with the type of the dns query (which should be 1: TYPE A) and Question class before sending the buffer
	query_question->QTYPE= htons( dns_type );query_question->QCLASS = htons(1);

   //if we send to an IPV4 destination
    struct sockaddr_in pservaddr;
      pservaddr.sin_addr.s_addr = inet_addr(dns_server);
      pservaddr.sin_family = AF_INET;
      pservaddr.sin_port = htons(53);
 	  int family = AF_INET;

   //if we send to an IPV6 destination
// 	  struct sockaddr_in6 pservaddr;
//	inet_pton(AF_INET6, "2001:4860:4860::8888", &(pservaddr.sin6_addr));
//	pservaddr.sin6_family = AF_INET6;
//  pservaddr.sin6_port = htons(53);
//  int family = AF_INET6;

    //create socket for UDP protocol
    int sockfd = socket( family , SOCK_DGRAM , IPPROTO_UDP);//UDP packet for DNS queries
    // set socket options for timeout of 40 seconds
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

  //  printf("%s\n",buf);

    //for IPV4 , whole query has size of Header,Address and Question
   if( sendto(sockfd,(char*)buf,sizeQuestion+Qsize,0,(struct sockaddr*)&pservaddr,sizeof(pservaddr)) < 0)
   // for IPV6
 // if( sendto(sockfd,(char*)buf,strlen((char *)buf),0,(struct sockaddr *)&pservaddr,sizeof(pservaddr)) < 0)

   {
        //failed to send query\n"
        return "ERROR";
    }
    printf("Sending DNS query was successful\n");

    int sizeofservadd = sizeof(pservaddr);
    socklen_t* socklen = (socklen_t*)&sizeofservadd;

    //for IPV4
    if(recvfrom (sockfd,(char*)buf , 20000 , 0 , (struct sockaddr*)&pservaddr ,socklen ) <0)
    //for IPV6
   // if(recvfrom (sockfd,(char*)buf , 20000 , 0 , (struct sockaddr*)&pservaddr ,socklen ) <0)
     {
         //failed to receive DNS response\n"
         return "ERROR";
     }
     printf("receiving DNS response was successful\n");

     query_header = (struct Header_dns*) buf;

     //attaching the answer to a pointer to char to be able to parse it
     char * response= &buf[sizeQuestion+Qsize];
     close(sockfd); //closing DNS socket as its not needed anymore


       //lets read the ans
     struct Answer_dns ans[30]; // lets make it max 30 answers
     i=0;


     while(i < ntohs(query_header->ANCOUNT)) //loop for all answers
       {
         // storing the names found in the DNS answer
    	 ans[i].name =ReadName(response,buf, &end); // changing format of name found in answer from 3www6google3com => www.google.com
         response+= end; // end is the size of the address so we know how many bytes we can traverse to read the next field
//printf("name is %s\n",ans[i].name);

         // storing the resource record
         ans[i].resource = (struct ResourceRecord_dns*)(response);
         response+= sizeof(struct ResourceRecord_dns);

         // if Record type is of type A (IPV4) then allocate memory for the data(Addresses) equal to the size of the record
         if(ntohs(ans[i].resource->RRTYPE) == 1){
           ans[i].rdata = ( char*)malloc(ntohs(ans[i].resource->RRSIZE));

           j=0;
           // store record data fields (addresses)
           while(j < ntohs(ans[i].resource->RRSIZE))
           {
     	ans[i].rdata[j] = response[j];
           j=j+1;
           }
           // final data field should contain '\0'
           ans[i].rdata[ntohs(ans[i].resource->RRSIZE)] = '\0';
           response+=ntohs(ans[i].resource->RRSIZE);
         }else{  // alias names
        	 printf("else");
        	 ans[i].rdata = ReadName(response,buf, &end);
           response+=end;
         }
       i=i+1;
       }


       struct sockaddr_in addr;
       printf("finished reading answer\n");
strcpy(output,"");
		i=0;
// storing answer in a format :
// <IPV4> belongs to <name>
// then returning it to post_request
		while(i < ntohs(query_header->ANCOUNT))
       {
         if( ntohs(ans[i].resource->RRTYPE) == 1){ //IPv4 address
        addr.sin_addr.s_addr = (*((long*)ans[i].rdata));
        	  strcat(output,"IPv4 address: ");
        	  strcat(output, inet_ntoa(addr.sin_addr));
        	  strcat(output, " belongs to: ");

          }

       strcat(output, ans[i].name);
       strcat(output, "\n");
       printf("%s\n",output);
       i=i+1;
       }

       return output;
}



