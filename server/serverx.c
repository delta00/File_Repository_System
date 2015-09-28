#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <string.h>

/*for getting file size using stat()*/
#include<sys/stat.h>

/*for sendfile()*/
#include<sys/sendfile.h>

/*for O_RDONLY*/
#include<fcntl.h>

typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;


void * process(void * ptr)
{
	char * buffer;
	int len;
	connection_t * conn;
	long addr = 0;

	if (!ptr) pthread_exit(0);
	conn = (connection_t *)ptr;

	/* read length of message */
	read(conn->sock, &len, sizeof(int));
	if (len > 0)
	{
		addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
		buffer = (char *)malloc((len+1)*sizeof(char));
		buffer[len] = 0;

		/* read message */
		read(conn->sock, buffer, len);

		/* print message */
		printf("User: %s has been disconnected. IP: %d.%d.%d.%d\n",
                buffer,
			(int)((addr      ) & 0xff),
			(int)((addr >>  8) & 0xff),
			(int)((addr >> 16) & 0xff),
			(int)((addr >> 24) & 0xff));
		free(buffer);
	}

	/* close socket and clean up */
	close(conn->sock);
	free(conn);
	pthread_exit(0);
}

int main(int argc, char ** argv)
{
	int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;
    char buf[100], command[5], filename[20];
    int k, i, size, len, c;
    int test = 0;
    int filehandle;
    struct stat obj;
    char * buffer;
	char username[BUFSIZ], *password;
	/* check for command line arguments */
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/* listen on port */
	if (listen(sock, 5) < 0)
	{
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	printf("%s: ready and listening\n", argv[0]);

	while(1)
    {

    /* accept incoming connections */
    connection = (connection_t *)malloc(sizeof(connection_t));
    connection->sock = accept(sock, &connection->address, &connection->addr_len);

    int loop = 1;

	do
	{

        recv(connection->sock, buf, 100, 0);
        sscanf(buf, "%s", command);
		if (connection->sock <= 0)
		{
			free(connection);
		}

    else if(!strcmp(command, "login"))
         {
                recv(connection->sock, buf, 100, 0);
                char username[BUFSIZ], *password;  /* User input buffers         */
                char pbuf[BUFSIZ];                  /* File input buffer          */
                char *user_file, *pass_file;       /* Buffers for values in file */
                char passfile[BUFSIZ];             /* Filename buffer            */
                FILE *infile;                      /* File handle                */
                strcpy(passfile, "pass.txt");

                sscanf(buf, "%s", username);

                recv(connection->sock, buf, 100, 0);
                sscanf(buf, "%s", password);

                /* Open the file */
                if((infile = fopen(passfile, "r")) == NULL){

                printf("\nFile error!\nAborting...\n");

                } else {

                /* Loop throught the file */
                if((infile = fopen(passfile, "r")) == NULL){

                    printf("\nFile error!\nAborting...\n");

                } else {

                /* Loop throught the file */
                while (!feof(infile)) {

                /* Initialize with empty string */
                pbuf[0] = '\0';

                /* Read in one line */
                    fscanf(infile, "%s", pbuf);

                /* If it's an empty line, continue */
                if(strlen(pbuf) == 0) continue;

                /* Point to the buffer */
                user_file = pbuf;

                /* Point to the delimiter in the buffer */
                pass_file = strchr(pbuf, ':');

                /* Change the delimiter to a nul character */
                pass_file[0] = '\0';

                /* Move to the next character */
                pass_file++;

                /* See if this matches the name the user entered */
                if(strcmp(user_file, username) == 0){

                /* See if the passwords match */
                if(strcmp(pass_file, password) == 0){

                printf("Correct password...\n");
                c = 1;

                } else {

                printf("Invalid password!\n\n");
                 c = 0;

                }  /* End if */

                /* We found what we wanted; get out of loop */


                }  /* End if */

                }  /* End while */

            }  /* End if */

            /* Close the file */
            fclose(infile);
            send(connection->sock, &c, sizeof(int), 0);

         }
         }

	else if(!strcmp(command, "register"))
        {
            recv(connection->sock, buf, 100, 0);
            char passfile[BUFSIZ];                        /* Buffer for filename                      */
            FILE *outfile;
            filehandle = creat("pass.txt", O_WRONLY);
            strcpy(passfile, "pass.txt");
            /* Open the output file for append */
            if((outfile = fopen(passfile, "a+")) == NULL){

                    printf("\nFile error!\nAborting...\n");
                    c = 0;

            } else {

                    /* Print the record to the file */
                    fprintf(outfile, "%s\n", buf);
                    c = 1;
                    }  /* End if */

                    /* Close the file */
                send(connection->sock, &c, sizeof(int), 0);
                fclose(outfile);
        }

      else if(!strcmp(command, "ls"))
	{
	  system("ls >temps.txt");
	  i = 0;
	  stat("temps.txt",&obj);
	  size = obj.st_size;
	  send(connection->sock, &size, sizeof(int),0);
	  filehandle = open("temps.txt", O_RDONLY);
	  sendfile(connection->sock,filehandle,NULL,size);
	}
      else if(!strcmp(command,"get"))
	{
	  sscanf(buf, "%s%s", filename, filename);
	  stat(filename, &obj);
	  filehandle = open(filename, O_RDONLY);
	  size = obj.st_size;
	  if(filehandle == -1)
	      size = 0;
	  send(connection->sock, &size, sizeof(int), 0);
	  if(size)
	  sendfile(connection->sock, filehandle, NULL, size);

	}
      else if(!strcmp(command, "put"))
        {
	  int c = 0, len;
	  char *f;
	  sscanf(buf+strlen(command), "%s", filename);
	  recv(connection->sock, &size, sizeof(int), 0);
	  i = 1;
	  while(1)
	    {
	      filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
	      if(filehandle == -1)
		{
		  sprintf(filename + strlen(filename), "%d", i);
		}
	      else
		break;
	    }
	  f = malloc(size);
	  recv(connection->sock, f, size, 0);
	  c = write(filehandle, f, size);
	  close(filehandle);
	  send(connection->sock, &c, sizeof(int), 0);
        }
      else if(!strcmp(command, "pwd"))
	{
	  system("pwd>temp.txt");
	  i = 0;
          FILE*f = fopen("temp.txt","r");
          while(!feof(f))
            buf[i++] = fgetc(f);
          buf[i-1] = '\0';
	  fclose(f);
          send(connection->sock, buf, 100, 0);
	}
      else if(!strcmp(command, "cd"))
        {
          if(chdir(buf+3) == 0)
	    c = 1;
	  else
	    c = 0;
          send(connection->sock, &c, sizeof(int), 0);
        }


      else if(!strcmp(command, "quit"))
	{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
			loop = 0;
	}
		else
		{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);

		}
		}while(loop);
	}
	return 0;
}
