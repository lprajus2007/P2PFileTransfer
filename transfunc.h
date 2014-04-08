

#define ConMax 10



typedef struct InfoTable{

        int sid;                                //No. in sequence

        char pno[10];

        char hostname[30];

        char ip[20];

        int fd;

} infot;



typedef struct MessageFormat{

        int kind;                               //which kind of method

        infot lcinfo[ConMax];   //local information

        char hostname[100];             //hostname  || filename

        int nlink;                              //number of links in lcinfo table  || chunk size

} mft;



typedef struct LinklistNode{

        int value;

        struct LinklistNode* next;

} node;



infot lc;

infot rec[ConMax];                      //received from server

infot lcred[ConMax];            //local record

infot sereg[ConMax];            //registered to server










