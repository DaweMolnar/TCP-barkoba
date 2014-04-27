#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 31231

struct tipp 
{
	char question;
	int value;
};


int main()
{
  char sName[255];
  char* line="---------------------------------";
	gethostname(sName, 255);
	struct hostent *host_entry;
	host_entry=gethostbyname(sName);
	char * sIP = inet_ntoa (*(struct in_addr *)*host_entry-> h_addr_list);
  printf ("Server IP:%s\nServer Starting...\n%s\n", sIP, line);
  int rounds=0;	
  int server;
  size_t length;
  struct sockaddr_in server_name, client_name;
  struct tipp client_tipp;
  char buffer[100];
  int randnro=-1;
  int client_count=0;

  server = socket( AF_INET, SOCK_STREAM, 0 );
  if( server == -1 ) {
  	 perror( "opening stream socket" );
  	 return 1;
  }

  server_name.sin_family = AF_INET;
  server_name.sin_addr.s_addr = INADDR_ANY;
  server_name.sin_port = htons(PORT);
  memset(&(server_name.sin_zero),'\0',8);
  if( bind( server, (struct sockaddr *) &server_name, sizeof server_name ) == -1 ) 
  {
    	perror( "binding stream socket" );
    	return 2;
  }

  length = sizeof(server_name);
  if( getsockname( server, (struct sockaddr *) &server_name, &length) == -1 ) {
    	perror( "getting socket name" );
    	return 3;
  }

  if(listen(server,10)==-1){
    perror( "start listening" );
    return 4;
  }
  
  fd_set master, slave;
  FD_ZERO(&master);                
	FD_ZERO(&slave);
	FD_SET(server, &master);
  int index_max = server;
  length = sizeof(client_name);
  
  srand( time(NULL) );
  randnro= (rand()%100) +1;
  printf("New round started. Goal: %d\n", randnro);
  buffer[0]='\0';
          
  do
  {
      if(client_count==0 && strcmp(buffer,"end")==0){
        rounds++;
        if(rounds==2) break;
        srand( time(NULL) );
        randnro= (rand()%100) +1;
        printf("New round started. Goal: %d\n", randnro);
        buffer[0]='\0';
      } 
      slave = master;
  		if(select(index_max+1, &slave, NULL, NULL, NULL) == -1)
  		{
  			perror("select");
  			return -1;
  		}	
  		
  		int i;
      for(i=0; i<= index_max; i++) 
  		{
  			 if(FD_ISSET(i, &slave))
  			 {
            if(i==server) {
                if(strcmp(buffer,"end")==0) continue;
                length = sizeof(client_name);
                int accept_fd = accept(server, (struct sockaddr *)&client_name, &length); 
    				    if(accept_fd==-1)
    				    {  
    					     perror("accept");
    				    }
    				    else
    				    { 
    				      FD_SET(accept_fd, &master);
    					    if(accept_fd>index_max)
    					    {
    						    index_max= accept_fd;
    					    }
    					    printf("New connection from %s:%d on socket %d\n",
    						    inet_ntoa(client_name.sin_addr), ntohs(client_name.sin_port), accept_fd);
    						  client_count++;
    				    }
            }else{
                if(strcmp(buffer,"end")==0)
                {
                  send(i, buffer, sizeof("end"),0);
                  FD_CLR(i, &master);
                  client_count--;
                  close(i);
                  if(client_count==0) break;
                  else continue;
                } 
                int nbytes = recv(i, &client_tipp, sizeof(struct tipp),0);
                if(nbytes == 0)
                {
						      printf("selectserver: socket %d hung up\n", i);
						      close(i);
                  FD_CLR(i, &master);
                  client_count--;
                  close(i);
                  if(client_count==0) break;
                  else continue;
                  continue;
					      }
					      else if(nbytes==-1){
						      printf("selectserver: socket %d hung up\n", i);
                  close(i);
                  FD_CLR(i, &master);
                  client_count--;
                  close(i);
                  if(client_count==0) break;
                  else continue;                  
					      }
                else
                {
                  if(client_tipp.question=='<')
                  {
                    if(randnro<ntohl(client_tipp.value))
                    {
                      send(i, "yes",sizeof("yes"),0);
                    }
                    else
                    {
                      send(i, "no",sizeof("no"),0);                    
                    }
                    
                    printf("%d%c%d\n" , randnro, client_tipp.question,ntohl(client_tipp.value)  );
                    continue;
                  }
                  
                  if(client_tipp.question=='>')
                  {
                    if(randnro>ntohl(client_tipp.value))
                    {
                      send(i, "yes",sizeof("yes"),0);
                    }
                    else
                    {
                      send(i, "no",sizeof("no"),0);                    
                    }
                    printf("%d%c%d\n" , randnro, client_tipp.question,ntohl(client_tipp.value)  );
                    continue;
                  }
                  
                  if(client_tipp.question=='=')
                  {
                    if(ntohl(client_tipp.value)==randnro)
                    {
                      send(i, "win",sizeof("win"),0);
                      client_count--;
                      close(i);
                      FD_CLR(i, &master);
                      strcat(buffer,"end\0");
                    }
                    else
                    {
                      send(i, "no",sizeof("no"),0);                    
                    }
                    printf("%d%c%d\n" , randnro, client_tipp.question,ntohl(client_tipp.value)  );
                    
                    continue;
                  }                  
                }                
            }  			   
        }
    }
  }while(1);
  
  close(server);
  return 0;
}
