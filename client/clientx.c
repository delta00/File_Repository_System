#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*for getting file size using stat()*/
#include<sys/stat.h>

/*for sendfile()*/
#include<sys/sendfile.h>

/*for O_RDONLY*/
#include<fcntl.h>

/*for signal function*/
#include<signal.h>

void sigproc(){ /*disable 'ctrl-c' to quit */
signal(SIGINT, sigproc);
printf("'\nCtrl-c' is disabled\n");
printf("Enter a choice:");
}

int main(int argc, char ** argv)
{
        int port;
        struct stat obj;
        int sock = -1;
        struct sockaddr_in address;
        struct hostent * host;
        int choice;
        char buf[100], command[5], filename[20], *f;
        int k, size, status;
        int filehandle;
        int len;

        /* checking commandline parameter */
        if (argc != 4)
        {
                printf("usage: %s <hostname> <port> <name>\n", argv[0]);
                return -1;
        }

        /* obtain port number */
        if (sscanf(argv[2], "%d", &port) <= 0)
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

        /* connect to server */
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        host = gethostbyname(argv[1]);
        if (!host)
        {
                fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
                return -4;
        }
        memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);
        if (connect(sock, (struct sockaddr *)&address, sizeof(address)))
        {
                fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
                return -5;
        }

        int i = 1;
        signal(SIGINT,sigproc); /* call signal function */
        while(1)
        {

            printf("\n***Welcome to C# File Repository System***\n");
            printf("\nMain Menu:\n1- Download\n2- Upload\n3- Current Path\n4- List\n5- Change Path\n6- Register\n7- Quit\n");
            printf("Enter a choice:");
            scanf("%d", &choice);
        switch(choice)
        {
        case 1:
          printf("\n***Download File***\n");
          printf("Enter filename to Download: ");
          scanf("%s", filename);
          strcpy(buf, "get ");
          strcat(buf, filename);
          send(sock, buf, 100, 0);
          recv(sock, &size, sizeof(int), 0);
            if(!size)
            {
              printf("No such file on the remote directory\n\n");
                break;
            }
            f = malloc(size);
            recv(sock, f, size, 0);
            printf("Downloading....\n\n");
            while(1)
            {
              filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
              if(filehandle == -1)
                {
                  sprintf(filename + strlen(filename), "%d", i);//needed only if same directory is used for both server and client
                }
              else break;
            }
            write(filehandle, f, size);
            close(filehandle);
            printf("Download completed\n\n");
            break;

        case 2:
          printf("\n***Upload File***\n");
          printf("Enter filename to put to server: ");
          scanf("%s", filename);
          filehandle = open(filename, O_RDONLY);
          if(filehandle == -1)
            {
              printf("No such file on the local directory\n\n");
              break;
            }
          printf("Uploading....\n\n");
          strcpy(buf, "put ");
          strcat(buf, filename);
          send(sock, buf, 100, 0);
          stat(filename, &obj);
          size = obj.st_size;
          send(sock, &size, sizeof(int), 0);
          sendfile(sock, filehandle, NULL, size);
          recv(sock, &status, sizeof(int), 0);
          if(status)
            printf("File stored successfully\n");
          else
            printf("File failed to be stored to remote machine\n");
          break;

        case 3:
          strcpy(buf, "pwd");
          send(sock, buf, 100, 0);
          recv(sock, buf, 100, 0);
          printf("\n***Current Path***\n");
          printf("Path: %s\n", buf);
          break;

        case 4:
          printf("\n***List of Current Directory***\n");
          strcpy(buf, "ls");
          send(sock, buf, 100, 0);
          recv(sock, &size, sizeof(int), 0);
          f = malloc(size);
          recv(sock, f, size, 0);
          filehandle = creat("temp.txt", O_WRONLY);
          system("chmod 666 temp.txt");
          write(filehandle, f, size);
          close(filehandle);
          printf("The remote directory listing is as follows:\n");
          printf("-------------------------------------------\n");
          system("cat temp.txt");
          printf("-------------------------------------------\n");
          break;

        case 5:
          strcpy(buf, "cd ");
          printf("\n***Change Path***\n");
          printf("Enter the path to change the remote directory: ");
          scanf("%s", buf + 3);
          send(sock, buf, 100, 0);
          recv(sock, &status, sizeof(int), 0);
          if(status)
            printf("Remote directory successfully changed\n");
          else
            printf("Remote directory failed to change\n");
          break;

        case 6:
            strcpy(buf, "register");
            send(sock, buf, 100, 0);
            char username[BUFSIZ],  password1[BUFSIZ],    /* Buffers for user input and comparison    */
                    password2[BUFSIZ], *pbuf;

            /* Get the username */
            printf("Username: ");
            scanf("%s", username);
            strcpy(buf, username);
            strcat(buf, ":");

             do {

                    /* Get the password */
                    pbuf = getpass("Password: ");

                    /* Copy to a "stable" pointer */
                    sprintf(password1, "%s", pbuf);

                    /* Get the password */
                    pbuf = getpass("Enter Again: ");

                    /* Copy to a "stable" pointer */
                    sprintf(password2, "%s", pbuf);

                    /* See if the passwords are the same */
                    if(strcmp(password1, password2) != 0)
                        printf("\nPasswords do not match!\nTry again.\n\n");

                } while(strcmp(password1, password2) != 0);
                    strcat(buf, password2);
                    send(sock, buf, 100, 0);
                    recv(sock, &status, sizeof(int), 0);
                    if(status)
                        printf("User successfully registered\n");
                    else
                        printf("Error. Registration failed.\n");
            break;

        case 7:
            strcpy(buf, "quit");
            send(sock, buf, 100, 0);
            len = strlen(argv[3]);
            write(sock, &len, sizeof(int));
            write(sock, argv[3], len);
            printf("Successfully Disconnected.\nQuitting..\n");

            /* close socket */
            close(sock);

            return 0;


        }
    }

}
