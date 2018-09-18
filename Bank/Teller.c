/*socket tcp client*/
#include <sys/stat.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <netdb.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <unistd.h>

int main(int argc, char *argv[])  
{  
    int port = atoi("8888");
    struct hostent *server = gethostbyname("localhost");

    //clientscoket for connecting to server  
    int clientSocket;  
      
    struct sockaddr_in serverAddr;  
    char sendbuf[BUFSIZ];  
    char recvbuf[BUFSIZ];  
    int iDataNum;  
    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
    {  
        perror("socket");  
        return -1;  
    }  
  
    serverAddr.sin_family = AF_INET;  //ipv4
    serverAddr.sin_port = htons(port);

    /*connect to server at particular port*/
    if(connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)  
    {  
        perror("connect");  
        return -1;  
    }
    //printf("connect with destination host...\n");  
    
    INIT:
    ;
    int logged = 0;
    char account[BUFSIZ];
    while(logged == 0) // main menu
    {
        printf("Supportted operations:\n");
        printf("1. Create Teller account\n");
        printf("2. Create Customer account\n");
        printf("3. Account login\n");
        printf("4. Exit\n");
        printf("Enter your operation: ");

        memset (sendbuf, 0, BUFSIZ);
        scanf("%s", sendbuf);
        int choice = atoi(sendbuf);
        send(clientSocket, sendbuf, strlen(sendbuf), 0); //send choice to server first

        switch(choice) 
        {
            case 1: // create teller
            memset (sendbuf, 0, BUFSIZ);
            printf("Please enter the teller name: ");
            scanf("%s", sendbuf);
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            
            memset (sendbuf, 0, BUFSIZ);
            char *pw;
            pw = getpass("Please enter your password:\n");
            strcpy(sendbuf, pw);
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            
            memset (recvbuf, 0, BUFSIZ);
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
            recvbuf[iDataNum] = '\0';
            if (strcmp(recvbuf, "Success") == 0) {
                printf("The account is created!");
            } else {
                printf("System Error. Please try later. Sorry for the inconvenience. \n");
            	close(clientSocket);  
            	return 0; 
            }
            break;

            case 2: // create account
            printf("Please enter your first name: ");
        	memset (sendbuf, 0, BUFSIZ);
        	scanf("%s", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0);

        	printf("Please enter your last name: ");
        	memset (sendbuf, 0, BUFSIZ);
        	scanf("%s", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0);

        	printf("Please enter your email: ");
        	memset (sendbuf, 0, BUFSIZ);
        	scanf("%s", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0);
        	
        	char *pwd;
        	pwd = getpass("Please enter your password: \n");
        	memset (sendbuf, 0, BUFSIZ);
        	strcpy(sendbuf, pwd);
        	//printf("password: %s\n", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0); // send password
        	
        	//receive teller name from server
        	memset (sendbuf, 0, BUFSIZ);
        	memset (recvbuf, 0, BUFSIZ);
        	iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);  
        	recvbuf[iDataNum] = '\0';
        	printf("Teller list: %s\n", recvbuf);
        	printf("Please choose one teller from the above list: ");
        	scanf("%s", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0); // send teller to server
        	
        	printf("Please enter the account type - saving or checking: "); // choose type
        	memset (sendbuf, 0, BUFSIZ);
        	scanf("%s", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0);
        	
        	//receive account number
        	printf("Sign up success!\n");
        	memset (recvbuf, 0, BUFSIZ);
        	iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);  
        	recvbuf[iDataNum] = '\0';
        	printf("Your account number is: %s\n", recvbuf);
        	printf("You can use your account number and password to log in!\n");
        	break;
        	
            case 3: // log in 
            memset (sendbuf, 0, BUFSIZ);
        	printf("Please enter your bank account: ");
        	scanf("%s", sendbuf);
        	strcpy(account, sendbuf); 
        	//printf("account: %s\n", account);
        	//printf("Send %s\n", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0); /*send to server*/

        	memset (sendbuf, 0, BUFSIZ);
        	char *pswd;
        	pswd = getpass("Please enter your password:\n");
        	strcpy(sendbuf, pswd);
        	//printf("Send %s\n", sendbuf);
        	send(clientSocket, sendbuf, strlen(sendbuf), 0); /*send to server*/    

        	/*receive the result from server*/
        	iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);  
        	recvbuf[iDataNum] = '\0';
        	//printf("received %s\n", recvbuf);
        	if (strcmp(recvbuf, "1") == 0) // customer login
        	{
        	    logged = 1;
        	    memset (recvbuf, 0, BUFSIZ);
            	iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);  
            	recvbuf[iDataNum] = '\0';
            	strcpy(account, recvbuf);
            	printf("Welcome, %s\n", account);
        	} 
        	else if (strcmp(recvbuf, "2") == 0) // teller login
        	{
        	    logged = 2;
            	printf("Welcome, %s\n", account);
        	} 
        	else if (strcmp(recvbuf, "System error") == 0) 
        	{
            	printf("System Error. Please try later. Sorry for the inconvenience. \n");
            	close(clientSocket);  
            	return 0;
        	} else {
            	printf("Wrong account or password!\n");
        	}        	
            break;

            case 4:
            printf("Have a nice day! Bye!\n");
            exit(1);
        }
    }

    int choice;
    char *operation;
    while(logged == 1) // customer menu
    { 
        printf("Supportted operations:\n");
        printf("1. Account overview\n");
        printf("2. Withdraw money\n");
        printf("3. Deposit money\n");
        printf("4. Transfer money\n");
        printf("5. Account statement\n");
        printf("6. View profile\n");
        printf("7. Update profile\n");
        printf("8. Delete account\n");
        printf("9. Log out\n");

        printf("Enter your operation: ");
        memset (sendbuf, 0, BUFSIZ);
        scanf("%s", sendbuf);
        choice = atoi(sendbuf);
        //printf("%d\n", choice);
        send(clientSocket, sendbuf, strlen(sendbuf), 0);

        switch(choice) {
            case 1:
            memset (recvbuf, 0, BUFSIZ);
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);  
            //recvbuf[iDataNum] = '\0';
            printf("The account balance is $%s\n", recvbuf);
            break;

            case 2:
            printf("Please enter the amount you want to withdraw: $");
            memset (sendbuf, 0, BUFSIZ);
            memset(recvbuf, 0, BUFSIZ);
            scanf("%s", sendbuf);
            send(clientSocket, sendbuf, strlen(sendbuf), 0);

            //receive result from server
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0); 
            printf("%s\n\n", recvbuf);
            break;
            
            case 3:
            printf("Please enter the amount you want to deposit: $");
            memset (sendbuf, 0, BUFSIZ);
            memset(recvbuf, 0, BUFSIZ);
            scanf("%s", sendbuf);
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0); 
            printf("%s\n\n", recvbuf);
            break;

            case 4:
            printf("Please enter the account you want to transfer: ");
            memset (sendbuf, 0, BUFSIZ);
            scanf("%s", sendbuf);
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            printf("Please enter the amount you want to transfer: $");
            memset (sendbuf, 0, BUFSIZ);
            scanf("%s", sendbuf);
            send(clientSocket, sendbuf, strlen(sendbuf), 0);
            
            memset(recvbuf, 0, BUFSIZ);
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0); 
            printf("%s\n\n", recvbuf);
            break;

            case 5:
            memset(recvbuf, 0, BUFSIZ);
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0); 
            if (strcmp(recvbuf, "Notrans") == 0) {
                printf("No recent transaction.\n");
            } else {
                printf("Your recent ten transactions.\n");
                printf("+---------+----------------------+--------+-----------+\n");
                printf("| account | time                 | amount | operation |\n");
                printf("+---------+----------------------+--------+-----------+\n");
                while(1) {
                    memset(recvbuf, 0, BUFSIZ);
                    iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                    if (strcmp(recvbuf, "Done") == 0) break;
                    printf("%s\n", recvbuf);
                    printf("+---------+----------------------+--------+-----------+\n");
                }
            }
            break;
                
            case 6:
                memset(recvbuf, 0, BUFSIZ);
                iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                if (strcmp(recvbuf, "Notrans") == 0) {
                    printf("No recent transaction.\n");
                } else {
                    printf("Your profile is:\n");
                    printf("+--------------+-----------------+-------------+-------------------------+------------+----------+----------------------+-------------+\n");
                    printf("|  account_no  |   First name    |   Last name |         Email           |   Teller   |   Type   | time                 |   balance   |\n");
                    printf("+--------------+-----------------+-------------+-------------------------+------------+----------+----------------------+-------------+\n");
                    while(1) {
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        if (strcmp(recvbuf, "Done") == 0) break;
                        printf("%s\n", recvbuf);
                        printf("+--------------+-----------------+-------------+---------------------+------------+----------+----------------------+-------------+\n");
                    }
                }
                break;
                
            case 7:
                printf("Please enter the option you want to update: \n");
               
                
                printf("1. First name\n");
                printf("2. Last name\n");
                printf("3. Email\n");
                printf("4. Password\n");
                printf("5. Teller\n");
                printf("6. Type\n");
                printf("Enter your operation: ");
                memset (sendbuf, 0, BUFSIZ);
                scanf("%s", sendbuf);
                int option;
                option = atoi(sendbuf);
                //printf("%d\n", choice);
                send(clientSocket, sendbuf, strlen(sendbuf), 0);
                
                switch(option) {
                case 1:
                        printf("Enter your First name: ");
                        memset (sendbuf, 0, BUFSIZ);
                        scanf("%s", sendbuf);
                        send(clientSocket, sendbuf, strlen(sendbuf), 0);
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        printf("%s\n\n", recvbuf);
                        break;
                    case 2:
                        printf("Enter your Last name: ");
                        memset (sendbuf, 0, BUFSIZ);
                        scanf("%s", sendbuf);
                        send(clientSocket, sendbuf, strlen(sendbuf), 0);
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        printf("%s\n\n", recvbuf);
                        break;
                    case 3:
                        printf("Enter your Email: ");
                        memset (sendbuf, 0, BUFSIZ);
                        scanf("%s", sendbuf);
                        send(clientSocket, sendbuf, strlen(sendbuf), 0);
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        printf("%s\n\n", recvbuf);
                        break;
                    case 4:
                        printf("Enter your Password: ");
                        memset (sendbuf, 0, BUFSIZ);
                        scanf("%s", sendbuf);
                        send(clientSocket, sendbuf, strlen(sendbuf), 0);
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        printf("%s\n\n", recvbuf);
                        break;
                    case 5:
                        printf("Enter your Teller: ");
                        memset (sendbuf, 0, BUFSIZ);
                        scanf("%s", sendbuf);
                        send(clientSocket, sendbuf, strlen(sendbuf), 0);
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        printf("%s\n\n", recvbuf);
                        break;
                    case 6:
                        printf("Enter your Type: ");
                        memset (sendbuf, 0, BUFSIZ);
                        scanf("%s", sendbuf);
                        send(clientSocket, sendbuf, strlen(sendbuf), 0);
                        memset(recvbuf, 0, BUFSIZ);
                        iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                        printf("%s\n\n", recvbuf);
                        break;
            
                }
                break;
            
            case 8:
                memset(recvbuf, 0, BUFSIZ);
                iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);
                printf("%s\n\n", recvbuf);
                break;
                
                
            
            case 9:
            printf("Have a nice day! Bye!\n");
            goto INIT;
            break;
        }
    }
    
    while(logged == 2) // teller menu
    { 
        printf("Supportted operations:\n");
        printf("1. View Customer List\n");
        printf("2. Log out\n");

        printf("Enter your operation: ");
        memset (sendbuf, 0, BUFSIZ);
        scanf("%s", sendbuf);
        choice = atoi(sendbuf);
        send(clientSocket, sendbuf, strlen(sendbuf), 0);

        switch(choice) 
        {
            case 1:
            memset(recvbuf, 0, BUFSIZ);
            iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0); 
            printf("recv %s\n", recvbuf);
            if (strcmp(recvbuf, "Nocustomer") == 0) 
            {
                printf("You have no customer. %s\n", recvbuf);
            }
            else
            {
                memset(recvbuf, 0, BUFSIZ);
                iDataNum = recv(clientSocket, recvbuf, BUFSIZ, 0);  
                printf("You have the following customers.\n%s\n", recvbuf);
            }
            break;
            
            case 2:
            printf("Have a nice day! Bye!\n");
            goto INIT;
            break;
        }
    }

    close(clientSocket);  
    return 0;  
}  
