#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <stdio.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <errno.h>

#include "transfunc.h"

#include <sys/ioctl.h>

#include <net/if.h>

#include <arpa/inet.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <sys/time.h>

#include <time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <errno.h>

#include <fcntl.h>

#include <netdb.h>

#include <signal.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <sys/stat.h>

#include <sys/uio.h>

#include <unistd.h>

#include <sys/wait.h>

#include <sys/un.h>

#include <sys/select.h>

#include <time.h>



/*

//This GetLocalIp function is from online blog http://blog.csdn.net/tanghuachun/article/details/8945778

char* GetLocalIp()

{

    int MAXINTERFACES=16;

    char *ip = NULL;

    int fd, intrface, retn = 0;

    struct ifreq buf[MAXINTERFACES];

    struct ifconf ifc;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)

    {   ifc.ifc_len = sizeof(buf);

        ifc.ifc_buf = (caddr_t)buf;

        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {

            intrface = ifc.ifc_len / sizeof(struct ifreq);

            while (intrface-- > 0)

            {

                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))

                {

                    ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));

                    break;

                }

            }

        }

        close (fd);

        return ip;

    }

} */





int lcip(char localip[])

{

        struct sockaddr_in Googleaddr;

        struct sockaddr_in rec;



        int tempsk = socket(AF_INET, SOCK_DGRAM, 0);

        int len = sizeof(rec);

        bzero(&Googleaddr, sizeof(Googleaddr));

        Googleaddr.sin_family = AF_INET;

        Googleaddr.sin_port = htons(4565);

        inet_pton(AF_INET, "8.8.4.4", &Googleaddr.sin_addr);



        bzero(&rec, sizeof(rec));

        rec.sin_family = AF_INET;

        rec.sin_port = htons(4567);

        connect(tempsk, (struct sockaddr*)&Googleaddr, sizeof(Googleaddr));

        if(getsockname(tempsk, (struct sockaddr*)&rec,&len)!=0)

                printf("error sock\n");

        strcpy(localip, inet_ntoa(rec.sin_addr));

}







int Cprocess(char pno[])

{

//      printf("trialaa");

        int nlink = 0;                                                                                                                          //number of connection established

        int alink = 0;                                                                                                                          //number of existing connection

        int fdmax = 0;

        int szfd = 0;

        int i,j,k;

        int mlength = 0;                                                                                                                        //length of massage

        int rlength = 0;

        int fc = 0;                                                                                                                                     //count of times sending when file sharing

        int tfsz = 0;                                                                                                                           //transmit file size

        int rfsz[ConMax];                                                                                                                       //receive file size

//      printf("trial1");

        int fsz = 0;                                                                                                                            //file size

        int lsfd = socket(AF_INET, SOCK_STREAM, 0);                                                                     //file descriptor for local listenning

        int cofd = socket(AF_INET, SOCK_STREAM, 0);                                                                     //file descriptor for connection

        int ncofd;                                                                                                                                      //new file descriptor for connection to be established

        double timel = 0;                                                                                                                       //lasting time

        double rtime[ConMax];

        struct timeval start;

        struct timeval end;

        struct sockaddr_in lcaddr;                                                                                                      //local address

        struct sockaddr_in svaddr;                                                                                                      //server address

        struct sockaddr_in praddr;                                                                                                      //peer client address

        mft smsg;                                                                                                                                       //send message

        mft rmsg;

        fd_set linkfd;                                                                                                                          //the set of file descriptors

        fd_set templk;                                                                                                                          //temp fd set for select

        fd_set templkd;

        fd_set downlk;                                                                                                                          //the set for download fd

        char stdcom[100];                                                                                                                       //input command

        char localip[20];

        char lchon[50];                                                                                                                         //local hostname

        char tempip[20];

        char temppt[20];

        char tempid[20];                                                                                                                        //temp id for termination

        char tempfn[100];                                                                                                                       //temp filename for download and upload

        char tempfs[15];                                                                                                                        //temp filesize

        char recbuf[1000];                                                                                                                      //buffering message received

        char fbuf[80];                                                                                                                          //buffer for file exchange, with unit in byte

        char* downbuf;                                                                                                                          //buffer for download

        node* downtr[ConMax];                                                                                                           //download track

        node* tempdt[ConMax];

        int downct[ConMax];

        int slen = 0;                                                                                                                           //download count for each host



        FILE *fp = NULL;                                                                                                                        //for time statistics

//    struct addrinfo    hints;

//    struct addrinfo    *res;

//    struct sockaddr_in *sa;

        bzero(&lcaddr, sizeof(lcaddr));

        lcaddr.sin_family = AF_INET;

        lcaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        lcaddr.sin_port = htons(atoi(pno));

//      printf("trial3");

        bzero(&svaddr, sizeof(svaddr));

        svaddr.sin_family = AF_INET;

//      svaddr.sin_port = htons();





        lcip(localip);

        gethostname(lchon, sizeof(lchon));



        if( bind(lsfd, (struct sockaddr *)&lcaddr, sizeof(lcaddr)) == -1 ){

                printf("clinet bind error\n");

                return 1;

        }

        if ( listen(lsfd, ConMax) == -1 )

        {

                return 1;

        }



        FD_ZERO(&linkfd);

        FD_SET(0,&linkfd);                                                                                                                      //stdin

        FD_SET(lsfd, &linkfd);                                                                                                          //put listen file descriptor in set

        szfd = lsfd + 1;                                                                                                                        //the number of fds possibly have



        while(1){

                templk = linkfd;

                select(szfd, &templk, NULL, NULL, NULL);



                if ( FD_ISSET(lsfd, &templk) )

                {

                        bzero(&praddr, sizeof(praddr));

                        slen = sizeof(praddr);

                        ncofd = accept(lsfd, (struct sockaddr*)&praddr, &slen);

                        FD_SET(ncofd, &linkfd);

                        if(ncofd>szfd-1)

                                szfd = ncofd+1;

                        bzero( recbuf, sizeof(recbuf) );

                        bzero( &smsg, sizeof(smsg) );

                        strcpy( smsg.lcinfo[0].hostname, lchon );

                        recv( ncofd, recbuf, sizeof(recbuf), 0 );

                        send( ncofd, &smsg, sizeof(smsg), 0 );



                        nlink++;

                        alink++;

                        lcred[nlink-1].fd = ncofd;

                        lcred[nlink-1].sid = nlink-1;

                        bzero(&rmsg, sizeof(rmsg));

                        memcpy(&rmsg, recbuf, sizeof(rmsg));

                        strcpy(lcred[nlink-1].hostname, rmsg.lcinfo[0].hostname);

                        strcpy(lcred[nlink-1].ip, rmsg.lcinfo[0].ip);

                        strcpy(lcred[nlink-1].pno, rmsg.lcinfo[0].pno);



                        for (i = 0; i < nlink; i++) {

                                        if( strcmp(lcred[i].hostname,"") )

                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                }

                }

                else if( FD_ISSET(0, &templk) )

                {

                        gets(stdcom);

//                      fgets(stdcom,100,stdin);

                        //----------------------------------------------------------------------  case command line

                        if ( !strncasecmp(stdcom, "myip", 4) )

                        {

        //                      lcip(localip);

                                printf("local ip is %s\n", localip);

//                              sleep(10);

                        }

                        else if ( !strncasecmp(stdcom, "help", 4) )

                        {

                                printf("Commands can be used as follows:\n\

1. HELP displays information\n\

2. MYIP displays local ip\n\

3. MYPORT displays port no.of listening\n\

4. REGISTER<SERVER IP><PORT NO.> to take registration to server\n\

5. CONNECT<DESTITATION><PORT NO>to connect to a client\n\

6. LIST to list all connections\n\

7. TERMINATE<connection id> to terminate a certain connection\n\

8. EXIT to exit the current process\n\

9. DOWNLOAD<filename><chunksize> to download parallel from multiple machine\n\

10.CREATOR to display creator information\n");

//                              sleep(10);

                        }



                        else if ( !strncasecmp(stdcom, "mypo", 4) )

                        {

                                printf("port No. is %s\n", pno);

//                              sleep(10);

                        }

                        else if ( !strncasecmp(stdcom, "regi", 4) )                                                                             //register to server

                        {

                                i = 0;

                                j = 0;

                                while( stdcom[i++]>'9' )

                                {

                                }

                                while( stdcom[i++]!=' ' )

                                {

                                        tempip[j++] = stdcom[i-1];

                                }

                                tempip[j] = '\0';

                                j = 0;

                                while( stdcom[i++]!='\0')

                                {

                                        temppt[j++] = stdcom[i-1];

                                }

                                temppt[j] = '\0';

                                svaddr.sin_port = htons(atoi(temppt));

                                inet_pton(AF_INET, tempip, &svaddr.sin_addr);

                                if(connect(cofd, (struct sockaddr*)&svaddr, sizeof(svaddr))<0) printf("error when connection!\n");

                                else{

                                        nlink++;

                                        alink++;

                                        lcred[0].sid = 0;                                                                                                              //0 means server

                                        strcpy(lcred[0].ip, tempip);

                //                      strcpy(lcred[0].hostname, recbuf);

                                        strcpy(lcred[0].pno, temppt);

                                        lcred[0].fd = cofd;



                                        smsg.kind = 1;                                                                                                                 //1 means regiter command

                //                      smsg.lcinfo[0] = rec[0];

                                        strcpy( smsg.lcinfo[0].hostname, lchon );

                                        strcpy( smsg.lcinfo[0].ip, localip );

                                        strcpy( smsg.lcinfo[0].pno, pno );

                                        if( mlength = send(cofd, &smsg, sizeof(smsg), 0)<sizeof(smsg) ){

                                                printf("message sending error when registration");

                                                return 1;

                                        }

                                        if( mlength = recv(cofd, recbuf, sizeof(recbuf), 0)<0 ){

                                                printf("message receiving error");

                                                return 1;

                                        }

                //                      recbuf[mlength] = '\0';

                                        memcpy(&rmsg, recbuf, sizeof(rmsg));

                                        memcpy(rec,rmsg.lcinfo,sizeof(rec));

                                        printf("Received List from Server:\n");

                                        for (i = 0; i < rmsg.nlink; i++) {

                                                printf("%-2d:%-30s%-25s%-2s\n", rec[i].sid+1,rec[i].hostname, rec[i].ip, rec[i].pno);

                                        }

                                        strcpy(lcred[0].hostname, rmsg.hostname);

                                        FD_SET(cofd, &linkfd);

                                        printf("register successfully\n");

                                        if( cofd+1>szfd )

                                                szfd = cofd+1;

                                }

                        }

                        else if ( !strncasecmp(stdcom, "conn", 4) )

                        {

                                i = 0;

                                j = 0;

                                while( stdcom[i++]>'9' )

                                {

                                }

                                while( stdcom[i++]!=' ' )

                                {

                                        tempip[j++] = stdcom[i-1];

                                }

                                tempip[j] = '\0';

                                j = 0;

                                while( stdcom[i++]!='\0')

                                {

                                        temppt[j++] = stdcom[i-1];

                                }

                                temppt[j] = '\0';



                                bzero(&praddr, sizeof(praddr));

                                praddr.sin_family = AF_INET;

                                praddr.sin_port = htons(atoi(temppt));



                                cofd = socket(AF_INET, SOCK_STREAM, 0);

                                inet_pton(AF_INET, tempip, &praddr.sin_addr);

                                connect( cofd, (struct sockaddr*)&praddr, sizeof(praddr) );



                                nlink++;

                                alink++;

                                lcred[nlink-1].sid = nlink-1;                                                                                           //0 means server

                                strcpy(lcred[nlink-1].ip, tempip);

        //                      strcpy(lcred[nlink-1].hostname, recbuf);

                                strcpy(lcred[nlink-1].pno, temppt);

                                lcred[nlink-1].fd = cofd;



                                memset(&smsg, 0, sizeof(smsg));

                                smsg.kind = 2;                                                                                                                         //2 means connect command

        //                      smsg.lcinfo[0] = rec[0];

                                strcpy( smsg.lcinfo[0].hostname, lchon );

                                strcpy( smsg.lcinfo[0].ip, localip );

                                strcpy( smsg.lcinfo[0].pno, pno );

                                if( mlength = send(cofd, &smsg, sizeof(smsg), 0)<sizeof(smsg) )

                                {

                                        printf("message sending error when registration\n");

                                        return 1;

                                }

                                if( mlength = recv(cofd, recbuf, sizeof(recbuf), 0)<0 )

                                {

                                        printf("message receiving error\n");

                                        return 1;

                                }

                                memcpy( &rmsg, recbuf, sizeof(rmsg) );



                                strcpy( lcred[nlink-1].hostname, rmsg.lcinfo[0].hostname );



                                FD_SET(cofd, &linkfd);

                                if( cofd+1>szfd )

                                        szfd = cofd+1;



                                for (i = 0; i < nlink; i++) {

                                        if( strcmp(lcred[i].hostname,"") )

                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                }

                        }

                        else if ( !strncasecmp(stdcom, "list", 4) ){

                                for (i = 0; i < nlink; i++) {

                                        if( strcmp(lcred[i].hostname,"") )

                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                }

                        }

                        else if ( !strncasecmp(stdcom, "term", 4) ){

                                i = 0;

                                j = 0;

                                while( stdcom[i++]>'9' )

                                {

                                }

                                while( stdcom[i++]!='\0' )

                                {

                                        tempid[j++] = stdcom[i-1];

                                }

                                tempid[j] = '\0';



                                for(i=0;i<nlink;i++)

                                {

                                        if(lcred[i].sid == atoi(tempid)-1)

                                                break;

                                }

                                memset(&smsg, 0, sizeof(smsg));

                                smsg.kind = 3;

                                send(lcred[i].fd, &smsg, sizeof(smsg), 0);                                                                              //3for terminate



                                FD_CLR(lcred[atoi(tempid)-1].fd, &linkfd);

                                close(lcred[atoi(tempid-1)].fd);



                                lcred[i].fd = -1;

                                bzero(lcred[i].hostname, sizeof(lcred[i].hostname));

                                bzero(lcred[i].ip, sizeof(lcred[i].ip));

                                bzero(lcred[i].pno,sizeof(lcred[i].pno));

                                for(j=i+1;j<nlink; j++)

                                {

                                        lcred[j].sid--;

                                }

                                alink--;

                        }

                        else if ( !strncasecmp(stdcom, "exit", 4) )                                                                                     //4 for exit

                        {

                                //in this case client should inform all neighbors in localrecord(lcred)

                                memset(&smsg, 0, sizeof(smsg));

                                smsg.kind = 4;

                                for( i=0; i<nlink; i++ )

                                {

                                        if( strcmp(lcred[i].hostname,"") )

                                        {

                                                send( lcred[i].fd, &smsg, sizeof(smsg),0 );

                                                close(lcred[i].fd);

                                        }

                                }

                                return 0;

                        }

                        else if ( !strncasecmp(stdcom, "down", 4) )                                                                                     //6 for parallel download

                        {

                                i = 0;

                                j = 0;

                                while( stdcom[i++]>'9' ){

                                }

                                while( stdcom[i++]!=' ' ){

                                        tempfn[j++] = stdcom[i-1];

                                }

                                tempfn[j] = '\0';

                                j=0;

                                while( stdcom[i++]!='\0'){

                                        tempfs[j++] = stdcom[i-1];

                                }

                                tempfs[j] = '\0';

                                printf("%d byte chunksize\n", atoi(tempfs));

                                downbuf = (char*)malloc(atoi(tempfs)*sizeof(char));

                                bzero(&downlk, sizeof(downlk));

                                i = 0;

                                k = 0;

                                if( !(fp = fopen(tempfn,"w+")) && !(fp=fopen(tempfn,"wb+")) )  printf("file create error\n");

                                for( i=0;i<ConMax;i++ ){

                                        rtime[i] = 0;

                                        rfsz[i] = 0;

                                        downct[i] = 0;

                                        downtr[i] = (node*)malloc(sizeof(node));

                                        tempdt[i] = downtr[i];

                                }

                                for( i=1; i<nlink; i++ ){

                                        if(lcred[i].fd != -1){

                                //              printf("%dof%d\n", i, alink-1);

                                                memset(&smsg, 0, sizeof(smsg));

                                                smsg.kind = 6;

                                                smsg.nlink = atoi(tempfs);

                                                strcpy(smsg.hostname, tempfn);

                                                send(lcred[i].fd, &smsg, sizeof(smsg),0);

                                                FD_SET(lcred[i].fd, &downlk);

                                        }

                                }

                                i = 0;

                                printf("value is %d, %d\n", alink, nlink);

                                while(i<alink-1)                                                //except server

                                {

                                        templkd = downlk;

                                        printf("start selecting\n");

                                        select(szfd, &templkd, NULL, NULL, NULL);

                                        printf("finish selecting\n");

                                        for( j=1;j<nlink;j++ ){

                                                if(FD_ISSET(lcred[j].fd, &templkd)){

                                                        printf("start downloading from id %d\n", j);

                                                        memset(downbuf, 0, atoi(tempfs)*sizeof(char));

                                                        gettimeofday(&start, NULL);

                                                        if((mlength = recv(lcred[j].fd, downbuf, atoi(tempfs)*sizeof(char),0))<0) printf("receive error from id%d\n",j);

                                                        gettimeofday(&end, NULL);

                                                        if(!strcmp(downbuf,"#")){

                                                                i++;

                                                                printf("finish %d of %d\n",j,alink-1);

                                                                FD_CLR(lcred[j].fd,&downlk);

                                                                k--;

                                                        }else{

                                                                rfsz[j] += mlength;

                                                                rtime[j] += end.tv_sec-start.tv_sec+((double)end.tv_usec-start.tv_usec)/1000000;

                                                                fwrite(downbuf, sizeof(char), mlength, fp);

//                                                              printf("%s\n",downbuf);

                                                                tempdt[j]->value = k;

                                                                tempdt[j]->next = (node*)malloc(sizeof(node));

                                                                tempdt[j] = tempdt[j]->next;

                                                                downct[j]++;

                                                                printf("fwriting %d\n", mlength);

                                                        }

                                                        break;

                                                }

                                        }

                                        k++;

                                }//while(i<actuallink)

                                fclose(fp);



                                printf("Host                          Host-id      File Chunks Downloaded\n");

                                for( i=1; i<nlink; i++ ){

                                        if(lcred[i].fd!=-1){

                                                tempdt[i] = downtr[i];

                                                printf("%-30s%-13d",lcred[i].hostname,lcred[i].sid+1);

                                                for(j=0;j<downct[i];j++){

                                                        printf("%d-%d,", tempdt[i]->value*atoi(tempfs), (tempdt[i]->value+1)*atoi(tempfs)-1 );

                                                        tempdt[i] = tempdt[i]->next;

                                                }

                                                printf("\n");

                                        }

                                }

                                for( i=1; i<nlink; i++ ){

                                        if(lcred[i].fd!=-1){

                                                printf("Rx rate from connection %d is %lf Bytes/s, FileSize %d Bytes, time consumed %lf s\n", i, rfsz[i]/rtime[i], rfsz[i], rtime[i]);

                                        }

                                }

                        }

                        else if ( !strncasecmp(stdcom, "crea", 4) )

                        {

                                printf("This program is made by\nSatya Lakshmipathi Raju S , UBIT: satyalak\n Email:satyalak@buffalo.edu\n");

                        }

                        else{

                                printf("Invalid Command\n");

                        }

                        memset(stdcom, 0, 100);



                }//else if

                else{

                        for( i=0; i<nlink; i++ )

                        {

                                if(FD_ISSET(lcred[i].fd, &templk))

                                {

//                                      printf("%d,%d,\n",i,nlink);

                                        bzero(recbuf, sizeof(recbuf));

                                        bzero(&rmsg, sizeof(rmsg));

                                        if(recv(lcred[i].fd, recbuf, sizeof(recbuf), 0 )<0)     printf("error receiving when responding\n");

                                        memcpy(&rmsg, recbuf, sizeof(rmsg));

                                        switch(rmsg.kind)

                                        {

                                        case 3:                                                                                         //3 for termination

                                                FD_CLR(lcred[i].fd, &linkfd);

                                                close(lcred[i].fd);

                                                bzero(lcred[i].hostname, sizeof(lcred[i].hostname));

                                                lcred[i].fd = -1;

                                                alink--;

                                                for(j = i+1;j<nlink; j++)

                                                {

                                                        lcred[j].sid--;

                                                }

                                                for (i = 0; i < nlink; i++) {

                                                        if( strcmp(lcred[i].hostname,"") )

                                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                                }

                                                break;

                                        case 4:                                                                                         //4 for exit

                                                FD_CLR(lcred[i].fd, &linkfd);

                                                close(lcred[i].fd);

                                                bzero(lcred[i].hostname, sizeof(lcred[i].hostname));

                                                lcred[i].fd = -1;

                                                alink--;

                                                for(j = i+1;j<nlink; j++)

                                                {

                                                        lcred[j].sid--;

                                                }

                                                for (i = 0; i < nlink; i++) {

                                                        if( strcmp(lcred[i].hostname,"") )

                                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                                }

                                                break;

                                        case 6:                                                                                         //6 for download

                                                memset(tempfn, 0, sizeof(tempfn));

                                                strcpy(tempfn,rmsg.hostname);

                                                downbuf = (char*)malloc(rmsg.nlink*sizeof(char));

                                                tfsz = 0;

                                                timel = 0;

                                                if( fp = fopen(tempfn,"r+") ){

                                                        while( (rlength = fread(downbuf, sizeof(char), rmsg.nlink, fp)) > 0 ){

                                                                memset(downbuf, 0, rmsg.nlink*sizeof(char));

                                                                printf("sending\n");

                                                                gettimeofday(&start, NULL);

                                                                mlength = send(lcred[i].fd, downbuf, rlength*sizeof(char), 0);

                                                                gettimeofday(&end, NULL);

                                                                timel += end.tv_sec-start.tv_sec+((double)end.tv_usec-start.tv_usec)/1000000;

                                                                tfsz += mlength;

                                                        }

                                                        sleep(2);

                                                        strcpy(downbuf,"#");

                                                        send(lcred[i].fd, downbuf, 2, 0);

                                                        printf("endoffile!\nTx Rate is %lf Bytes/s, FileSize %d Bytes, time consumed %lf s\n", tfsz/timel, tfsz, timel);

                                                }

                                                else if( fp = fopen(tempfn,"rb+") ){

                                                        while( (rlength = fread(downbuf, sizeof(char), rmsg.nlink, fp)) > 0 ){

                                                                printf("sending2\n");

                                                                gettimeofday(&start, NULL);

                                                                mlength = send(lcred[i].fd, downbuf, rlength*sizeof(char), 0);

                                                                gettimeofday(&end, NULL);

                                                                timel += end.tv_sec-start.tv_sec+((double)end.tv_usec-start.tv_usec)/1000000;

                                                                tfsz += mlength;

                                                        }

                                                        sleep(2);

                                                        strcpy(downbuf,"#");

                                                        send(lcred[i].fd, downbuf, 2, 0);

                                                        printf("endoffile!\nTR is %lf Bytes/s, FileSize %d Bytes, time consumed %lf s\n", tfsz/timel, tfsz, timel);

                                                }

                                                else{

                                                        strcpy(downbuf,"#");

                                                        printf("endoffile!!\n");

                                                        send(lcred[i].fd, downbuf, 2, 0);

                                                }

                                                break;

                                        case 9:                                                         //for receiving serverip list

                                                memset(recbuf, 0, sizeof(recbuf));

                //                              printf("trail1\n");

                                                memset(rec, 0, sizeof(rec));

                                                memcpy(rec,rmsg.lcinfo,sizeof(rec));

                                                printf("Received List from Server:\n");

                                                for (i = 0; i < rmsg.nlink; i++) {

                                                        printf("%-2d:%-30s%-25s%-2s\n", rec[i].sid+1,rec[i].hostname, rec[i].ip, rec[i].pno);

                                                }

                                                break;

                                        default:

        //                                      printf("Received Message Error!\n");

                                                break;



                                        }



                                        break;



                                }

                        }



                }

        }//while(1)







        return 0;

}



int Sprocess(char pno[])

{

        struct sockaddr_in lcaddr;

        struct sockaddr_in praddr;

        int lsfd = socket(AF_INET, SOCK_STREAM, 0);

        int szfd = 0;

        int slen = 0;

        fd_set linkfd;

        fd_set templk;

        char localip[20];

        char lchon[50];

        int nlink = 0;

        int alink = 0;

        int i,j,k;

        int cofd;

        int ncofd;

        mft smsg;                                                                                                                                       //send message

        mft rmsg;

        char stdcom[100];                                                                                                                       //input command

        char tempip[20];

        char temppt[20];

        char tempid[20];                                                                                                                        //temp id for termination

        char recbuf[1000];                                                                                                                      //buffering message received



        memset( &lcaddr, 0, sizeof(lcaddr) );

        memset( &lcred, 0, sizeof(lcred) );

        lcaddr.sin_family = AF_INET;

        lcaddr.sin_addr.s_addr = htonl(INADDR_ANY);

        lcaddr.sin_port = htons(atoi(pno));



        if( bind(lsfd, (struct sockaddr*)&lcaddr, sizeof(lcaddr))==-1 )

        {

                printf("sever bind error\n");

                return 1;

        }

        if( listen(lsfd, ConMax)==-1 )

        {

                printf("sever listen error\n");

                return 1;

        }

        FD_ZERO(&linkfd);

        FD_SET(lsfd,&linkfd);

        FD_SET(0,&linkfd);

        szfd = lsfd+1;

        lcip(localip);

        gethostname(lchon, sizeof(lchon));



        while(1){

                templk = linkfd;

                select(szfd, &templk, NULL, NULL, NULL);

                if(FD_ISSET(lsfd, &templk))

                {

                        bzero(&praddr, sizeof(praddr));

                        slen = sizeof(praddr);

                        ncofd = accept(lsfd, (struct sockaddr*)&praddr, &slen);

                        FD_SET(ncofd, &linkfd);

                        if(ncofd>szfd-1)

                                szfd = ncofd+1;

                        bzero( recbuf, sizeof(recbuf) );



                        if(recv( ncofd, recbuf, sizeof(recbuf), 0 )<0)

                        {

                                printf("receive error\n");

                                return 1;

                        }

                        nlink++;

                        alink++;

                        lcred[nlink-1].fd = ncofd;

                        lcred[nlink-1].sid = nlink-1;

                        bzero(&rmsg, sizeof(rmsg));

                        memcpy(&rmsg, recbuf, sizeof(rmsg));

                        strcpy(lcred[nlink-1].hostname, rmsg.lcinfo[0].hostname);

                        strcpy(lcred[nlink-1].ip, rmsg.lcinfo[0].ip);

                        strcpy(lcred[nlink-1].pno, rmsg.lcinfo[0].pno);





                        bzero( &smsg, sizeof(smsg) );

                        strcpy( smsg.hostname, lchon );

                        memcpy(smsg.lcinfo, lcred, sizeof(lcred));

                        smsg.nlink = nlink;

                        smsg.kind = 9;

                        for(i=0; i<nlink; i++){

                                if(lcred[i].fd!=-1){

                                        printf("%d of %d start\n",i,nlink);

                                        if(send( lcred[i].fd, &smsg, sizeof(smsg), 0 )<0)

                                        {

                                                printf("send error\n");

                                                return 1;

                                        }

                                        printf("%d of %d end\n",i,nlink);

                                }



                        }

                        printf("Current List:\n");

                        for (i = 0; i < nlink; i++) {

                                if( strcmp(lcred[i].hostname,"") )

                                        printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                        }

                }else if(FD_ISSET(0,&templk)){

                        gets(stdcom);

                        //----------------------------------------------------------------------  case command line

                        if ( !strncasecmp(stdcom, "myip", 4) ){

                                printf("local ip is %s\n", localip);

                        }

                        else if ( !strncasecmp(stdcom, "help", 4) ){

                                printf("Commands can be used as follows:\n1. HELP displays information\n2. MYIP displays local ip\n3. MYPORT displays port no.of listening\n4. LIST to list all connections\n5. EXIT to exit the current process\n6. CREATOR to display creator information\n");

                        }

                        else if ( !strncasecmp(stdcom, "mypo", 4) ){

                                printf("port No. is %s\n", pno);

                        }

                        else if ( !strncasecmp(stdcom, "list", 4) ){

                                printf("id:Hostname                      IP address               Port No.\n");

                                for (i = 0; i < nlink; i++) {

                                        if( strcmp(lcred[i].hostname,"") )

                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                }

                        }

                        else if ( !strncasecmp(stdcom, "exit", 4) ){

                                for( i=0;i<nlink;i++ ){

                                        if(lcred[i].fd!=-1){

                                                printf("server exit error\n");

                                                return 1;

                                        }

                                }

                                close(lsfd);

                                return 0;

                        }

                        else if ( !strncasecmp(stdcom, "crea", 4) ){

                                printf("This program is made by\nSatya Lakshmipathi Raju S, UBIT: satyalak\nEmail:satyalak@buffalo.edu\n");

                        }else{

                                printf("Invalid Command\n");

                        }

                }else{

                        for( i=0; i<nlink; i++ ){

                                if(FD_ISSET(lcred[i].fd, &templk)){

                                        bzero(recbuf, sizeof(recbuf));

                                        bzero(&rmsg, sizeof(rmsg));

                                        recv(lcred[i].fd, recbuf, sizeof(recbuf),0);

                                        memcpy(&rmsg, recbuf, sizeof(rmsg));

                                        switch(rmsg.kind){

                                        case 3:                                                                                         //3 for termination

                                                FD_CLR(lcred[i].fd, &linkfd);

                                                close(lcred[i].fd);

                                                bzero(lcred[i].hostname, sizeof(lcred[i].hostname));

                                                lcred[i].fd = -1;

                                                alink--;

                                                for(j = i+1;j<nlink; j++){

                                                        lcred[j].sid--;

                                                }

                                                for (i = 0; i < nlink; i++) {

                                                        if( strcmp(lcred[i].hostname,"") )

                                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                                }

                                                break;

                                        case 4:                                                                                         //4 for exit

                                                FD_CLR(lcred[i].fd, &linkfd);

                                                close(lcred[i].fd);

                                                bzero(lcred[i].hostname, sizeof(lcred[i].hostname));

                                                lcred[i].fd = -1;

                                                alink--;

                                                for(j = i+1;j<nlink; j++){

                                                        lcred[j].sid--;

                                                }

                                                for (i = 0; i < nlink; i++) {

                                                        if( strcmp(lcred[i].hostname,"") )

                                                                printf("%-2d:%-30s%-25s%-2s\n", lcred[i].sid+1, lcred[i].hostname,lcred[i].ip, lcred[i].pno);

                                                }

                                                break;

                                        default:

                                                printf("Received Message Error\n");

                                                break;

                                        }

                                }

                        }

                }

        }

        return 0;

}



int main(int argc, char* argv[])

{

        if( argc<3 || argc>3 ){

                fprintf(stderr,"input format error");

                return 1;

        }else{

                if(!strcasecmp(argv[1],"s")){

                        Sprocess(argv[2]);

                }

                else if(!strcasecmp(argv[1],"c")){

//                      printf("trial0");

                        Cprocess(argv[2]);

                }

                else{

                        fprintf(stderr, "input format error, choose either server or client");

                }

        }

        return 0;
}
