/***************************************************************************
                  main.c  -  OSPF Router daemon
                             -------------------
    begin        : Sun Apr  7 10:47:21 IST 2002
    copyright    : (C) 2002 by Ashish Puri, Anurag Damani, Jignesh Patel
    email        : ospfrouter@how.to
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ospf.h"

void configure_interfaces()
{
	int i;
	struct llist *l;

	for(i=0;i<MAXIF;i++)
	{
		phyif[i].if_type = IFT_MULTICAST ; // Assuming Multicast enable
		phyif[i].if_state = IFS_DOWN ; //default initial state
		//inet_aton(MIP,&phyif[i].if_ipaddr);	// Interface IP Address	
		//inet_aton(NMASK,&phyif[i].if_inmask);//subnet Mask
		phyif[i].if_areaid = AREAID;	// Area ID of attached n/w
		phyif[i].if_helloin = HELLOINTERVAL;// Hello Packet Interval	
		phyif[i].if_rdin = ROUTERDEADINTERVAL;//RouterDeadInterval	
		phyif[i].if_txdelay = 1;// Interface Transmission Delay
		phyif[i].if_rtrprio = ROUTERPRIORITY;// Router Priority
		phyif[i].if_thello[0] = HELLOINTERVAL;// Hello Timer
		phyif[i].if_twait[0] = ROUTERDEADINTERVAL;	// Wait Timer
		l = initllist(); // Neighbor list of n/w
		phyif[i].if_nblist = *l;

		inet_aton("0.0.0.0",&phyif[i].if_dipa);// IP Addr of dr
		inet_aton("0.0.0.0",&phyif[i].if_bipa);// IP Addr of bdr
		phyif[i].if_metric = ETHERNET_COST;// Cost to use this Interface
		phyif[i].if_rxmtin = RXMTINTERVAL;	// Retransmit Interval
		phyif[i].if_aytype = AU_PASSWD;	// Authentication Type
		strcpy(phyif[i].if_auth,PASSWORD); // Authentication Key
		// FIRE interface up event
		if(ifsm(IFE_UP,&phyif[i]) == -1)printf("Error in IFSM\n");
	}
	inet_aton(MIP,&phyif[0].if_ipaddr);	// Interface IP Address	
	inet_aton(NMASK,&phyif[0].if_inmask);//subnet Mask
	//inet_aton(MIP2,&phyif[1].if_ipaddr);	// Interface IP Address	
	//inet_aton(NMASK2,&phyif[1].if_inmask);//subnet Mask
	for( i = 0 ;i<MAXIF;i++)	
	{	
		struct ospf_ra d;
		d.header.lsa_age = 0  ;
		d.header.lsa_opts= 0;
		d.header.lsa_type = 1;//LST_RLINK;
		d.header.lsa_lsid.s_addr = phyif[0].if_ipaddr.s_addr;
		d.header.lsa_rid.s_addr  = phyif[0].if_ipaddr.s_addr;
		d.header.lsa_seq = -1 ;
		d.header.lsa_cksum = 0;
		d.header.lsa_len = LSA_RLEN ;
		
		d.ra_mbz = '\0';
		d.ra_nlinks = MAXIF;
		d.links[i].ra_lid.s_addr  = phyif[0].if_dipa.s_addr;
		d.links[i].ra_data.s_addr = phyif[0].if_ipaddr.s_addr;
		d.links[i].ra_type        = RAT_TRANSIT;
		d.links[i].ra_ntos        = 0;
		d.links[i].ra_metric      = ETHERNET_COST;
		area[1].rla_list          = init_rla_llist();
		area[1].na_list          = NULL;

		rla_insert(area[1].rla_list,d );
	}
	free(l);
	display(5,0,1,"CONFIGURED");
}

void SHUTDOWN(int i)
{
	display(6,4,5,"Closing OSPF ROUTER");
	display(6,4,1,"THANKS FOR USING IT");
	exit(0);
}

void daemonize(const char *pname,int facility)
{
	int i;
	pid_t pid;
	
	if((pid=fork()) != 0)
		exit(0);	//parent terminates...
	//1st child contd..
	setsid();
	signal(SIGHUP,SIG_IGN);
	if((pid=fork()) != 0)
		exit(0);	//1st child terminates...
	//2nd child contd..
	//daemon_proc = 1;
	umask(0);			// clear file mode creation mask
	for(i = 0; i < 64; i++)
		close(i);	//close any open descriptors
	openlog(pname,LOG_PERROR,facility);
}

unsigned int s;

int main(int argc, char *argv[])
{
 	unsigned int len;
	int len2;
	char loopback = 0;
	struct sockaddr_in cadd;
	struct itimerval timeperiod,t;
	char buf[1024];
	struct ip_mreq ma;
	
	inet_aton(ALLSPFRouter,&(ma.imr_multiaddr));
	inet_aton(MIP,&(ma.imr_interface));
	
//	daemonize(argv[0],LOG_DAEMON);
	
	display(7,5,5,"CONFIGURING INTERFACES");
	configure_interfaces(); //INITIALIZE INTERFACE DATA STRUCTURE

	timeperiod.it_interval.tv_sec  = 1;
	timeperiod.it_interval.tv_usec = 1*1000;
	timeperiod.it_value.tv_sec     = 1;
	timeperiod.it_value.tv_usec    = 1*1000;
	signal(SIGINT,&SHUTDOWN);
	signal(SIGALRM,&handler);
	setitimer(ITIMER_REAL,&timeperiod,&t);
	
	s = socket(PF_INET, SOCK_RAW, 89);
	//perror("socket");

#ifdef MCAST
	setsockopt(s,IPPROTO_IP,IP_ADD_MEMBERSHIP,&ma,8);
///	setsockopt(s,IPPROTO_IP,IP_MULTICAST_LOOP, &loopback, sizeof(loopback));
#endif
	
	while(1)
	{
	len=recvfrom(s,(char *)buf,1024,0,(struct sockaddr*)&cadd,&len2);
		//perror("recvfrom");
		//printf("\nLENGTH %d    %s\n\n",len,inet_ntoa(cadd.sin_addr));
		process_packet(buf+20, cadd.sin_addr);
	}
	close(s);
  return EXIT_SUCCESS;
}
