/*
 * getput.h
 *
 *  Created on: Mar 3, 2014
 *      Author: root
 */

#ifndef GETPUT_H_
#define GETPUT_H_

int buffer_read(long long sockfd, char *body,char *file_name, char *filelen_str);
void get_request(char *buf, long long sockfd);
void put_request(char *buf, long long sockfd);
void post_request(char *buf, long long sockfd);


#endif /* GETPUT_H_ */
