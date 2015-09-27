// An HTTP client program using Get and Put, part of Network Programming Course in Aalto

#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUF  1024

int sockfd, file_read, This_is_a_GET = 0;
char buffer[MAXBUF];                      /*our buffer has a max size of 1024*/
char URL[100],hostname_withouthttp[100];
char *hostname,*file_path;*file_name,website;

void buffer_read(){     /*Read the file content that comes from the Get and POST and print it, and in general for Get and Put Requests check the reply and give an indication if file wasn't downloaded/uploaded correctly*/
char *HTTP, *body;
int k=0; /*counter to signal the first reply read*/
FILE *fileget;

if(	This_is_a_GET == 1){  // If its a get_request open a file to store result into

	fileget = fopen(file_path,"w");
	}


        do
    	{
        bzero(buffer, sizeof(buffer));       /*empty the buffer*/
        file_read = read(sockfd, buffer, sizeof(buffer)); /*read from socket and store in buffer*/
        if ( file_read > 0 ) { /*if there is any data in socket*/
printf("%s\n",buffer);
	if (k==0){ /*The first data will include the GET/PUT/POST header reply*/
	HTTP = strstr(buffer,"HTTP/1.1"); /*parsing the header for http/1.1 xxx response*/
	if(!(strncmp(HTTP+9,"2",1)== 0)) /*if the first character after 'http/1.1 ' included 2 then the file was uploaded/downloaded successfuly , 'HTTP/1.1 200', 'HTTP/1.1 202', etc,... */
	{
	printf("Request failed: with an HTTP failour response from server\n");
	close(sockfd);
	exit(1);
	}
	body=strstr(buffer,"\r\n\r\n"); /*seperate the header from the body in the first reply*/

	if (strlen(body)==0)/*check if the first reply includes a body or not */
	break;
//printf("%s\n",buffer);
	printf("%s",body); /*if it includes then print it, which is a case of GET and POST*/
	// Additionally if its a get_request print the body in a file
	if(	This_is_a_GET == 1){

    	fprintf(fileget,body);
	}

	}
	else
	{/* for all replies except the first one , which should include data in GET and would be empty in put*/
	printf("%s",buffer); /**/

	if(	This_is_a_GET == 1){

	    	fprintf(fileget,buffer);
		}

	}
	k=1;
	}
    	}
    	while ( file_read > 0 );

        if(	This_is_a_GET == 1){


        	    	fclose(fileget);
        		}

     	printf("\n");

	}

void post_request(){

sprintf(buffer, "POST /dns-query HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\nIam: aymannedaa\r\n\r\nName=%s&Type=A\r\n\r\n",hostname,strlen(website)+12,website);

printf("%s\n",buffer);
write(sockfd, buffer, strlen(buffer));

buffer_read();

}

void get_request(){

	This_is_a_GET = 1;

	sprintf(buffer, "GET /%s HTTP/1.1\r\nHost: %s\r\nIam: aymannedaa\r\n\r\n",file_path,hostname);

	write(sockfd, buffer, strlen(buffer));

	buffer_read(); /* Read the data that comes from the Get Request and print it */

	}

void put_request(){

 FILE *file;
   int i=0; /*i is the counter for reaching MAXBUF*/
   size_t size;
   char ch, ch_senttobuf[MAXBUF];


   file = fopen(file_name,"r"); /* opening the file that the user PUTs*/
	   if(file == NULL){
                fprintf(stderr,"The file %s doesn't exist\n",file_name);
	close(sockfd);
	exit(1);
		}

/*these next 3 lines for getting the file size are copied from http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c */
fseek(file, 0, SEEK_END); // seek to end of file
size = ftello(file); // get current file pointer
fseek(file, 0, SEEK_SET); // seek back to beginning of file
// proceed with reading the file

//send the header seperatly in the first write//
	sprintf(buffer, "PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nIam: aymannedaa\r\nContent-Length: %lu\r\n\r\n",file_name,URL,size);
	write(sockfd, buffer, strlen(buffer));
	bzero(buffer, sizeof(buffer));

//in consequent writes read part of the file equaling to MAXBUF and send it, keep looping until the sll the file is sent //
   while(1)
   {
//read character by character storing it in ch_senttobuf and each time checking for end of file
      ch = fgetc(file);  //TODO: change this to 'fread' like in server
      if( feof(file) )
      {
          break ;
      }
    // ch_senttobuf[i]= (char)(ch) ;
	ch_senttobuf[i]= ch ;
	if(i >= MAXBUF || (strlen(ch_senttobuf) >= size))	/*start writing only if MAXBUF is reached or if file size is less than MAXBUF and it is full */
	{
	sprintf(buffer,"%s",ch_senttobuf);
	write(sockfd,buffer,strlen(buffer));
   // while (1){}; // to test question 1 in assignment 2 when client halts
	bzero(buffer,sizeof(buffer));
	bzero(ch_senttobuf,sizeof(ch_senttobuf)); /*free the ch_senttobuf everytime*/

	if(size< MAXBUF)  /*write end of file and break as no more writes needed in case of small file size*/
	{
	sprintf(buffer,"\r\n\r\n");
	write(sockfd,buffer,strlen(buffer));
	break;
	}
	i=-1;
	}
	i=i+1;
  	}

	if(strlen(ch_senttobuf)>0) /*in case of large file sizes the last read characters from the file are printed as they were not printed in the loop above*/
	{
	sprintf(buffer,"%s\r\n\r\n",ch_senttobuf);
	write(sockfd,buffer,strlen(buffer));
	bzero(buffer,sizeof(buffer));
	bzero(ch_senttobuf,sizeof(ch_senttobuf));
	 }

	//sprintf(buffer,EOF);
	//write(sockfd,buffer,strlen(buffer));
	//bzero(buffer,sizeof(buffer));

   	fclose(file);	/*close file*/

        buffer_read(); /* Read the data that comes from the Put Request and print it */

}

//print_address function is copied from name_connect.c file given in lecture 3 with minor changes//

void print_address(const char *prefix, const struct addrinfo *res) /*check whether the returned address from getaddrinfo() called in tcp_connect is of a valid IPV6 or IPV4 address format*/
{
	struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)res->ai_addr;
	void *address;

	if (res->ai_family == AF_INET)
		address = &(sin->sin_addr);
	else if (res->ai_family == AF_INET6)
		address = &(sin6->sin6_addr);
	else {
		printf("Unknown address\n");
		return;
	}

}

//tcp_connect function is copied from name_connect.c file given in lecture 3 with minor changes//

int tcp_connect(const char *host, const char *serv) /*function responsible for getting the IP address of the hostname , opening the socket and connecting to that IP address*/
{
	int n;
	struct addrinfo	hints, *res, *ressave;


	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) { /*function resposible for getting IP adddress from a given hostname through DNS*/
		fprintf(stderr, "tcp_connect error for %s, %s: %s\n",
			host, serv, gai_strerror(n));
		return -1;
	}
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, /*opening the socket */
				res->ai_protocol);
		if (sockfd < 0)
			continue;	/* ignore this one */

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) /*connecting to the IP address*/
			break;		/* success */
		printf("connect failed\n");

		close(sockfd);	/* ignore this one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {	/* errno set from final connect() */
		fprintf(stderr, "tcp_connect error for %s, %s\n", host, serv);
		sockfd = -1;
	} else {
		print_address("We are using address", res);
		}


	freeaddrinfo(ressave);

	return(sockfd);
}


int main(int argc, char *argv[])
{

int i;


if ( (argc == 4) && (strncmp(argv[1],"GET",5) == 0)){ /*check if user inputs GET with its 3 arguments*/

strcpy(URL,argv[2]);

if( URL[0]=='h' && URL[1]=='t' && URL[2]=='t' && URL[3]=='p' ){  /*if user inputs the URL with 'http://' convert the URL to a hostname_withouthttp which is the URL without the 'http://' */
for(i=0;i<=strlen(URL)-7;i++)
{
hostname_withouthttp[i]=URL[i+7];
}
}
else
strcpy(hostname_withouthttp,URL);


    hostname = strtok (hostname_withouthttp, " /"); /*Converts URL into hostname used in tcp_connect and file_path used in GET header*/
    file_path = strtok (NULL, " ");

sockfd = tcp_connect(hostname, argv[3]); /*call function tcp_connect with hostname and service name/no */
	if (sockfd >= 0) {       /*if tcp_connect returned successfuly call get_request and close the socket*/
		get_request();
		close(sockfd);
		printf("file downloaded successfuly\n");
	} else return -1;

}
else
if ( (argc == 5) && (strncmp(argv[1],"PUT",3) == 0)){ /* check if user inputs PUT with its 4 arguments*/

strcpy(URL,argv[2]);
file_name=argv[3];  /*different than in GET as here the user gives the file_name as a seperate argument not within the URL as in GET*/

if( URL[0]=='h' && URL[1]=='t' && URL[2]=='t' && URL[3]=='p' ){ /*if user inputs the URL with 'http://' convert the URL to a hostname_withouthttp which is the URL without the 'http://' */
for(i=0;i<=strlen(URL)-7;i++)
{
hostname_withouthttp[i]=URL[i+7];
}
}
else
strcpy(hostname_withouthttp,URL);


    hostname = strtok (hostname_withouthttp, " /"); /*Converts URL into hostname used in tcp_connect*/

sockfd = tcp_connect(hostname, argv[4]); /*call function tcp_connect with hostname and service name/no */
	if (sockfd >= 0) {  /*if tcp_connect returned successfuly call put_request and close the socket*/
		put_request();
		close(sockfd);
		printf("file uploaded successfuly\n");
	} else return -1;

}
else
if( (argc == 5) && (strncmp(argv[1],"POST",4) == 0)){ /* check if user inputs POST with its 4 arguments*/

	website=argv[2];
	strcpy(URL,argv[3]);
	  /*different than in GET as here the user gives the file_name as a seperate argument not within the URL as in GET*/

	if( URL[0]=='h' && URL[1]=='t' && URL[2]=='t' && URL[3]=='p' ){ /*if user inputs the URL with 'http://' convert the URL to a hostname_withouthttp which is the URL without the 'http://' */
	for(i=0;i<=strlen(URL)-7;i++)
	{
	hostname_withouthttp[i]=URL[i+7];
	}
	}
	else
	strcpy(hostname_withouthttp,URL);


	    hostname = strtok (hostname_withouthttp, " /"); /*Converts URL into hostname used in tcp_connect*/

	sockfd = tcp_connect(hostname, argv[4]); /*call function tcp_connect with hostname and service name/no */
		if (sockfd >= 0) {  /*if tcp_connect returned successfuly call put_request and close the socket*/
			post_request();
			close(sockfd);
			printf("operation ended successfuly\n");
		} else return -1;

}
else    /* if user inputs any of GET or PUT with wrong number of arguments or 'GET' or 'PUT' wasn't issued by the user as a first argument issue an error showing the correct user arguments  */
{
fprintf(stderr, "Usage in case of GET: <GET> <full URL of the file> <service name or number>\nUsage in case of PUT: <PUT> <URL of Server> <input file name> <service name or number> \nUsage in case of POST: <POST> <Website> <URL of Proxy Server> <service name or number of Proxy>\nEx. GET http://www.ietf.org/rfc/rfc2616.txt 80\nPUT http://nwprog1.netlab.hut.fi test.txt 3000\nPOST comnet.aalto.fi http://nwprog1.netlab.hut.fi 3000\n");
return -1;
}
	return 0;
}
