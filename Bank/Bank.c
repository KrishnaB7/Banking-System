/*socket tcp server*/  
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
#include <time.h>
#include <stdbool.h>
#include <mysql/mysql.h>

/*
return 0 - account is valid;
return 1 - account is not valid;
return -1 - system error. 
*/
int validateAccount(int sock, char *account, char *password) // no longer in use
{
	printf("open file 1\n");
	FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int balance;

    fp = fopen("Account.txt", "r");
    if (fp == NULL)
    {
		send(sock, "System error", sizeof("System error"), 0);
		return -1; // system error
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        char temp[BUFSIZ] = "placeholder";
		strcpy(temp, line); // strtok may change the string, so make a copy of original string
		char *p = strtok(temp, " ");
	    char *token[3];
	    int i = 0;
	    while (p != NULL)
	    {
	        token[i++] = p;
	        p = strtok(NULL, " ");
	    }
	    printf("readline: %s", line);
	    printf("after split: %s, %s\n", token[0], token[1]);
	    printf("string to compare: %s, %s\n", account, password);
		if (strcmp(token[0], account) == 0 && strcmp(token[1], password) == 0) {
			balance = atoi(token[2]);
            printf("matched\n");
			fclose(fp);
			return balance;
		}
    }

    printf("not matched\n");
    fclose(fp);
	return -1;
}

char* getCurrentTime() {
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    return asctime (timeinfo);
}

char* genAccount() {
    srand(time(NULL));
    int count = 0;
    char* account = malloc(6);
    strcpy(account, "");
    while (count <= 5) {
        int r = rand() % 10;
        char c = r + '0';
        account[count] = c;
        count++;
    }
    return account;
}

int main(int argc, char *argv[])  
{  
    // mysql
    MYSQL mysql;  
    MYSQL_RES *res;  
    MYSQL_ROW row;  
    //char query[BUFSIZ];
    //strcpy(query, "select * from account");
    //char *query;

    char *server = "localhost";
    char *user = "root";
    char *pw = "cmpe207";
    char *database = "cmpe207";

    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql, server, user, pw, database, 0, NULL, 0)) 
    {
        printf( "Error connecting to database: %s\n", mysql_error(&mysql));
    }
    else 
    {
        printf("Connect to database!\n");
    }

   /** if (mysql_query(&mysql, query)) 
    {
      printf("%s\n", mysql_error(&mysql));
      exit(1);
   }
   res = mysql_store_result(&mysql); 

   if (NULL == res)
   {
        printf("mysql_store_result failure!\n");    
   }

   while(row = mysql_fetch_row(res))
   {
      for(int t = 0; t < mysql_num_fields(res); t++) 
      {
        printf("%s ", row[t]);
      }
      printf("\n");
   }**/


 


    int port = atoi("8888");

    int serverSocket;

    //two sockaddr_in for client and server  
    struct sockaddr_in server_addr;  
    struct sockaddr_in client_Addr;  
    int addr_len = sizeof(client_Addr);  
    int client;  
    char recvbuf[BUFSIZ];
    char sendbuf[BUFSIZ];
    char *balance;  
    //socket, if fail, return -1 
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
    {  
        perror("socket");  
        return -1;  
    }  
  
    bzero(&server_addr, sizeof(server_addr));  
      
    server_addr.sin_family = AF_INET;  
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    // bind to endpoint
    if(bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)  
    {  
        perror("connect");  
        return -1;  
    }

    //listen to any client for connecting  
    if(listen(serverSocket, 5) < 0)   
    {  
        perror("listen");  
        return -1;  
    }
    
    INIT:
    while(1)  
    {  
        //printf("Listening on port: %d\n", portNo);  
         
        client = accept(serverSocket, (struct sockaddr*)&client_Addr, (socklen_t*)&addr_len);  
        if(client < 0)  
        {  
            perror("accept");  
            continue;  
        }  
        printf("receive client data...\n");  
  
        printf("IP is %s\n", inet_ntoa(client_Addr.sin_addr));  
        printf("Port is %d\n", htons(client_Addr.sin_port));  
        
        MAIN:
        ;
        char account[BUFSIZ];
        char password[BUFSIZ];
        float balance;
        int logged = 0;
        while(logged == 0) 
        {
            // receive operation from client
            memset (recvbuf, 0, BUFSIZ);
            int iDataNum = recv(client, recvbuf, BUFSIZ, 0);
            if(iDataNum < 0) {
                perror("recv");
            }
            recvbuf[iDataNum] = '\0';
            
            int operation = atoi(recvbuf);

            switch(operation) 
            {
                case 1: // create teller
                memset (recvbuf, 0, BUFSIZ);
                iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                char tellerName[BUFSIZ];
                strcpy(tellerName, recvbuf);
                
                memset (recvbuf, 0, BUFSIZ);
                iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                char tellerPw[BUFSIZ];
                strcpy(tellerPw, recvbuf);
                
                char addTeller[BUFSIZ];
                strcpy(addTeller, "INSERT INTO teller (account, password) VALUES(\"");
                strncat(addTeller, tellerName, strlen(tellerName));
                strcat(addTeller, "\", \"");
                strncat(addTeller, tellerPw, strlen(tellerPw));
                strcat(addTeller, "\")");
                printf("%s\n", addTeller);
                if (mysql_query(&mysql, addTeller)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                } 
                else 
                {
                    printf("Add teller %s\n", tellerName);
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Success");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0); 
                }
                break;
                
                case 2: // create user
                ; //first name
                char fName[20];
                char lName[20];
                char email[20];
                char pwd[20];
                char tellerNo[20];
                char acnt_type[20];
                //get first name
                memset (recvbuf, 0, BUFSIZ);
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(fName, recvbuf);
                printf("%s ", fName);
                
                //get last name
                memset (recvbuf, 0, BUFSIZ);
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(lName, recvbuf);
                printf("%s ", lName);
                
                //get email
                memset (recvbuf, 0, BUFSIZ);
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(email, recvbuf);
                printf("%s ", email);
                
                //get password
                memset (recvbuf, 0, BUFSIZ);
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(pwd, recvbuf);
                //printf("%s ", pwd);
                
                //send teller
                //char tellerList[BUFSIZ];
                memset (sendbuf, 0, BUFSIZ);
                char *teller_query = "select account from teller";
                if (mysql_query(&mysql, teller_query)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                }
                res = mysql_store_result(&mysql);
                
                char rowstr[BUFSIZ];
                strcpy(rowstr, "");
                while ((row = mysql_fetch_row(res)))
                {
                    strcat(rowstr, row[0]);
                    strcat(rowstr, " ");
                }
                memset (sendbuf, 0, BUFSIZ);
                strcpy(sendbuf, rowstr);
                printf("Sendbuf: %s\n", sendbuf);
                send(client, sendbuf, sizeof(sendbuf), 0);
                
                //receive the teller
                memset (recvbuf, 0, BUFSIZ);
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(tellerNo, recvbuf);
                printf("%s ", tellerNo);
                
                //receive the type
                memset (recvbuf, 0, BUFSIZ);
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(acnt_type, recvbuf);
                printf("%s\n", acnt_type);
                
                //insert to table account
                char *randomAccount;
                randomAccount = genAccount();
                char *now_time;
                now_time = getCurrentTime();
                printf("now_time is: %s\n",now_time);
                char addUser[BUFSIZ];
                strcpy(addUser, "insert into account values ('");
                strncat(addUser, randomAccount, strlen(randomAccount));
                strcat(addUser, "', '");
                strncat(addUser, fName, strlen(fName));
                strcat(addUser, "', '");
                strncat(addUser, lName, strlen(lName));
                strcat(addUser, "', '");
                strncat(addUser, email, strlen(email));
                strcat(addUser, "', '");
                strncat(addUser, pwd, strlen(pwd));      
                strcat(addUser, "', '");
                strncat(addUser, tellerNo, strlen(tellerNo));
                strcat(addUser, "', '");
                strncat(addUser, acnt_type, strlen(acnt_type));
                strcat(addUser, "', '");
                strncat(addUser, now_time, strlen(now_time));
                strcat(addUser, "', 0);");
                if (mysql_query(&mysql, addUser)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                }
                memset (sendbuf, 0, BUFSIZ);
                strcpy(sendbuf, randomAccount);
                send(client, sendbuf, sizeof(sendbuf), 0);
                break;
                
                case 3: // login
                memset (recvbuf, 0, BUFSIZ); // clean buffer
                // get account from client
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(account, recvbuf);
                printf("account: %s\n", account);
                
                memset (recvbuf, 0, BUFSIZ);
                // get password from client
                if(recv(client, recvbuf, BUFSIZ, 0) < 0)  
                {  
                    perror("recv");
                }
                strcpy(password, recvbuf);
                printf("password: %s\n", password);
                
                memset (recvbuf, 0, BUFSIZ);
                memset (sendbuf, 0, BUFSIZ);
                char actQuery[BUFSIZ];
                char* firstName;
                strcpy(actQuery, "select password, user_Fname, balance from account where account_no = '");
                strncat(actQuery, account, strlen(account));
                strcat(actQuery, "'");
                printf("%s\n", actQuery);
                if (mysql_query(&mysql, actQuery)) 
                {
                    printf("MYSQL ERROE: %s\n", mysql_error(&mysql));
                }
                res = mysql_store_result(&mysql); 
                if(res == NULL) {
                    balance = -1.0f;
                }
                
                if (mysql_num_rows(res) != 0) 
                {
                    row = mysql_fetch_row(res);
                    //printf("recv %s vs stored %s\n", password, row[0]);
                    if(strcmp(row[0], password) == 0) {
                        printf("Match!\n");
                        balance = atof(row[2]);
                        firstName = row[1];
                        //printf("%.2f\n", balance);
                    }
                    else
                    {
                        printf("Wrong password!\n");
                        balance = -1.0f;
                    }
                }
                else 
                {
                    //printf("Empty set!\n");
                    balance = -1.0f;
                }
                mysql_free_result(res);
                //printf("%.2f\n", balance);
                
                if (balance >= 0.0f) {
                    printf("Account %s logged in!\n", account);
                    strcpy(sendbuf, "1");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, firstName);
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                    logged = 1;
                    //break;
                } else if (balance == -1.0f){
                    char telQuery[BUFSIZ];
                    strcpy(telQuery, "select password from teller where account = '");
                    strncat(telQuery, account, strlen(account));
                    strcat(telQuery, "'");
                    printf("%s\n", telQuery);
                    if (mysql_query(&mysql, telQuery)) 
                    {
                        printf("%s\n", mysql_error(&mysql));
                    }
                    res = mysql_store_result(&mysql); 
                    if (mysql_num_rows(res) != 0) 
                    {
                        row = mysql_fetch_row(res);
                        //printf("recv %s vs stored %s\n", password, row[0]);
                        if(strcmp(row[0], password) == 0) 
                        {
                            printf("Match!\n");
                            logged = 2;
                            strcpy(sendbuf, "2");
                        }
                        else
                        {
                            printf("Wrong password!\n");
                            strcpy(sendbuf, "Error");
                        }
                    }
                    else 
                    {
                        printf("Wrong account!\n");
                        strcpy(sendbuf, "Error");
                    }
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                    mysql_free_result(res);
                } else {
                    printf("Cannot open datebase! Please fix it.\n");
                    return 0;
                }
                break;
                
                case 4:
                printf("Client exit!\n");
                goto INIT;
                break;
            }
        }
        
        while(logged == 1) // customer logged in
        {
            // receive operation from client
            memset (recvbuf, 0, BUFSIZ);
            int iDataNum = recv(client, recvbuf, BUFSIZ, 0);
            if(iDataNum < 0) {
                perror("recv");
            }
            recvbuf[iDataNum] = '\0';
            int operation = atoi(recvbuf);
            char* op = "";
            char* curTime;
            bool success = true;
            float recvAmount;
            switch(operation) {
                case 1:
                printf("The Customer wants to view the account balance.\n");
                memset (sendbuf, 0, BUFSIZ);

                //strcpy(sendbuf, "Account balance is: ");
                sprintf(sendbuf, "%.2f", balance);
                printf("%s\n", sendbuf);
                send(client, sendbuf, sizeof(sendbuf), 0);
                break;
                
                case 2:
                //printf("%d\n", balance);
                printf("The Customer wants to withdraw cash.\n");
                op = "Withdraw";
                curTime = getCurrentTime();
                memset (recvbuf, 0, BUFSIZ);
                iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                
                float withdrawAmount;
                sscanf(recvbuf, "%f", &withdrawAmount);
                printf("Withdraw Amount: %.2f\n", withdrawAmount);
                recvAmount = withdrawAmount;
                memset (sendbuf, 0, BUFSIZ);
                if(withdrawAmount <= balance) {
                    balance -= withdrawAmount;
                    char amount[20];
                    sprintf(amount, "%10.2f", balance);
                    char balanceQuery[BUFSIZ];
                    strcpy(balanceQuery, "update account set balance = ");
                    strncat(balanceQuery, amount, strlen(amount));
                    strcat(balanceQuery, " where account_no = '");
                    strncat(balanceQuery, account, strlen(account));
                    strcat(balanceQuery, "'");
                    if (mysql_query(&mysql, balanceQuery)) 
                    {
                        printf("%s\n", mysql_error(&mysql));
                    }                  
                    //strcpy(sendbuf, "Success! The account balance is ");
                    printf("Success!\n");
                    sprintf(sendbuf, "Success! The account balance is $%.2f", balance);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                }
                else{
                    success = false;
                    printf("Failed!\n");
                    strcpy(sendbuf, "Failed! Not enough balance");
                    send(client, sendbuf, sizeof(sendbuf), 0);
                }
                break;

                case 3:
                printf("The Customer wants to deposit.\n");
                op = "Deposit";
                curTime = getCurrentTime();
                memset (recvbuf, 0, BUFSIZ);
                iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                float depositAmount;
                sscanf(recvbuf, "%f", &depositAmount);
                printf("Deposit Amount: %.2f\n", depositAmount);
                recvAmount = depositAmount;
                balance += depositAmount;
                char amount[20];
                sprintf(amount, "%10.2f", balance);
                char balanceQuery[BUFSIZ];
                strcpy(balanceQuery, "update account set balance = ");
                strncat(balanceQuery, amount, strlen(amount));
                strcat(balanceQuery, " where account_no = '");
                strncat(balanceQuery, account, strlen(account));
                strcat(balanceQuery, "'");
                if (mysql_query(&mysql, balanceQuery)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                }                  
                memset (sendbuf, 0, BUFSIZ);
                sprintf(sendbuf, "Success! The account balance is $%.2f", balance);
                send(client, sendbuf, sizeof(sendbuf), 0);
                break;

                case 4: // transfer
                printf("The Customer wants to transfer cash.\n");
                op = "Transfer";
                curTime = getCurrentTime();
                memset (recvbuf, 0, BUFSIZ);
                iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                char dest[BUFSIZ];
                strcpy(dest, recvbuf);
                printf("Destination account: %s\n", dest);
                memset (recvbuf, 0, BUFSIZ);
                iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                float transAmount;
                sscanf(recvbuf, "%f", &transAmount);
                printf("Transfer amount: %.2f\n", transAmount);
                recvAmount = transAmount;
                if(transAmount > balance) {
                    success = false;
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Failed! Insufficient Balance!");
                    send(client, sendbuf, sizeof(sendbuf), 0);
                }
                else {
                    balance -= transAmount;
                    char transQuery[BUFSIZ];
                    char transBalance[20];
                    sprintf(transBalance, "%10.2f", balance);
                    strcpy(transQuery, "update account set balance = ");
                    strncat(transQuery, transBalance, strlen(transBalance));
                    strcat(transQuery, " where account_no = '");
                    strncat(transQuery, account, strlen(account));
                    strcat(transQuery, "'");
                    if (mysql_query(&mysql, transQuery)) 
                    {
                        printf("%s\n", mysql_error(&mysql));
                    }  
                    //get the destination balance
                    memset(transQuery, 0, BUFSIZ);
                    strcpy(transQuery, "update account set balance = balance + ");
                    char transferMoney[20];
                    sprintf(transferMoney, "%10.2f", transAmount);
                    strncat(transQuery, transferMoney, strlen(transferMoney));
                    strcat(transQuery, " where account_no = '");
                    strncat(transQuery, dest, strlen(dest));
                    strcat(transQuery, "'");
                    if (mysql_query(&mysql, transQuery)) 
                    {
                        printf("%s\n", mysql_error(&mysql));
                    } 
                    //send result to client
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Success!");
                    send(client, sendbuf, sizeof(sendbuf), 0);
                }
                    break;
                
                case 5: // statement
                ; // do not delete it. walk around of error: a label can only be part of a statement and a declaration is not a statement
                char trandQuery[BUFSIZ];
                strcpy(trandQuery, "select * from transaction where account = '");
                strncat(trandQuery, account, strlen(account));
                strcat(trandQuery, "' limit 10");
                //printf("%s\n", trandQuery);
                if (mysql_query(&mysql, trandQuery)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                }
                res = mysql_store_result(&mysql);
                if(mysql_num_rows(res) == 0) {
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Notrans");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                } else {
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Trans");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                    
                    unsigned int num_fields;
                    unsigned int i;
                    num_fields = mysql_num_fields(res);
                    while ((row = mysql_fetch_row(res)))
                    {
                        unsigned long *lengths;
                        lengths = mysql_fetch_lengths(res);
                        char rowstr[BUFSIZ];
                        strcpy(rowstr, "| ");
                        for(i = 0; i < num_fields; i++)
                        {
                            //printf("%.*s  |", (int) lengths[i], row[i] ? row[i] : "NULL");
                            strcat(rowstr, row[i]);
                            strcat(rowstr, "  | ");
                        }
                        memset (sendbuf, 0, BUFSIZ);
                        strcpy(sendbuf, rowstr);
                        printf("Sendbuf: %s\n", sendbuf);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                    }
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Done");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                }
                mysql_free_result(res);
                break;
                    
                
                case 6: // View profile
                    ; // do not delete it. walk around of error: a label can only be part of a statement and a declaration is not a statement
                    char ViewQuery[BUFSIZ];
                    strcpy(ViewQuery, "select account_no,user_Fname,user_Lname,email,teller,type,time,balance from account where account_no = '");
                    strncat(ViewQuery, account, strlen(account));
                    strcat(ViewQuery, "' limit 10");
                    //printf("%s\n", trandQuery);
                    if (mysql_query(&mysql, ViewQuery))
                    {
                        printf("%s\n", mysql_error(&mysql));
                    }
                    res = mysql_store_result(&mysql);
                    if(mysql_num_rows(res) == 0) {
                        memset (sendbuf, 0, BUFSIZ);
                        strcpy(sendbuf, "Notrans");
                        printf("Sendbuf: %s\n", sendbuf);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                    } else {
                        memset (sendbuf, 0, BUFSIZ);
                        strcpy(sendbuf, "Trans");
                        printf("Sendbuf: %s\n", sendbuf);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        
                        unsigned int num_fields;
                        unsigned int i;
                        num_fields = mysql_num_fields(res);
                        while ((row = mysql_fetch_row(res)))
                        {
                            unsigned long *lengths;
                            lengths = mysql_fetch_lengths(res);
                            char rowstr[BUFSIZ];
                            strcpy(rowstr, "| ");
                            for(i = 0; i < num_fields; i++)
                            {
                                //printf("%.*s  |", (int) lengths[i], row[i] ? row[i] : "NULL");
                                strcat(rowstr, row[i]);
                                strcat(rowstr, "     |    ");
                            }
                            memset (sendbuf, 0, BUFSIZ);
                            strcpy(sendbuf, rowstr);
                            printf("Sendbuf: %s\n", sendbuf);
                            send(client, sendbuf, sizeof(sendbuf), 0);
                        }
                        memset (sendbuf, 0, BUFSIZ);
                        strcpy(sendbuf, "Done");
                        printf("Sendbuf: %s\n", sendbuf);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                    }
                    mysql_free_result(res);
                    break;
                    
                case 7: // update
                    printf("The Customer wants to update profile.\n");
                    memset (recvbuf, 0, BUFSIZ);
                    iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                    int opt = atoi(recvbuf);
                    
                    if(opt == 1) {
                    memset (recvbuf, 0, BUFSIZ);
                    iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                    char updateQuery[BUFSIZ];
                    char updatefname[20];
                       
                    sprintf(updatefname, "%s", recvbuf);
                    strcpy(updateQuery, "update account set user_Fname = '");
                    strncat(updateQuery, updatefname, strlen(updatefname));
                    strcat(updateQuery, "'");
                    strcat(updateQuery, " where account_no = '");
                    strncat(updateQuery, account, strlen(account));
                    strcat(updateQuery, "'");
                        if (mysql_query(&mysql, updateQuery))
                        {
                            printf("%s\n", mysql_error(&mysql));
                        }
                        //strcpy(sendbuf, "Success! The account balance is ");
                        printf("Success!\n");
                        sprintf(sendbuf, "Success! Your First name is modified to %s", updatefname);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        break;
                    }
                    if(opt == 2) {
                        memset (recvbuf, 0, BUFSIZ);
                        iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                        char updateQuery[BUFSIZ];
                        char updatelname[20];
                        
                        sprintf(updatelname, "%s", recvbuf);
                        strcpy(updateQuery, "update account set user_Lname = '");
                        strncat(updateQuery, updatelname, strlen(updatelname));
                        strcat(updateQuery, "'");
                        strcat(updateQuery, " where account_no = '");
                        strncat(updateQuery, account, strlen(account));
                        strcat(updateQuery, "'");
                        if (mysql_query(&mysql, updateQuery))
                        {
                            printf("%s\n", mysql_error(&mysql));
                        }
                        //strcpy(sendbuf, "Success! The account balance is ");
                        printf("Success!\n");
                        sprintf(sendbuf, "Success! Your Lirst name is modified to %s", updatelname);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        break;
                    }
                    if(opt == 3) {
                        memset (recvbuf, 0, BUFSIZ);
                        iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                        char updateQuery[BUFSIZ];
                        char email[20];
                        
                        sprintf(email, "%s", recvbuf);
                        strcpy(updateQuery, "update account set email = '");
                        strncat(updateQuery, email, strlen(email));
                        strcat(updateQuery, "'");
                        strcat(updateQuery, " where account_no = '");
                        strncat(updateQuery, account, strlen(account));
                        strcat(updateQuery, "'");
                        if (mysql_query(&mysql, updateQuery))
                        {
                            printf("%s\n", mysql_error(&mysql));
                        }
                        //strcpy(sendbuf, "Success! The account balance is ");
                        printf("Success!\n");
                        sprintf(sendbuf, "Success! Your Email is modified to %s", email);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        break;
                    }
                    if(opt == 4) {
                        memset (recvbuf, 0, BUFSIZ);
                        iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                        char updateQuery[BUFSIZ];
                        char password[20];
                        
                        sprintf(password, "%s", recvbuf);
                        strcpy(updateQuery, "update account set password = '");
                        strncat(updateQuery, password, strlen(password));
                        strcat(updateQuery, "'");
                        strcat(updateQuery, " where account_no = '");
                        strncat(updateQuery, account, strlen(account));
                        strcat(updateQuery, "'");
                        if (mysql_query(&mysql, updateQuery))
                        {
                            printf("%s\n", mysql_error(&mysql));
                        }
                        //strcpy(sendbuf, "Success! The account balance is ");
                        printf("Success!\n");
                        sprintf(sendbuf, "Success! Your password is modified to %s", password);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        break;
                    }
                    if(opt == 5) {
                        memset (recvbuf, 0, BUFSIZ);
                        iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                        char updateQuery[BUFSIZ];
                        char teller[20];
                        
                        sprintf(teller, "%s", recvbuf);
                        strcpy(updateQuery, "update account set teller = '");
                        strncat(updateQuery, teller, strlen(teller));
                        strcat(updateQuery, "'");
                        strcat(updateQuery, " where account_no = '");
                        strncat(updateQuery, account, strlen(account));
                        strcat(updateQuery, "'");
                        if (mysql_query(&mysql, updateQuery))
                        {
                            printf("%s\n", mysql_error(&mysql));
                        }
                        //strcpy(sendbuf, "Success! The account balance is ");
                        printf("Success!\n");
                        sprintf(sendbuf, "Success! Your teller is modified to %s", teller);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        break;
                    }
                    if(opt == 6) {
                        memset (recvbuf, 0, BUFSIZ);
                        iDataNum = recv(client, recvbuf, BUFSIZ, 0);
                        char updateQuery[BUFSIZ];
                        char type[20];
                        
                        sprintf(type, "%s", recvbuf);
                        strcpy(updateQuery, "update account set type = '");
                        strncat(updateQuery, type, strlen(type));
                        strcat(updateQuery, "'");
                        strcat(updateQuery, " where account_no = '");
                        strncat(updateQuery, account, strlen(account));
                        strcat(updateQuery, "'");
                        if (mysql_query(&mysql, updateQuery))
                        {
                            printf("%s\n", mysql_error(&mysql));
                        }
                        //strcpy(sendbuf, "Success! The account balance is ");
                        printf("Success!\n");
                        sprintf(sendbuf, "Success! Your type is modified to %s", type);
                        send(client, sendbuf, sizeof(sendbuf), 0);
                        break;
                    }
                    break;
                    
                    
                case 8:
                    printf("The Customer wants to delete his account.\n");
                    memset (sendbuf, 0, BUFSIZ);
                    sprintf(sendbuf, "Success! Your account has been deleted\nPlease collect your remaining balance is $%.2f", balance);
                    char deleteQuery[BUFSIZ];
                    memset(deleteQuery, 0, BUFSIZ);
                    strcat(deleteQuery, "DELETE FROM account WHERE account_no = '");
                    strncat(deleteQuery, account, strlen(account));
                    strcat(deleteQuery,"'");
                    printf("deletequery is: %s",deleteQuery);
                    if (mysql_query(&mysql, deleteQuery))
                    {
                        printf("%s\n", mysql_error(&mysql));
                    }
                    //strcpy(sendbuf, "Success! The account balance is ");
                    printf("Success!\n");
                    send(client, sendbuf, sizeof(sendbuf), 0);
                    break;
                    
                    
           
                
                case 9:
                printf("Customer logged out.\n");
                goto MAIN;
                break;
            }
            // write each transaction into DB
            if (success && (operation == 2 || operation == 3 || operation == 4)) {
                char amount[20];
                sprintf(amount, "%10.2f", recvAmount);
                char transactionQuery[BUFSIZ];
                strcpy(transactionQuery, "INSERT INTO transaction (account, time, amount, operation) VALUES(\"");
                strncat(transactionQuery, account, strlen(account));
                strcat(transactionQuery, "\", \"");
                strncat(transactionQuery, curTime, strlen(curTime));
                strcat(transactionQuery, "\", \"");
                strncat(transactionQuery, amount, strlen(amount));
                strcat(transactionQuery, "\", \"");
                strncat(transactionQuery, op, strlen(op));
                strcat(transactionQuery, "\")");
                //printf("%s\n", transactionQuery);
                if (mysql_query(&mysql, transactionQuery)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                }
                else {
                    printf("%s, %s, %s, %s\n", account, curTime, amount, op);
                }
            }
        }
        
        while(logged == 2) // teller logged in
        { 
            memset (recvbuf, 0, BUFSIZ);
            int iDataNum = recv(client, recvbuf, BUFSIZ, 0);
            if(iDataNum < 0) {
                perror("recv");
            }
            recvbuf[iDataNum] = '\0';
            int operation = atoi(recvbuf);
            switch(operation) {
                case 1:
                printf("The teller wants to view the account list.\n");
                
                char tellerQuery[BUFSIZ];
                strcpy(tellerQuery, "select account_no from account where teller = '");
                strncat(tellerQuery, account, strlen(account));
                strcat(tellerQuery, "'");
                printf("%s\n", tellerQuery);
                if (mysql_query(&mysql, tellerQuery)) 
                {
                    printf("%s\n", mysql_error(&mysql));
                }
                res = mysql_store_result(&mysql);
                if(mysql_num_rows(res) == 0) {
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "Nocustomer");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                } else {
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, "customer");
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                    
                    char rowstr[BUFSIZ];
                    while ((row = mysql_fetch_row(res)))
                    {
                        strcpy(rowstr, "");
                        strcat(rowstr, row[0]);
                        strcat(rowstr, " ");
                    }
                    memset (sendbuf, 0, BUFSIZ);
                    strcpy(sendbuf, rowstr);
                    printf("Sendbuf: %s\n", sendbuf);
                    send(client, sendbuf, sizeof(sendbuf), 0);
                }
                mysql_free_result(res);
                break;
                
                case 2:
                printf("Teller logged out.\n");
                goto MAIN;
                break;
            }
        }
    }
    mysql_close(&mysql);
}  
