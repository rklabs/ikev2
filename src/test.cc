/*
 * Copyright (C) 2014 Raju Kadam <rajulkadam@gmail.com>
 *
 * IKEv2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IKEv2 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//g++ -std=c++11 test.cc -lpthread -Wl,--no-as-needed -o echo
#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

int ipv4_client(char *argv[], int count) {
    int sk;
    struct sockaddr_in server;
    struct sockaddr_in client;
    struct hostent *hp;
    char buf[4] = "abc";
    int buf_len;
    int n_sent;
    int n_read;
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;
    std::chrono::duration<double> elapsed_seconds;

    start = std::chrono::system_clock::now();

    if ((sk = socket(AF_INET, SOCK_DGRAM, 0 )) < 0) {
        printf("Problem creating socket\n");
        exit(1);
    }

    server.sin_family = AF_INET;
    if ((hp = gethostbyname(argv[1]))==0) {
        printf("Invalid or unknown host\n");
        exit(1);
    }

    memcpy(&server.sin_addr.s_addr, hp->h_addr, hp->h_length);

    /* establish the server port number - we must use network byte order! */
    server.sin_port = htons(atoi(argv[2]));

    /* read everything possible */
    buf_len = sizeof(buf);
    //printf("Got %d bytes from stdin - sending...\n",buf_len);

    for (int i = 0 ; i < count ; i++) {

        /* send it to the echo server */

        n_sent = sendto(sk, buf ,buf_len, 0, (struct sockaddr*)&server, sizeof(server));

        if (n_sent < 0) {
            perror("Problem sending data");
            exit(1);
        }

        if (n_sent != buf_len) {
            printf("Sendto sent %d bytes\n",n_sent);
        }

        /* Wait for a reply (from anyone) */
        socklen_t len;
        n_read = recvfrom(sk, buf, buf_len, 0, (struct sockaddr*)&client, &len);
        if (n_read < 0) {
            perror("Problem in recvfrom");
            exit(1);
        }

        buf[n_read] = '\0';
    }

    //printf("Got back %d bytes, buf %s\n",n_read, buf);

    close(sk);

    end = std::chrono::system_clock::now();

    elapsed_seconds = end - start;
    std::cout << "IPv4: time taken :" << elapsed_seconds.count() << "s\n";

    return 0;
}

int ipv6_client(char *argv[], int count) {
    int sk;
    struct sockaddr_in6 client;
    struct sockaddr_in6 server;
    char buf[4] = "abc";
    int buf_len;
    int n_sent;
    int n_read;
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;
    std::chrono::duration<double> elapsed_seconds;

    // Set random seed
    std::srand(std::time(0));

    /* Make sure we have the right number of command line args */

    start = std::chrono::system_clock::now();


    if ((sk = socket(AF_INET6, SOCK_DGRAM, 0 )) < 0) {
        printf("Problem creating socket\n");
        exit(1);
    }

    server.sin6_family = AF_INET6;

    inet_pton(AF_INET6, "::", &server.sin6_addr);

    /* establish the server port number - we must use network byte order! */
    server.sin6_port = htons(atoi(argv[2]));

    /* read everything possible */
    buf_len = sizeof(buf);
    //printf("Got %d bytes from stdin - sending...\n",buf_len);

    for (int i = 0 ; i < count ; i++) {
        /* send it to the echo server */
        n_sent = sendto(sk, buf, buf_len, 0, (struct sockaddr*)&server, sizeof(server));

        if (n_sent < 0) {
            perror("Problem sending data");
            exit(1);
        }

        if (n_sent != buf_len) {
            printf("Sendto sent %d bytes\n",n_sent);
        }

        socklen_t len;
        /* Wait for a reply (from anyone) */
        n_read = recvfrom(sk, buf, buf_len, 0, (struct sockaddr*)&client, &len);
        if (n_read < 0) {
            perror("Problem in recvfrom");
            exit(1);
        }

        buf[n_read] = '\0';
    }
    //printf("Got back %d bytes, buf %s\n",n_read, buf);

    close(sk);


    end = std::chrono::system_clock::now();

    elapsed_seconds = end - start;
    std::cout << "IPv6: time taken :" << elapsed_seconds.count() << "s\n";

    return 0;
}

int main( int argc, char *argv[] ) {
    std::vector<std::thread> ipv4Threads;
    std::vector<std::thread> ipv6Threads;

    if (argc != 5) {
        printf("Usage: %s <server name> <port number> <packet_count> <threads>\n",argv[0]);
        exit(0);
    }

    // Set random seed
    std::srand(std::time(0));

    int connectionCount = atoi(argv[3]);
    int threadCount = atoi(argv[4]);

    for(int _ = 0 ; _ < threadCount ; _++) {
        ipv4Threads.push_back(std::thread(ipv4_client, argv, connectionCount));
        ipv6Threads.push_back(std::thread(ipv6_client, argv, connectionCount));
    }

    for (auto & iter : ipv4Threads) {
        iter.join();
    }

    for (auto & iter : ipv6Threads) {
        iter.join();
    }

    std::cout << "All threads completed!" << std::endl ;

    return 0;
}
