/*
 * HTTP_Proxy.h
 *
 *  Created on: Apr 20, 2014
 *      Author: root
 */

#ifndef HTTP_PROXY_H_
#define HTTP_PROXY_H_

u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count);
void ChangetoDnsNameFormat(char* dns, char* host);
char* proxy(char *dns_name, short dns_type);
#endif /* HTTP_PROXY_H_ */
