%{
#include "cache.h"
#include "sim.h"

    
extern FILE *yyin;
void yyerror (char *s);
int yylex();
%}

%union {int addr;int num;char * s;}         
%token MEMDUMP REGDUMP
%token <addr> ADDR 
%token <num> NUM
%token er END 
%token STEP QUIT 
%token CONTINUE RUN BREAK DELETE
%start Input
%%

Input: Line  
	| Line Input 
;

Line: END	      {/*printf("Got only end\n");*/}
     | Expression {/*printf("Got only expression\n");*/}
;

Expression:
  REGDUMP END               {print_regdump();}
| MEMDUMP ADDR NUM END      {print_memdump($2,$3);}
| STEP END                  {return 50;}        
| QUIT END                  {return 100;}

| RUN END                   {return 150;}
| CONTINUE END              {return 200;}
| BREAK NUM END             {Add_Break($2);}
| DELETE NUM END            {Delete_Break($2);}

;

| REGDUMP er                {}
| MEMDUMP ADDR NUM er       {}
| MEMDUMP ADDR er           {}
| MEMDUMP er                {}
| STEP er                   {}
| CONTINUE er               {}
| RUN er                    {}
| BREAK NUM er              {}
| DELETE NUM er             {}
| BREAK er                  {}
| DELETE er                 {}
| er {}
;

%%                    

void initialise()
{
    iacc=dacc=numins=numcycles=icache=0;
    

    int i=0;
    for(i=0;i<34;++i)
    {reg[i]=0;}
    reg[34]=0x00400000;//pc=0x00400000

    for(i=0;i<74000005;++i)
    {mem[i]=0;}
    
    inf[0].Ins.type=0;
    inf[1].Ins.type=0;    
    id[0].Ins.type=0;
    id[1].Ins.type=0;
    ex[0].Ins.type=0;
    ex[1].Ins.type=0;    
    ma[0].Ins.type=0;
    ma[1].Ins.type=0;

    isbreak=(int *)calloc((1e7+55),sizeof(int));


    
}

void print_regdump()
{
    int i;
    for(i=0;i<32;++i)
    {printf("$%02d: 0x%08x \n",i,reg[i]);}
    
    printf("hi: 0x%08x \n",reg[32]);
    printf("lo: 0x%08x \n",reg[33]);
    printf("pc: 0x%08x \n",reg[34]);
}

void print_memdump(int addr,int num)
{
    if(addr<0x10010000)
    {fprintf(stderr,"Error - Address given is out of range\n");}
    else if(addr>0x10010000+67108864)
    {fprintf(stderr,"Error - Address given is out of range\n");}
    else 
    {
        int i=0;
        for(i=0;i<num;++i)
        {printf("0x%08x: 0x%02x\n",addr+i,mem[addr-0x10010000+i]);}
    }
}

void llinttobinary(long long int a,int num)
{
    long long int b=a;
    int j;
    for(j=31;j>=0;--j)
    {ins_string[num][j]=b%2+'0';b/=2;}
    //ins_string[num][32]='\0';  
}

void input_hexin()
{
    if (hexin == NULL) {
        fprintf(stderr,"Error - Input hex file not found\n");
        exit(-1);
    }

    int x=1;
    char str[1000];

    ins=(long long int *)calloc((1e7+10),sizeof(long long int));
    ins_string = (char**)calloc((1e7+10), sizeof(char*));
  

    numins=0;
    while(fscanf(hexin,"%s",str)!=EOF)
    {
        if(strlen(str)==8)
        {
            int i=0;
            for(i=0;i<8;++i)
            {
                str[i]=toupper(str[i]);
                if(!( ('0'<=str[i] && str[i]<='9')||('A'<=str[i] && str[i]<='F') ))
                {fprintf(stderr,"Error - Invalid instruction number %d\n",x);
                goto label;}
            }
                   

            ins[numins]=(long long int)strtol(str, NULL, 16);//hex string to long long int
            ins_string[numins] = (char*)calloc(32, sizeof(char));            
            llinttobinary(ins[numins],numins);


    //printf("ins_string[%d]=\"%s\"  len=%lu  ins[%d]=%lld\n",numins,ins_string[numins],strlen(ins_string[numins]),numins,ins[numins]);
            ++numins;
        }
        else
        {fprintf(stderr,"Error - Invalid instruction number %d\n",x);}

    label:;

    ++x;
    }

    fclose(hexin);

}

void test_hexin()
{
    printf("numins=%d\n",numins);
    int i=0;
    for(i=0;i<numins;++i)
    {
        printf("ins[%d]=%lld\n",i,ins[i]);
    }
}


int main(int argc, char* argv[])
{
    initialise();
    int printres=0;

    //test();

    if(argc == 4)
    {
        hexin=fopen(argv[1],"r");
        if (hexin == NULL) {
        fprintf(stderr,"Error - Input hex file \"%s\" not found\n",argv[1]);
        exit(-1);
        }

        // svgout=fopen(argv[2],"w");
        // if (svgout == NULL) {
        // fprintf(stderr,"Error - Output svg file \"%s\" not found\n",argv[2]);
        // exit(-1);
        // }

        svgfile=strdup(argv[2]);

        printres=0;
    }
    else if(argc == 5)
    {
        hexin=fopen(argv[1],"r");
        if (hexin == NULL) {
        fprintf(stderr,"Error - Input hex file \"%s\" not found\n",argv[1]);
        exit(-1);
        }

        // svgout=fopen(argv[2],"w");
        // if (svgout == NULL) {
        // fprintf(stderr,"Error - Output svg file \"%s\" not found\n",argv[2]);
        // exit(-1);
        // }

        svgfile=strdup(argv[2]);

        resout=fopen(argv[3],"w");
        if (resout == NULL) {
        fprintf(stderr,"Error - Output result file \"%s\" not found\n",argv[3]);
        exit(-1);
        }

        printres=1; 
    }
    else
    {
        fprintf(stderr, "Error - 3 or 4 arguments required! Now %d arguments given\n",argc);
        exit(-1);
    }
    
    /*hexin=fopen("in6.hex","r");
    if (hexin == NULL) {
    fprintf(stderr,"Error - Input hex file \"%s\" not found\n","");
    exit(-1);
    }    printres=1; 

    svgout=fopen("test6.svg","w");
    if (svgout == NULL) {
    fprintf(stderr,"Error - Output svg file \"%s\" not found\n","");
    exit(-1);
    }
    resout=fopen("results6.txt","w");
    if (resout == NULL) {
    fprintf(stderr,"Error - Output result file \"%s\" not found\n","");
    exit(-1);
    }
    resout=fopen("results6.txt","w");
    if (resout == NULL) {
    fprintf(stderr,"Error - Output result file \"%s\" not found\n","");
    exit(-1);
    }

    */

    yyin = stdin;
    if (yyin == NULL) {
        fprintf(stderr,"Error - yyin is null\n");
        exit(-2);
    }

    //these have to be called only once
    set_default();
    input_cfg();

    init_vars();//allocate space
    init_cache();
    //don't change the order



    simulate();

    
    //change to keep parsing multiple times because we just want to ignore the wrong line
    do {
        //printf("Shell>>");
        int temp=yyparse();
        if(temp==100)//quit
        {break;}
        else if(temp==50 || temp==150 || temp==200)//step,run,continue
        {printf("Simulation has already finished!\t enter \"quit\" to exit\n");}

    } while (!feof(yyin));
    

    //	test_print();	//debug

    if(printres==1)//output the result file
    {
        print_result();
        fflush(resout);
        fclose(resout);
    }
    //printf("%d\n",printres);

       
    return 0;
}

void print_result()//-------------TODO
{

    fprintf(resout,"Instructions,%d\n",iacc);
    fprintf(resout,"Cycles,%d\n",numcycles);
    double ipc=iacc*1.0/numcycles*1.0;
    fprintf(resout,"IPC,%.04lf\n",ipc);

    double Time=numcycles*0.5;
    int time=Time;


    if(time==Time)
    fprintf(resout,"Time (ns),%d\n",time);
    else
    fprintf(resout,"Time (ns),%.04lf\n",Time);//

    
    double Idle_time=(numcycles-iacc)*0.5;
    int idle_time=Idle_time;
    

    if(idle_time==Idle_time)
    fprintf(resout,"Idle time (ns),%d\n",idle_time);
    else
    fprintf(resout,"Idle time (ns),%.4lf\n",Idle_time);//


    fprintf(resout,"Idle time (%%),%.04lf%%\n",(Idle_time/Time)*100.0);

    fprintf(resout,"Cache Summary\n");
    fprintf(resout,"Cache L1-I\n");
    fprintf(resout,"num cache accesses,%d\n",icache-4);
    fprintf(resout,"Cache L1-D\n");
    fprintf(resout,"num cache accesses,%d\n",dacc);
}

//print error but don't exit
void yyerror(char * s) {

    if (strcmp(s, "syntax error") == 0) {
        fprintf(stderr,"Error - Format of input is wrong\n");
    } else {
        fprintf(stderr,"%s\n", s);
    }

}
