#include "sim.h"
#include "cache.h"
#include <pthread.h>

const int base_pc=0x00400000;

pthread_mutex_t syn;

void IF(int insnum)//curr,numins
{
	pthread_mutex_lock(&syn);
		
	if(inf[1].Ins.stall==0)
	{

	inf[0].Ins=decoded[insnum];

	//pthread_mutex_lock(&syn);
		
		++icache;

	if(curr>=numins)
	{inf[0].Ins.invalid=1;}
	
	//pthread_mutex_unlock(&syn);
	
	//if(ma[1].Ins.invalid==0)//notinvalid
	//{printf("   in type=%d\n",inf[0].Ins.type);}
	}

	pthread_mutex_unlock(&syn);

}

void ID()
{

	pthread_mutex_lock(&syn);	
	

	if(id[1].Ins.stall==0)
	{
	
		if(inf[1].Ins.invalid==0)
	{
	int rs=inf[1].Ins.Rs , rt=inf[1].Ins.Rt , rd=inf[1].Ins.Rd;
	
	int s=inf[1].Ins.s , t=inf[1].Ins.t , d=inf[1].Ins.d;


	if(s==1)
	{
		id[0].vrs=reg[rs];
	}
	if(t==1)
	{
		id[0].vrt=reg[rt];
	}
	if(d==1)
	{
		id[0].vrd=reg[rd];
	}

	}

	id[0].Ins=inf[1].Ins;
	
	//if(id[0].Ins.type==21 && id[0].Ins.invalid==0)
	//{printf("id[0].vrs=%d\n",id[0].vrs);}
	//printf("ID: Rs=%d vrs=%d s=%d\n",id[0].Ins.Rs,id[0].vrs,id[0].Ins.s);
	}

	pthread_mutex_unlock(&syn);

}

void EX()
{
	ex[0].vrs=id[1].vrs;
	ex[0].vrt=id[1].vrt;
	ex[0].vrd=id[1].vrd;
	ex[0].address=-1;
	ex[0].Ins=id[1].Ins;

	int type=id[1].Ins.type;
	//pthread_mutex_lock(&syn);

	//	if(type==2)
	//	{printf("id[1].vrs=%d\n",id[1].vrs);}

	if(id[1].Ins.invalid==0)
	{

	if(type==1)
	{
		long long int address=id[1].vrs+(long long int)id[1].Ins.Offset;
		ex[0].address=address;
	}
	else if(type==2)
	{
		long long int address=id[1].vrs+(long long int)id[1].Ins.Offset;
		ex[0].address=address;
		//{printf("ex[0].address=%lld  \n",ex[0].address);}

	}
	else if(type==3)
	{
		ex[0].vrd=id[1].vrs+id[1].vrt;
		//printf("%d + %d \n",id[1].vrs,id[1].vrt);
	}
	else if(type==4)
	{
		ex[0].vrd=id[1].vrs-id[1].vrt;
	}
	else if(type==5)
	{
		ex[0].vrt=id[1].vrs+id[1].Ins.Offset;
	}
	else if(type==6)
	{
		ex[0].vrd=id[1].vrs & id[1].vrt;	
	}
	else if(type==7)
	{
		ex[0].vrd=id[1].vrs | id[1].vrt;
	}
	else if(type==8)
	{
		ex[0].vrt=id[1].vrs | id[1].Ins.Offset;
		//printf("%d = %d | %d \n",ex[0].vrd,id[1].vrs,id[1].Ins.Offset);

	}
	else if(type==9)
	{
		ex[0].vrt=(id[1].Ins.Offset<<16);
	}
	else if(type==10)//TODO---------CAUTION
	{
		long long int temp=(long long int)id[1].vrs*((long long int)id[1].vrt);	
		//hi is 32 , lo is 33
		reg[33]=temp%(1LL<<32);
		reg[32]=(temp>>32);
	}
	else if(type==11)//TODO carry//CAUTION
	{
		long long int temp=(long long int)id[1].vrs*(long long int)id[1].vrt;	
		//hi is 32 , lo is 33
		reg[33]=temp%(1LL<<32)+reg[33];
		reg[32]=(temp>>32)+reg[32];	 
		
	}
	else if(type==12)
	{
		ex[0].vrd= ~(id[1].vrs | id[1].vrt);	// '~' is bitwise not operator 
	}
	else if(type==13)//TODO----sign extended
	{
		long long int address=id[1].vrs+(long long int)id[1].Ins.Offset;
		char ch1=mem[address+0-0x10010000];

		ex[0].vrt=(int)(ch1);

	}
	else if(type==14)
	{
		int t=ex[0].vrt;
		char ch4=t%256;//t&255

		long long int address=id[1].vrs+(long long int)id[1].Ins.Offset;
		mem[address+0-0x10010000]=ch4;
	
	}
	else if(type==15)
	{
		ex[0].vrd= (id[1].vrt << id[1].vrs);	
	}
	else if(type==16)
	{
		ex[0].vrd= ( (unsigned int)id[1].vrs < (unsigned int)id[1].vrt )?1:0;
	}
	else if(type==17)
	{
		ex[0].vrt= ( (int)id[1].vrs < (int)id[1].Ins.Offset )?1:0;
	}
	else if(type==18)
	{
		ex[0].vrd= (id[1].vrt << id[1].Ins.Offset);	
	}
	else if(type==19)
	{
		if(id[1].vrt == id[1].vrs)
		{
			curr+=id[1].Ins.Offset;
			curr-=2;
			
			//printf("off1=%d\n",id[1].Ins.Offset);
			
			if(curr<0)
			{fprintf(stderr,"Error - Invalid branch instruction\n");}	

			inf[0].Ins.invalid=1;
			inf[1].Ins.invalid=1;
			id[0].Ins.invalid=1;
		}	
	}
	else if(type==20)
	{
		if(id[1].vrs >= 0)
		{
			curr+=id[1].Ins.Offset;
			curr-=2;

			//printf("off2=%d\n",id[1].Ins.Offset);
			
			if(curr<0)
			{fprintf(stderr,"Error - Invalid branch instruction\n");}	

			inf[0].Ins.invalid=1;
			inf[1].Ins.invalid=1;
			id[0].Ins.invalid=1;
		}	
	}
	else if(type==21)
	{
		//printf("id[1].vrs=%d\n",id[1].vrs);
			
		if(id[1].vrs > 0)
		{
			curr+=id[1].Ins.Offset;
			curr-=2;

			//printf("off3=%d\n",id[1].Ins.Offset);
			
			if(curr<0)
			{fprintf(stderr,"Error - Invalid branch instruction\n");}	

			inf[0].Ins.invalid=1;
			inf[1].Ins.invalid=1;
			id[0].Ins.invalid=1;
		}	
	}
	else if(type==22)
	{
		if(id[1].vrs <= 0)
		{
			curr+=id[1].Ins.Offset;
			curr-=2;

			//printf("off4=%d .vrt=%d\n",id[1].Ins.Offset,id[1].vrt);
			
			if(curr<0)
			{fprintf(stderr,"Error - Invalid branch instruction\n");}	
		
			inf[0].Ins.invalid=1;
			inf[1].Ins.invalid=1;
			id[0].Ins.invalid=1;
		}	
	}
	else if(type==23)
	{
		if(id[1].vrs < 0)
		{
			curr+=id[1].Ins.Offset;
			curr-=2;

			//printf("off5=%d\n",id[1].Ins.Offset);
			
			if(curr<0)
			{fprintf(stderr,"Error - Invalid branch instruction\n");}	
		
			inf[0].Ins.invalid=1;
			inf[1].Ins.invalid=1;
			id[0].Ins.invalid=1;
		}	
	}
	else//NO ALU work, just pass forward
	{}
	//pthread_mutex_unlock(&syn);

	}	
}

void MA()
{
	pthread_mutex_lock(&syn);	

	ma[0].vrs=ex[1].vrs;
	ma[0].vrt=ex[1].vrt;
	ma[0].vrd=ex[1].vrd;

	ma[0].Ins=ex[1].Ins;

	int type=ex[1].Ins.type;

	if(ex[1].Ins.invalid==0)//notinvalid
	{

		//printf("id=%d invalid=%d\n",ex[1].Ins.id,ex[1].Ins.invalid);
	
	if(type==1)
	{
		long long int address=ex[1].address;

		char ch1=mem[address+0-0x10010000];
		char ch2=mem[address+1-0x10010000];
		char ch3=mem[address+2-0x10010000];
		char ch4=mem[address+3-0x10010000];

		int e=256;
		ma[0].vrt=(int)(ch1*e*e*e+ch2*e*e+ch3*e+ch4);

		++dacc;
	}
	else if(type==2)
	{
		int t=ex[1].vrt;
		char ch4=t%256;t/=256;
		char ch3=t%256;t/=256;
		char ch2=t%256;t/=256;
		char ch1=t%256;t/=256;

		long long int address=ex[1].address;
		//printf("addr=0x%08llx\n", address);
		mem[address+0-0x10010000]=ch1;
		mem[address+1-0x10010000]=ch2;
		mem[address+2-0x10010000]=ch3;
		mem[address+3-0x10010000]=ch4;			
	
		++dacc;
	}
	else if(type==13)//TODO----sign extended----probably done
	{
		int address=ex[1].address;
		char ch1=mem[address+0-0x10010000];

		ma[0].vrt=(int)(ch1);

		++dacc;
	}
	else if(type==14)
	{
		int t=ex[1].vrt;
		char ch4=t%256;//t&255

		int address=ex[1].address;
		mem[address+0-0x10010000]=ch4;
	
		++dacc;
	}
	else//NO Memory work, just pass forward
	{}
	
	}

	pthread_mutex_unlock(&syn);

}

void WB()
{
	//pthread_mutex_lock(&syn);

	//printf("%d\n",ma[1].vrd);
	if(ma[1].Ins.invalid==0)//notinvalid
	{

	if(ma[1].Ins.s==2)
	{reg[ma[1].Ins.Rs]=ma[1].vrs;}

	if(ma[1].Ins.t==2)
	{reg[ma[1].Ins.Rt]=ma[1].vrt;}

	if(ma[1].Ins.d==2)
	{reg[ma[1].Ins.Rd]=ma[1].vrd;}
	
	//printf("Rd=%d vrd=%d d=%d\n",ma[1].Ins.Rd,ma[1].vrd,ma[1].Ins.d);
	}
	//printf("%d %d %d\n",ma[1].vrs,ma[1].vrt,ma[1].vrd);

	//pthread_mutex_unlock(&syn);

}

void transfer()
{
	if(ma[1].Ins.invalid==0 && ma[1].Ins.type>0)//notinvalid
	{++iacc;}

	inf[1]=inf[0];//TODO-------check......correct
	id[1]=id[0];
	ex[1]=ex[0];
	ma[1]=ma[0];
	
}


int d[6];

void *IF_()
{
	core_id[1]=sched_getcpu();
	IF(curr);	
}

void *ID_()
{
	core_id[2]=sched_getcpu();
	ID();	
}

void *EX_()
{
	core_id[3]=sched_getcpu();
	pthread_mutex_lock(&syn);	
	EX();	
	pthread_mutex_unlock(&syn);
}

void *MA_()
{
	core_id[4]=sched_getcpu();
	MA();	
}

void *WB_()
{
	core_id[5]=sched_getcpu();
	WB();	
}


void *print_svg()
{
	while(print==1)	
	{
		printsvg();
		sleep(1);
	}	
}


void execute2()
{

   printf ("There are %d cores.\n", get_nprocs ());


	int pc,flag=0,cflag=0;
	//st=0;
	curr=0;

	inf[0].Ins.invalid=1;	
	inf[1].Ins.invalid=1;	
	id[0].Ins.invalid=1;
	id[1].Ins.invalid=1;
	ex[0].Ins.invalid=1;	
	ex[1].Ins.invalid=1;
	ma[0].Ins.invalid=1;
	ma[1].Ins.invalid=1;

//	
	print=1;
	pthread_t svg;
	pthread_create(&svg, NULL, print_svg, NULL);
//

	while(curr-4<numins)
	{

	forwarding();
	stalling();
			

	++numcycles;

//start
	pthread_t threads[6];
	pthread_mutex_init(&syn,NULL);//---------check!!
	
		pc=base_pc+4*curr;
		if(curr<=numins)
		{reg[34]=pc;}
	

	pthread_create(&threads[1], NULL, IF_, NULL);

	pthread_create(&threads[5], NULL, WB_, NULL);
	pthread_join(threads[5], NULL);
	
	pthread_create(&threads[2], NULL, ID_, NULL);
	pthread_join(threads[1], NULL);

	pthread_create(&threads[3], NULL, EX_, NULL);
	pthread_create(&threads[4], NULL, MA_, NULL);

	pthread_join(threads[1], NULL);
	pthread_join(threads[2], NULL);
	pthread_join(threads[3], NULL);
	pthread_join(threads[4], NULL);
	
	// int i;
	// for(i=1;i<6;++i)
	// {printf("%d  ",core_id[i]);}
	// printf("\n");
	
	pthread_mutex_destroy(&syn);


//till here

//threading this

	// 	IF(curr);//thread-1

	// 	pc=base_pc+4*curr;
	// 	if(curr<=numins)
	// 	{reg[34]=pc;}

	// //	printf(" %d %d %d \n", inf[0].Ins.type , (curr+1) , inf[0].Ins.invalid );
	
	// 	WB();//thread-2//join

	// 	ID();//thread-3
	// 	//if(id[0].Ins.type==21 && id[0].Ins.invalid==0)
	// 	//printf("***%d %d\n",id[0].Ins.type,id[0].vrs);
		
	// 	EX();//thread-4

	// 	MA();//thread-5

//till here

//		printsvg();

	
		transfer();//join

	//st=0;

	if(find(curr)==1)
	{cflag=0;}
		
	if(flag==0)
	{
  		
  		if(cflag==0)
  		{
		//printf("Shell>>");
		int code=yyparse();
		if(code==100)//quit
		{return;}
		else if(code==50)//step
		{/*printf("Step received\n");*/}
		else if(code==150)//run
		{flag=1;}
		else if(code==200)//continue
		{cflag=1;}

		}	
	}

		++curr;
		
	}

	print=0;
	print=0;
	pthread_join(svg, NULL);

	printsvg();


}

	//forwarding
	void forwarding()
	{

		//rt<-rd
		//rs<-rd

		if ( ex[1].Ins.invalid==0 && (ex[1].Ins.d==2 && id[1].Ins.s==1) && (ex[1].Ins.Rd == id[1].Ins.Rs) ) {id[1].vrs=ex[1].vrd;p11=1;}
		if ( ex[1].Ins.invalid==0 && (ex[1].Ins.d==2 && id[1].Ins.t==1)	&& (ex[1].Ins.Rd == id[1].Ins.Rt) ) {id[1].vrt=ex[1].vrd;p12=2;}


		if ( ma[1].Ins.invalid==0 && (ma[1].Ins.d==2 && id[1].Ins.s==1) && !( ex[1].Ins.d==2 && (ex[1].Ins.Rd == id[1].Ins.Rs)) && !( ex[1].Ins.t==2 && (ex[1].Ins.Rt == id[1].Ins.Rs)) && (ma[1].Ins.Rd==id[1].Ins.Rs)) 
		{id[1].vrs=ma[1].vrd;p21=1;}
		
		if ( ma[1].Ins.invalid==0 && (ma[1].Ins.d==2 && id[1].Ins.t==1) && !( ex[1].Ins.d==2 && (ex[1].Ins.Rd == id[1].Ins.Rt)) && !( ex[1].Ins.t==2 && (ex[1].Ins.Rt == id[1].Ins.Rt)) && (ma[1].Ins.Rd==id[1].Ins.Rt)) 
		{id[1].vrt=ma[1].vrd;p22=1;}


		//rs<-rt
		if ( ma[1].Ins.invalid==0 && (ma[1].Ins.t==2 && id[1].Ins.s==1) && !( ex[1].Ins.t==2 && (ex[1].Ins.Rt == id[1].Ins.Rs)) && !( ex[1].Ins.d==2 && (ex[1].Ins.Rd == id[1].Ins.Rs)) && (ma[1].Ins.Rt==id[1].Ins.Rs)) 
		{id[1].vrs=ma[1].vrt;p21=1;/*printf("****   %x\n",id[1].vrs);*/}
		if ( ex[1].Ins.invalid==0 && (ex[1].Ins.t==2 && id[1].Ins.s==1) && (ex[1].Ins.Rt == id[1].Ins.Rs) ) 
		{id[1].vrs=ex[1].vrt;p11=1;/*printf("****   %d\n",id[1].vrs);*/}


		//rt<-rt
		if ( ma[1].Ins.invalid==0 && (ma[1].Ins.t==2 && id[1].Ins.t==1) && !( ex[1].Ins.t==2 && (ex[1].Ins.Rt == id[1].Ins.Rt)) && !( ex[1].Ins.d==2 && (ex[1].Ins.Rd == id[1].Ins.Rt)) && (ma[1].Ins.Rt==id[1].Ins.Rt)) 
		{id[1].vrt=ma[1].vrt;p22=1;/*printf("****   %d\n",id[1].vrt);*/}
		if ( ex[1].Ins.invalid==0 && (ex[1].Ins.t==2 && id[1].Ins.t==1) && (ex[1].Ins.Rt == id[1].Ins.Rt) ) 
		{id[1].vrt=ex[1].vrt;p12=1;/*printf("****   %d\n",id[1].vrt);*/}

		if ( ma[1].Ins.invalid==0 && (ex[1].Ins.type==2||ex[1].Ins.type==14) && (ma[1].Ins.type==1||ma[1].Ins.type==13) && (ma[1].Ins.t==2 && ex[1].Ins.t==1) && (ma[1].Ins.Rt == ex[1].Ins.Rt) ) 
		{id[1].vrt=ex[1].vrt;p3=1;/*printf("****   %d\n",id[1].vrt);*/}



	}

	//stalling
	void stalling()
	{
		if ( ex[1].Ins.invalid==0 && (ex[1].Ins.type==1 || ex[1].Ins.type==13) && ((ex[1].Ins.Rt==id[1].Ins.Rs) || ex[1].Ins.Rt==id[1].Ins.Rt) )
		{id[1].Ins.stall=1;inf[1].Ins.stall=1;id[1].Ins.invalid=1;ex[0].Ins.invalid=1;--curr;/*printf("stalled\n");*/}

	}
