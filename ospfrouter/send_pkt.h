/***************************************************************************
                  send_pkt.h  -  Sending OSPF packets
                             -------------------
    begin        : Sun Apr 7 2002
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

#ifndef __SEND_PKT_H
#define __SEND_PKT_H

struct ospf_hello* build_hello(struct in_addr netmask, struct in_addr drid,struct in_addr bdrid, int numofneighbour, struct in_addr * add)
{
	int size = 0,i;
	struct ospf_hello *h;
	h = (struct ospf_hello *)malloc(MINHELLOLENGTH + (numofneighbour*4));

	//OSPF --- HEADER------------|
	//                           |
	//                           \/
	h->header.ospf_version = OSPF_VERSION;
	h->header.ospf_type = T_HELLO;
	inet_aton(ROUTERID,&h->header.ospf_rid);

	h->header.ospf_aid  = AREAID;
	h->header.ospf_cksum=  0;
	h->header.ospf_autype = AU_PASSWD;  //PASSWORD PROTECTED
	strcpy(h->header.ospf_auth,PASSWORD);

	//OSPF --- HELLO ------------|
	//                           |
	//                           \/
	h->oh_netmask = netmask ;
	h->oh_helloin = HELLOINTERVAL;
	h->oh_opts = OPTIONS;
	h->oh_rtrprio = ROUTERPRIORITY;
	h->oh_rdin    = ROUTERDEADINTERVAL;
	h->oh_drid    = drid;
	h->oh_bdrid   = bdrid;
	size = 52;
	for(i = 0;i<numofneighbour ;i++)
	{
		size += 4;
		*(h->oh_neighbor+i) = add[i];
	}
	h->header.ospf_len  = size;
	return h;
}

extern unsigned int s;

void send_hello(int inum)	//inum == interface number.
{

	char *pac;
	struct node * temp;
	struct in_addr netmask ;
	int  numofneighbours, i;
	struct in_addr bdrid;
	struct in_addr drid;
	struct sockaddr_in sadd;
	struct in_addr nb[0]; // 0 => fake array. Only pointer is used
	struct llist *neighbours ;
	
	u_char loopback = 0;

	inet_aton(NMASK,&netmask);
	inet_aton(ALLSPFRouter,&sadd.sin_addr);
	drid = phyif[inum].if_dipa;
	bdrid = phyif[inum].if_bipa;
	
	neighbours = &phyif[inum].if_nblist;
	numofneighbours = neighbours->count;
//	printf("NUM OF NB after numof %d\n",numofneighbours);
	
	for(i = 0, temp = neighbours->start->next;
		i< numofneighbours && temp!= 0; i++,temp = temp->next)
	{
		nb[i] = temp->data.neighid;
	}
	
	sadd.sin_family = PF_INET;
	sadd.sin_port = 0;
	memset(&(sadd.sin_zero) , '0',8);// INITIALIZE TO ZERO
	pac = (char*)build_hello(netmask,drid,bdrid,numofneighbours,nb);
	
///////	display(3,0,1,"Sending HELLO packet to %s ----->",inet_ntoa(sadd.sin_addr));
	
	sendto(s,(char *)pac,(MINHELLOLENGTH+numofneighbours*4+4),
		0,(struct sockaddr*)&sadd,sizeof(sadd));
	//perror("sendto");
	free(pac);
}

void senddd(struct in_addr nb_ip, int index)
{	
	struct sockaddr_in cadd ;
	struct timeval now;
	struct ospf_dd * tosend =(struct ospf_dd*)malloc(sizeof(struct ospf_dd));
	struct ospf_nb * nb     = nb_search(nb_ip);
	
	cadd.sin_family 	= AF_INET	;//SEND TO
	cadd.sin_port   	= 0;
	cadd.sin_addr.s_addr    = nb_ip.s_addr;
	memset(&cadd.sin_zero, '\0', 8);
	if(nb->neighip.s_addr == inet_addr(MIP))
	{	free(tosend);	return;}

	tosend->header.ospf_version = OSPF_VERSION;
	tosend->header.ospf_type    = T_DATADESC;
	tosend->header.ospf_rid.s_addr= inet_addr(ROUTERID);
	tosend->header.ospf_aid     = AREAID;
	tosend->header.ospf_cksum   = 0;
	tosend->header.ospf_autype  = AU_PASSWD;
	strcpy(tosend->header.ospf_auth, PASSWORD  );
	
	tosend->dd_mtu 		    = 1500;
	tosend->dd_opts 	    = nb->neigh_opts;
	tosend->flags.mast_slv      = nb->mast_slv ;
//	printf("NB STATEIS %d INDEXIS %d insendadd\n",nb->state,index);	
	if(nb->ddseqno == 0 || nb->state == ExStart )	
	{
		tosend->flags.more   = 1;
		tosend->flags.init   = 1;
		tosend->flags.mast_slv = nb->mast_slv ;
		gettimeofday(&now, NULL);
		if(nb->mast_slv == 0)//U'R SLAVE REPEAT SEQ NUM
		{
			tosend->dd_seq = nb->ddseqno;
		}
		else
		{
			tosend->dd_seq 	     = 1;//now.tv_sec ;
			nb->ddseqno          = 1;//now.tv_sec ;
		}
//printf("INTOSENDDD dd_seq = %ld \n",tosend->dd_seq);
		tosend->header.ospf_len = MINDDLEN;
		display(5,0,1,"--%d--SENDING EMPTY DD to %s ->>>>>>>>>>",
		tosend->flags.mast_slv,inet_ntoa(cadd.sin_addr));
		sendto(s, (char *)tosend, MINDDLEN, 0,(struct sockaddr *)&cadd,
						sizeof(struct sockaddr));
	}
	else if(nb->state == Exchange)
	{
		tosend->header.ospf_len = MINDDLEN;	
		tosend->flags.init = 0;
//		printf("[[[in exchange[[[[[[[ PRINTING LIST ]]]]]]]]]]\n");
//		print_lss_llist(nb->dbsl);
		if(nb->dbsl->start->next != NULL && nb->dbsl->count != 0)
		{	if(nb->dbsl->start->next != NULL)
			{	
//printf("IFSATISFIED\n");
//				printf("IFSATISFIED\n");
//				printf("IFSATISFIED\n");
//				printf("IFSATISFIED\n");
				
	tosend->dd_lsa[0].lsa_age= nb->dbsl->start->next->data.lsa_age;
	tosend->dd_lsa[0].lsa_opts = nb->dbsl->start->next->data.lsa_opts;
	tosend->dd_lsa[0].lsa_type = nb->dbsl->start->next->data.lsa_type;
tosend->dd_lsa[0].lsa_lsid.s_addr = nb->dbsl->start->next->data.lsa_lsid.s_addr;
tosend->dd_lsa[0].lsa_rid.s_addr = nb->dbsl->start->next->data.lsa_rid.s_addr;
	tosend->dd_lsa[0].lsa_seq = nb->dbsl->start->next->data.lsa_seq;
	tosend->dd_lsa[0].lsa_cksum = nb->dbsl->start->next->data.lsa_cksum;
	tosend->dd_lsa[0].lsa_len = nb->dbsl->start->next->data.lsa_len;
				
//	printf("SENDINGTYPE %c\n ",tosend->dd_lsa[0].lsa_type );
//	printf("SENDINGTYPE %d \n",tosend->dd_lsa[0].lsa_type );
				if(nb->dbsl->count == 1)//IS THIS LAST SUMMARY
					tosend->flags.more = 0;
				else
					tosend->flags.more = 1;
			}
			if(nb->mast_slv == 0)//U'R SLAVE REPEAT SEQ NUM
			{
				tosend->dd_seq = nb->ddseqno;
				if(nb->more == 0)
					tosend->flags.more = 0;
			}
			else
			{
			nb->ddseqno = nb->ddseqno + 1;//INCREMENT THE SEQ NUM..
			tosend->dd_seq= nb->ddseqno;//INCREMENT THE SEQ NUM.
//			printf("sending dd with seq num %d \n", tosend->dd_seq);
//			printf("sending dd with seq num %d \n", tosend->dd_seq);
			}
		}
		else
		{
			if(nb->mast_slv == 0)//U'R SLAVE REPEAT SEQ NUM
			{
				tosend->dd_seq = nb->ddseqno;
			}
			else
			{
			nb->ddseqno = nb->ddseqno + 1;//INCREMENT THE SEQ NUM..
			tosend->dd_seq= nb->ddseqno;//INCREMENT THE SEQ NUM.
//			printf("sending dd with seq num %d \n", tosend->dd_seq);
//			printf("sending dd with seq num %d \n", tosend->dd_seq);
			}
			tosend->flags.more = 0;
//			printf("in nsm.c in else condi. not fulfilled\n");
		}
		sendto(s, (char *)tosend,MINDDLEN+LSSHDRLEN,0,(struct sockaddr*)
				&cadd, sizeof(struct sockaddr));
	}
	//tosend->dd_lsa[0]  	 =;

	free(tosend);
	restart_nb_itimer(nb->rxmtindex);
}

void send_request(struct ospf_nb * nb)
{
	struct sockaddr_in cadd ;

struct ospf_lsr* tosend = (struct ospf_lsr *) malloc(sizeof(struct ospf_lsr));
	tosend->header.ospf_version   = OSPF_VERSION;
        tosend->header.ospf_type      = T_LSREQ;
        tosend->header.ospf_rid.s_addr= inet_addr(ROUTERID);
        tosend->header.ospf_aid       = AREAID;
        tosend->header.ospf_cksum     = 0;
        tosend->header.ospf_autype    = AU_PASSWD;
        strcpy(tosend->header.ospf_auth, PASSWORD  );
	

lss_fill_next(nb->lsreq,&(tosend->lsr_type),&(tosend->lsr_lsid),&(tosend->lsr_rid  ));
        cadd.sin_family         = AF_INET       ;//SEND TO
        cadd.sin_port           = 0;
        cadd.sin_addr.s_addr    = nb->neighip.s_addr;
        memset(&cadd.sin_zero, '\0', 8);

	sendto(s, (char *)tosend,sizeof(struct ospf_lsr),0,(struct sockaddr*)
				&cadd, sizeof(struct sockaddr));
	free(tosend);
}

void send_update(char * data, int updatetype,struct in_addr nb_ip)
{
	struct sockaddr_in cadd ;
	
	switch(updatetype)
	{
		case LST_RLINK:
			{
			struct ospf_lsu *tosend = (struct ospf_lsu *)
			malloc(
			sizeof(struct ospf_lsu) -1 + sizeof(struct ospf_ra)
			);	
			/*to = (char*)tosend;
			for(i=0; i<sizeof(struct ospf_ra); i++)	
				to[i+sizeof(struct ospf_lsu)-1]	= data[i];
			*/
			memcpy(tosend->lsu_data, data, sizeof(struct ospf_ra));
       			 tosend->header.ospf_version   = OSPF_VERSION;
			 tosend->header.ospf_type      = T_LSUPDATE;
		         tosend->header.ospf_rid.s_addr= inet_addr(ROUTERID);
		         tosend->header.ospf_aid       = AREAID;
		         tosend->header.ospf_cksum     = 0;
		         tosend->header.ospf_autype    = AU_PASSWD;
		         strcpy(tosend->header.ospf_auth, PASSWORD  );
			 tosend->lsu_nads 		=1;	
			 cadd.sin_family         = AF_INET       ;//SEND TO
		         cadd.sin_port           = 0;
		         cadd.sin_addr.s_addr    = nb_ip.s_addr;
		         memset(&cadd.sin_zero, '\0', 8);

			 sendto(s, (char *)tosend,
			 sizeof(struct ospf_lsu) -1 + sizeof(struct ospf_ra),
			 0,(struct sockaddr*)&cadd, sizeof(struct sockaddr));
			
		// 	 printf("sending lsu\n");
		//	 perror("sendto");
			free(tosend);
			}
				break;
		case LST_NLINK:
			{
			int non, interface,size;
			struct ospf_lsu *tosend;
			interface = search_if(nb_ip);
			non = phyif[interface].if_nblist.count;
			size = sizeof(struct ospf_lsu)-1+LSSHDRLEN+4+4*(non-1);

			tosend 	= (struct ospf_lsu *) malloc(size);	

			/*to = (char*)tosend;
			for(i=0; i<sizeof(struct ospf_na); i++)	
				to[i+sizeof(struct ospf_lsu)-1]	= data[i];
       			*/
			
			memcpy(tosend->lsu_data, data, size);
			tosend->header.ospf_version   = OSPF_VERSION;
			 tosend->header.ospf_type      = T_LSUPDATE;
		         tosend->header.ospf_rid.s_addr= inet_addr(ROUTERID);
		         tosend->header.ospf_aid       = AREAID;
		         tosend->header.ospf_cksum     = 0;
		         tosend->header.ospf_autype    = AU_PASSWD;
		         strcpy(tosend->header.ospf_auth, PASSWORD  );
			 tosend->lsu_nads 		=1;
			
			cadd.sin_family         = AF_INET       ;//SEND TO
		        cadd.sin_port           = 0;
		        cadd.sin_addr.s_addr    = nb_ip.s_addr;
		        memset(&cadd.sin_zero, '\0', 8);
	
			sendto(s, (char *)tosend,
			sizeof(struct ospf_lsu) -1 + sizeof(struct ospf_na),
			0,(struct sockaddr*)&cadd, sizeof(struct sockaddr));
			
		//	perror("sendto");
			free(tosend);
			}
				break;
		case LST_SLINK:
				break;
		case LST_BRSLINK:
				break;
		case LST_EXTERN:
				break;
		
	};
}

void send_ack(struct ospf_lsu * d , struct in_addr rip)
{
	struct sockaddr_in cadd ;
	struct ospf_lsack *tosend = (struct ospf_lsack *)
        malloc(
        sizeof(struct ospf_lsack)
        );
        tosend->header.ospf_version   = OSPF_VERSION;
        tosend->header.ospf_type      = T_LSACK;
        tosend->header.ospf_rid.s_addr= inet_addr(ROUTERID);
        tosend->header.ospf_aid       = AREAID;
        tosend->header.ospf_cksum     = 0;
        tosend->header.ospf_autype    = AU_PASSWD;
        strcpy(tosend->header.ospf_auth, PASSWORD  );

	memcpy(&tosend->lsheader,&(d->lsu_data),LSSHDRLEN);
/*	tosend->lsheader.lsa_age = ;
	tosend->lsheader.lsa_opts = ;
	tosend->lsheader.lsa_type = ;
	tosend->lsheader.lsa_lsid.s_addr = ;
	tosend->lsheader.lsa_rid.s_addr = ;
	tosend->lsheader.lsa_seq = ;
	tosend->lsheader.lsa_cksum = ;
	tosend->lsheader.lsa_len = ;
*/
	cadd.sin_family         = AF_INET       ;//SEND TO
        cadd.sin_port           = 0;
        cadd.sin_addr.s_addr    = rip.s_addr;
        memset(&cadd.sin_zero, '\0', 8);
	sendto(s, (char *)tosend,
        sizeof(struct ospf_lsack), 0,(struct sockaddr*)&cadd,
	sizeof(struct sockaddr));

//        printf("sending ack\n");
	free(tosend);
}

#endif // __SEND_PKT_H

