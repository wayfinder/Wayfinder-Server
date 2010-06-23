/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * httpdwrap.c
 *
 * Used to enable MC2's HttpServer to use privileged ports for its service.
 *
 * This small wrapper runs as root: (setuid)
 *    * opens port 80 and 443
 *      * changes uid to a specified one
 *      * drops root privileges
 *      * adds any environment variables passed as parameters (eg LD_LIBRARY_PATH)
 *      * executes the program passed as a parameter and passes the file descriptor numbers for the two sockets
 *
 * Tested on so far:
 *    * Linux, Red Hat 6.2
 *    * Linux, Red Hat 7.1
 *    * Solaris 8 x86
 *
 **/

#define UID             "mc2"
#define GID             "mc2"
#define LISTEN_ADDRESS  "ANY"

/* No user serviceable parts below */

#define HTTP_ARG      "--httpfd"
#define HTTPS_ARG     "--httpsfd"

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include <grp.h>
#include <pwd.h>
#include <sys/types.h>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int httpFD;
int httpsFD;
int i;
char* listenAddress;

void printUsage()
{
   printf("usage:\n");
   printf("httpdwrap [--putenv ENV=VALUE] [--listen IP] <path to HttpServer> [args to httpd]\n");
   printf("\nExamples: httpdwrap ./HttpServer\n");
   printf("          httpdwrap --putenv LD_LIBRARY_PATH=/usr/local/foo/lib ./HttpServer\n");
}

int createSocket(char* listenAddress, short port)
{
   int sock;
   struct sockaddr_in sockaddr;
   int val;

   memset(&sockaddr, 0, sizeof(struct sockaddr_in));
   sockaddr.sin_family = AF_INET; 
   sockaddr.sin_port = htons(port);

   if (strcmp(listenAddress, "ANY") == 0) {
      sockaddr.sin_addr.s_addr = INADDR_ANY;
   } else {
      if (inet_aton(listenAddress, &(sockaddr.sin_addr)) == 0) {
         fprintf(stderr, "Error: [createSocket] Invalid address (%s)\n", listenAddress);
         return(-1);
      }
   }

   if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
      return(-1);

   /* Reuse the port as quickly as possible and don't wait for timeouts */
   val = 1;
   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&val, sizeof(val)) < 0 ) {
      perror("Fatal Error: [createSocket] Couldn't set socket options (SO_REUSEADDR)");
      return(-1);
   }

   if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) < 0) {
      close(sock);
      return(-1);
   } 

   /* we were successful, return fd number */

   return(sock);
}

void openSockets(char* listenAddress)
{

   if ( (httpFD = createSocket(listenAddress, 80)) < 0 ) {
      fprintf(stderr, "Fatal error: [openSockets] Couldn't open port 80 !\n");
      exit(-1);
   }

   if ( (httpsFD = createSocket(listenAddress, 443)) < 0) {
      fprintf(stderr, "Fatal error: [openSockets] Couldn't open port 443!\n");
      exit(-1);
   }
}

void dropRootPrivs(char* uid, char* gid)
{
   struct group *gr;
   struct passwd *pw;

   if ((gr = getgrnam(gid)) == NULL) {
      fprintf(stderr, "Fatal error: [changeUid] Group %s doesn't seem to exist\n", GID); 
      exit(-1);
   }

   /* change group and make sure we only belong to the group "gid" */
   setgid(gr->gr_gid);
   setegid(gr->gr_gid);
   setgroups(1, &(gr->gr_gid) );

   if ((pw = getpwnam(uid)) == NULL) {
      fprintf(stderr, "Fatal error: [changeUid] User %s doesn't seem to exist\n", UID); 
      exit(-1);
   }

   /* change user */
   setuid(pw->pw_uid);
   seteuid(pw->pw_uid);
}

void handleArgs(int argc, char* argv[]) 
{
   while (strncmp(argv[i], "--", 2) == 0) {
      if (strcmp(argv[i], "--putenv") == 0) {
         i++;
         if (putenv(argv[i]) < 0) {
            fprintf(stderr, "Fatal Error: [execProgram] putenv: '%s' failed!\n", argv[i]);
            exit(-1);
         }
         i++; /* eat argument */
      } else if (strcmp(argv[i], "--listen") == 0) {
         i++;
         listenAddress = argv[i];
         i++; /* eat argument */
      } else {
         fprintf(stderr, "Unknown argument: %s\n", argv[i]);
	 printUsage();
      }
   }
}

void execProgram(int argc, char* argv[])
{
   char**   newargv;
   int   newargc;
   char   httpFDString[255], httpsFDString[255];

   sprintf(httpFDString, "%s=%i", HTTP_ARG, httpFD);
   sprintf(httpsFDString, "%s=%i", HTTPS_ARG, httpsFD);

   newargc = argc - i + 3; // remove args to ourself, add space for new args and NULL
   newargv = malloc(sizeof(char*) * newargc);
   memcpy(newargv, argv + i, newargc * sizeof(char*));

   newargv[newargc - 3] = httpFDString;
   newargv[newargc - 2] = httpsFDString;
   newargv[newargc - 1] = NULL;   /* execv() requires a NULL-terminated argv */

   execv(newargv[0], newargv);
   perror("Fatal Error: [execProgram] execv failed");
}

int main(int argc, char* argv[])
{
   if (argc < 2) {
      printUsage();
      exit(-1);
   }
   i = 1;
   listenAddress = LISTEN_ADDRESS;

   handleArgs(argc, argv);
   openSockets(listenAddress);
   dropRootPrivs(UID, GID);
   execProgram(argc, argv);

   return(-1);
}
