/*  Copyright (c) 2014 "Kaazing Corporation," (www.kaazing.com)
**
**  This file is part of Robot.
**
**  Robot is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Affero General Public License as
**  published by the Free Software Foundation, either version 3 of the
**  License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  You should have received a copy of the GNU Affero General Public License
**  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include "robot_test.h"

extern "C" {
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
}

const char * DIR = (char *)"scripts";
const char * SCRIPT_EXT = (char *)".rpt";

void echo_test();
void alt_echo_test();
void server_echo_test();
void server_alt_echo_test();
int accept(int sock, in_port_t port);
int socket_bind_and_listen(in_port_t port);
int socket_connect(in_port_t port);
char * ABS_PATH(char * script_name);

TEST(Client, EchoTest){
	/*	Arguments: 
    **  abs_path - the absolute path of the script (e.g. /home/user/../script_name.rpt),
	**	functionPointer (function where your client code is, NULL if none), 
	**	functionPointer (function to any cleanup code you need to run after the client code, NULL if none) 
	**	timeout (in seconds, set <= 0 for no timeout) 
	*/
	char * script_path = ABS_PATH((char * )"serverEcho");
	ROBOT_TEST(script_path, echo_test, NULL, 10);
	free(script_path);
}

TEST(Client, AltEchoTest){
	char * script_path = ABS_PATH((char * )"serverAltEcho");
	ROBOT_TEST(script_path, alt_echo_test, NULL, 10);
	free(script_path);
}

TEST(Server, EchoTest){
	char * script_path = ABS_PATH((char * )"clientEcho");
	ROBOT_TEST(script_path, server_echo_test, NULL, 10);
	free(script_path);
}

TEST(Server, AltEchoTest){
	char * script_path = ABS_PATH((char * )"clientAltEcho");
	ROBOT_TEST(script_path, server_alt_echo_test, NULL, 10);
	free(script_path);
}

TEST(Script, Self){
	char * script_path = ABS_PATH((char * )"self");
	ROBOT_TEST(script_path, NULL, NULL, 10);
	free(script_path);
}

/*
** Append script_name to current working directory to get absolute path
 */
char * ABS_PATH(char * script_name){
	char * working_directory = getcwd(NULL, 0);
	int len = strlen(working_directory) + strlen(script_name) + strlen(DIR) + strlen(SCRIPT_EXT) + 3;
	char * abs_path = (char * )malloc(sizeof(char) * len);
	strcpy(abs_path, working_directory);
	strcat(abs_path, "/");
	strcat(abs_path, DIR);
	strcat(abs_path, "/");
	strcat(abs_path, script_name);
	strcat(abs_path, SCRIPT_EXT);
	free(working_directory);
	return abs_path;
}

/*
** Simple echo server test
** Receives message "Testing...123" and sends it back
*/
void server_alt_echo_test(){
	int sock = socket_bind_and_listen(8001);
	ASSERT_TRUE(sock != -1);
	
	ROBOT_JOIN();

	int accepted = accept(sock, 8001);
	ASSERT_TRUE(accepted != -1);
	char * expected_msg = (char *)"Testing...123";
	char * recv_msg = (char *)malloc(strlen(expected_msg) + 1);
	int recvd = recv(accepted, recv_msg, strlen(expected_msg), 0);
	ASSERT_TRUE(recvd == (int)(strlen(expected_msg)));
	*(recv_msg + recvd) = '\0';
	ASSERT_STREQ(expected_msg, recv_msg);
	int sent = send(accepted, recv_msg, strlen(recv_msg), 0);
	ASSERT_TRUE(sent == (int)(strlen(recv_msg)));
	int closed = close(accepted);
	ASSERT_TRUE(closed != -1);
	closed = close(sock);
	ASSERT_TRUE(closed != -1);
	free(recv_msg);
}

/*
** Simple echo server test
** Sends message "Hello, world!" and receives it back
*/
void server_echo_test(){
	int sock = socket_bind_and_listen(8001);
	ASSERT_TRUE(sock != -1);
	
	ROBOT_JOIN();
	
	int accepted = accept(sock, 8001);
	ASSERT_TRUE(accepted != -1);
	char * send_msg = (char *)"Hello, world!";
	int sent = send(accepted, send_msg, strlen(send_msg), 0); 
	ASSERT_TRUE(sent == (int)(strlen(send_msg)));
	char * recv_msg = (char *)malloc(strlen(send_msg) + 1);
	int recvd = recv(accepted, recv_msg, strlen(send_msg), 0);
	ASSERT_TRUE(recvd == (int)(strlen(send_msg)));
	*(recv_msg + recvd) = '\0';
	ASSERT_STREQ(send_msg, recv_msg);
	int closed = close(accepted);
	ASSERT_TRUE(closed != -1);
	closed = close(sock);
	ASSERT_TRUE(closed != -1);
	free(recv_msg);
}

/*
** Accepts connection on given port using socket sock
*/
int accept(int sock, in_port_t port){
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t len = sizeof(sockaddr);
	int accepted = accept(sock, (struct sockaddr *)&sockaddr, &len);
	if(accepted == -1){
		perror("accept");
		return -1;
	}
	return accepted;
}

/*
** Opens socket on given port and connects
*/
int socket_bind_and_listen(in_port_t port){
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		perror("socket");
		return -1;
	}
	int so_reuseaddr = 1;
	int opt = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
	if(opt == -1){
		perror("setsockopt");
		return -1;
	}
	
	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int bound = bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	if(bound == -1){
		perror("bind");
		return -1;
	}
	
	int listening = listen(sock, 5);
	if(listening == -1){
		perror("listening");
		return -1;
	}
		
	return sock;
}

/*
** Simple echo test
** Receives message "Testing...123" and sends it back
*/
void alt_echo_test(){
	ROBOT_JOIN();
	
	int sock = socket_connect(8001);	
	ASSERT_TRUE(sock != -1);
	char * expected_msg = (char *)"Testing...123";
	char * recv_msg = (char *)malloc(strlen(expected_msg) + 1);
	int recvd = recv(sock, recv_msg, strlen(expected_msg), 0);
	ASSERT_TRUE(recvd == (int)(strlen(expected_msg)));
	*(recv_msg + recvd) = '\0';
	ASSERT_STREQ(expected_msg, recv_msg);
	int sent = send(sock, recv_msg, strlen(recv_msg), 0);
	ASSERT_TRUE(sent == (int)(strlen(recv_msg)));
	int closed = close(sock);
	ASSERT_TRUE(closed != -1);
	free(recv_msg);
}

/*
** Simple echo test
** Sends message "Hello, world!" and receives it back
*/
void echo_test(){
	ROBOT_JOIN();
	
	int sock = socket_connect(8001);
	ASSERT_TRUE(sock != -1);
	char * send_msg = (char *)"Hello, world!";
	int sent = send(sock, send_msg, strlen(send_msg), 0); 	
	ASSERT_TRUE(sent == (int)(strlen(send_msg)));
	char * recv_msg = (char *)malloc(strlen(send_msg) + 1);
	int recvd = recv(sock, recv_msg, strlen(send_msg), 0);
	ASSERT_TRUE(recvd == (int)(strlen(send_msg)));
	*(recv_msg + recvd) = '\0';
	ASSERT_STREQ(send_msg, recv_msg);
	int closed = close(sock);
	ASSERT_TRUE(closed != -1);
	free(recv_msg);
}

/*
** Opens socket on given port and connects
*/
int socket_connect(in_port_t port){
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		perror("socket");
		return -1;
	}

	struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int connected = connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	if(connected == -1){
		perror("connect");
		return -1;
	}
		
	return sock;
}
