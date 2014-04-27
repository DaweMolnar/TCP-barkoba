#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct tipp 
{
	char question;
	int value;
};

int main( int argc, char *argv[])
{
  if(argc!=3)
  {
    printf("Usage: %s server-address port-number\n", argv[0]);
    return -1;    
  }
  
  char line[10];
  int client;
  struct sockaddr_in name;
  struct hostent *hp;
  struct tipp client_tipp, server_response;
  char buffer[100];
  
  int query,nro,rv; 
  char c; 
  

  if((client = socket( AF_INET, SOCK_STREAM, 0 )) == -1 ) {
  	 perror( "Creating socket" );
  	 return 1;
  }
  
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = INADDR_ANY;
  name.sin_port = 0;

  if( bind( client, (struct sockaddr *) &name, sizeof name ) == -1 ) {
  	 perror( "binding failed" );
  	 return 2;
 }

  hp = gethostbyname(argv[1]);
  if( hp == (struct hostent *) 0 ) {
  	 fprintf( stderr, "%s: unknown host name\n", argv[1] );
  	 return 2;
	}

  memcpy( (void *) &name.sin_addr, (void *) hp->h_addr, hp->h_length );
  name.sin_family = AF_INET;
  name.sin_port = htons( atoi ( argv[2] ));
  
  if(connect(client, (struct sockaddr*) &name, sizeof(name))==-1)
    perror("Establishing connection");
  int tipp=0;
  while(tipp<5)
  {
    fgets(line,10,stdin);
    rv=sscanf(line,"%c%d\n",&query,&nro);
    if(rv!=2){
       printf("Bad input!\n");
       tipp++;
       continue;
    }
    tipp++;
  	  
    client_tipp.question=query;
  	client_tipp.value=htonl(nro);
    
    if( send(client, &client_tipp, sizeof (struct tipp), 0) == -1 )
        perror( "Sending data failed" );
  	
    if( (rv=recv(client, buffer, sizeof(buffer), 0)) == -1 )
  		  perror( "Recieving data failed" );
  		
  	buffer[rv]='\0';
  	printf("Server says: %s\n",buffer);
  	
  	if(strcmp(buffer,"end")==0 || strcmp(buffer,"win")==0) break;
  	tipp++;
  }
  
  close(client);
  return 0;
}
