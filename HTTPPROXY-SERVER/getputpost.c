// Handles GET and PUT client requests

#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>
#include <stdio.h>       // defines printf, perror, ...
#include <unistd.h>      // read, write
#include <stdlib.h>
#include <syslog.h>

#define MAXBUF 20000


int buffer_read(long long sockfd,char *body,char *file_name, char *filelen_str){     //Read the file content that comes from the PUT and prints it.

	char buf[MAXBUF];
	FILE *file = fopen(file_name,"w"); // opens a file having same name as the client sent
//	char *write_car="";
	int file_read,readsize = 0;

	 syslog(LOG_INFO,"saving file");


if (strlen(body) > 0)
{
fprintf(file,body); // write the body that was found in the first read to the file
readsize = strlen(body);
}


		while ( (file_read = read(sockfd, buf, sizeof(buf))) > 0){

			syslog(LOG_INFO,"reading file");
			printf("%s\n",buf);
	      //read from socket and store in buffer//
	       //if there is any data in socket//
	        	fprintf(file,buf);
	        	readsize +=strlen(buf);
	        //	syslog(LOG_INFO,"bytes writen: ");

	        	if ((strlen(buf) < 1))       //TODO: break if size was bigger that the content-lenght in the header
	        	{
	        		break;                  // break when buffer becomes empty (nothing to read)
	        	}
	        	if (strstr(buf,"\r\n\r\n") != NULL )  // addition over phase 1 implementation to break
	        		return 1;
	        	bzero(buf, sizeof(buf));       //empty the buffer//
		}
  printf("yes!\n");
    fclose(file);
    bzero(buf, sizeof(buf));
	return 1;

	}


void get_request(char *buf, long long sockfd){

	char *parse[2];
	FILE *file;

	 	 			   strtok (buf, " \t\n");
	 	 	parse[0] = strtok (NULL, " /");
	 	 	syslog(LOG_INFO,parse[0]);
	 	 	parse[1] = strtok (NULL, " \r\n");
	            syslog(LOG_INFO,parse[1]);
	            if ( strncmp("HTTP/1.1",parse[1], 8)!= 0 )
		    {
	                write(sockfd,"HTTP/1.1 400 Bad Request\n", 25);
		    }
	           else
	            {
	        	   syslog(LOG_INFO, "file: ");
	        	   syslog(LOG_INFO, parse[0]);


	               file = fopen(parse[0],"r"); // opening the file that the user requests

	               //TODO: change the starting path of the server
	               //  strcpy(path, ROOT);
	               //strcpy(&path[strlen(ROOT)], parse[1]);

	               if(file == NULL){
	               syslog(LOG_ERR,"The file %s doesn't exist\n",parse[0]);
	               bzero(buf,sizeof(buf));
	               sprintf(buf,"HTTP/1.1 404 Not Found\r\nThe file doesn't exist on the Server\r\n");
	               write(sockfd, buf, strlen(buf)); //FILE NOT FOUND
	               return;
	               }
	                else
	                {
	                	syslog(LOG_INFO,"opened file");
	                	/*these next 3 lines for getting the file size are copied from http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c */
	                	fseek(file, 0, SEEK_END); // seek to end of file
	                	size_t size = ftell(file); // get current file pointer
	                	fseek(file, 0, SEEK_SET); // seek back to beginning of file
	                	// proceed with reading the file

	                	bzero(buf, sizeof(buf));

	                	sprintf(buf,"HTTP/1.1 200 OK\r\nContent-Length: %lu\r\nContent-Type: text/plain\r\nIam: aymannedaa\r\n\r\n",size);
	                	write(sockfd,buf, strlen(buf));

	                    bzero(buf, sizeof(buf));


	                    while ( fread(buf,sizeof(char),MAXBUF,file) >0 )
	                    {
	                    	syslog(LOG_INFO, buf);
	                        write (sockfd, buf, strlen(buf));
	                        bzero(buf, sizeof(buf));
	                    }


	                    fclose(file);
	                }

	            }

}

void put_request(char *buf, long long sockfd){

	int br;
	char *filelen_str,*parse[2],*body,*parser;

			parser = strstr(buf,"\r\n\r\n"); // the body of the first reply
			body = parser+4;

			char *cont_len=strstr(buf,"Content-Length:");
			filelen_str=strtok(cont_len+16, " \r\n" );
	 		syslog(LOG_INFO,"Content length =");
	 		syslog(LOG_INFO,filelen_str);

	 					   strtok (buf, " \t\n"); // PUT
		        parse[0] = strtok (NULL, " /");   // filename
		        syslog(LOG_INFO,parse[0]);
		        parse[1] = strtok (NULL, " \r\n"); // HTTP/1.1
		        syslog(LOG_INFO,parse[1]);


				syslog(LOG_INFO,"body has been parsed successfuly");

				if ( strncmp("HTTP/1.1",parse[1],8)!= 0 ){
		             write(sockfd,"HTTP/1.1 400 Bad Request\n", 25);
				return;
				}
		        else
				    br = buffer_read(sockfd, body, parse[0], filelen_str);


		         if (br==1)
		        	 write(sockfd,"HTTP/1.1 200 OK\r\n\r\n", 19);
		         else
		        	 write(sockfd,"HTTP/1.1 400 Bad Request\r\n\r\n", 28);



	}

void post_request(char *buf, long long sockfd)
{
	printf("post request\n");
	char* reply; // the DNS answer that will be replied to client
	char *bodylen_str,*parse[4],*body,*parser;

	//start by parsing the body
			parser = strstr(buf,"\r\n\r\n"); // body of the first reply
			parse[2]=strtok(parser+9,"&");   // The host name in the body, ex. google.com

			printf("hostname: \n%s\n",parse[2]);
   // parse content-lenght in header
			char *cont_len=strstr(buf,"Content-Length:"); // parsing content-lenght
			bodylen_str=strtok(cont_len+16, " \r\n" );   // storing it in bodylen_str
	 		syslog(LOG_INFO,"Content length =");
	 		syslog(LOG_INFO,bodylen_str);
	// parse first line in the header
	 			strtok (buf, " \t\n"); // POST
		        parse[0] = strtok (NULL, " /");   // dns-query
		        syslog(LOG_INFO,parse[0]);

		 	 	if ( strncmp("dns-query",parse[0], 9)!= 0 ) // check 'dns-query'
		 	 			    {
		 	 		                write(sockfd,"HTTP/1.1 400 Bad Request\nNo dns-query found\n", 44);
		 	 		                return;
		 	 			    }

		 	 	parse[1] = strtok (NULL, " \r\n"); // HTTP/1.1
		 	 			        syslog(LOG_INFO,parse[1]);

				if ( strncmp("HTTP/1.1",parse[1],8)!= 0 ){ // check 'HTTP/1.1'
		             write(sockfd,"HTTP/1.1 400 Bad Request\n", 25);
				return;
				}

	syslog(LOG_INFO,"all has been parsed successfuly");
    printf("passed all parsing\n");

printf("starting proxy\n");
bzero(buf, sizeof(buf));

//sends DNS query and returns DNS answer
reply=proxy((char *)parse[2], 1);  // 1: Type A

printf("finished proxy\n");

if (strncmp(reply,"ERROR",5)!=0) // REPLY IS SUCCESSFUL
sprintf(buf,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\nIam: aymannedaa\r\n\r\n%s\r\n\r\n",strlen(reply),reply);

else
sprintf(buf,"HTTP/1.1 400 Bad Request\r\nIam: aymannedaa\r\n\r\nDNS Server can't be reached\r\n\r\n");

write(sockfd,buf, strlen(buf)); // write the DNS response to the client
syslog(LOG_INFO,buf);
printf("%s",buf);
}


