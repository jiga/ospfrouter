/***************************************************************************
                  process_pkt.h  -  Recieved packet processing
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

#ifndef __PROCESS_PKT_H
#define __PROCESS_PKT_H

extern int ifsm(int e,struct ospf_if *pif);
extern enum neighbour_state nsm(struct ospf_nb *, enum neighbour_event);

void process_hello(struct ospf_hello * h, struct in_addr rip)
{
	int numofnb;
	int i, ifnum=-1;
	struct ospf_nb nb,*neigh;
	struct llist * neighbours;
	ifnum = search_if(rip);
	if(h->oh_netmask.s_addr == phyif[ifnum].if_inmask.s_addr
	&&h->oh_rdin == phyif[ifnum].if_rdin
	&& h->oh_helloin == phyif[ifnum].if_helloin)
	{
		neighbours = &(phyif[ifnum].if_nblist) ;
		if(search_list(neighbours ,rip))//ALREADY SEEN NEIGHBOUR
		{
		/*	if(h->header.ospf_len > 52)
			{
				numofnb = (h->header.ospf_len - 52) /4;
				for(i = 0 ; i< numofnb ;i++)
				{
			  if(strcmp((char*)inet_ntoa(h->oh_neighbor[i]),MIP)==0)
			   {	
					neigh = nb_search(rip);
			   	nsm(neigh, TWO_WayReceived);
			   }
				}
			}
		*/		
	}
	else
	{	
		nb.state = Down;
		nb.neighid = h->header.ospf_rid;
		
		nb.mast_slv   =  1;
		nb.ddseqno    = 0 ;
	//		nb.lastddrecv = -1;
		nb.rxmtindex  = -1;

		nb.neighip = rip;
		nb.neigh_prio = h->oh_rtrprio;
		nb.neigh_opts = h->oh_opts;
		nb.neighdr = h->oh_drid;
		nb.neighbdr = h->oh_bdrid;

		nb.lsretrx = NULL;
		nb.dbsl = NULL;
		nb.lsreq = NULL;
		insert(&phyif[ifnum].if_nblist , nb);
		neigh = nb_search(rip);
		nsm(neigh, HelloReceived);
		return;
	}
	neigh = nb_search(rip);
	nsm(neigh, HelloReceived);
	if(h->header.ospf_len > 52)
	{
		int whether2way = 0;
		numofnb = (h->header.ospf_len - 52)/4;
		for(i=0;i<numofnb;i++)
		{
			if(h->oh_neighbor[i].s_addr == inet_addr(MIP))
			{
				nsm(neigh, TWO_WayReceived);
				whether2way = 1;
				break;
			}
		}
		if(whether2way == 0)
		{
			nsm(neigh, ONE_WayReceived);
			return;
		}
	}
	if(h->oh_rtrprio != neigh->neigh_prio)
	{
		neigh->neigh_prio = h->oh_rtrprio;
		ifsm(IFE_NCHG, &phyif[ifnum]);
	}
	if(h->oh_drid.s_addr == rip.s_addr
		&& h->oh_bdrid.s_addr == 0
		&& phyif[ifnum].if_state == IFS_WAITING)
	{
		ifsm(IFE_BSEEN, &phyif[ifnum]);
	}
	if((h->oh_drid.s_addr == rip.s_addr &&
		neigh->neighdr.s_addr != rip.s_addr)
	||(h->oh_drid.s_addr != rip.s_addr)&&
		neigh->neighdr.s_addr == rip.s_addr)
	{
		neigh->neighdr.s_addr = h->oh_drid.s_addr;
	//	if(h->oh_drid.s_addr == phyif[ifnum].if_dipa.s_addr)
	//	{}	
	//	else
		ifsm(IFE_NCHG, &phyif[ifnum]);
	}
	
	if(h->oh_bdrid.s_addr == rip.s_addr
		&& phyif[ifnum].if_state == IFS_WAITING)
	{
		ifsm(IFE_BSEEN, &phyif[ifnum]);
	}
	if((h->oh_bdrid.s_addr == rip.s_addr &&
		neigh->neighbdr.s_addr != rip.s_addr)
	||(h->oh_bdrid.s_addr != rip.s_addr)&&
		neigh->neighbdr.s_addr == rip.s_addr)
	{
		neigh->neighbdr.s_addr = h->oh_bdrid.s_addr;
	//	if(h->oh_bdrid.s_addr == phyif[ifnum].if_bipa.s_addr)
	//	{}
	//	else
		ifsm(IFE_NCHG, &phyif[ifnum]);
	}
	}//IF HELLOINTERVAL
	else
	{
	display(7,1,1,"CONFIGURATION MISMATCH: hello packet rejected");
	}
}

void process_dd(struct ospf_dd * d,struct in_addr rip)
{
	struct ospf_nb * nb = nb_search(rip);


/*printf("\nSTATE %d\n",nb->state);
printf("MAST %d\n",d->flags.mast_slv);
printf("MORE %d\n",d->flags.more);
printf("INIT %d\n",d->flags.init);
printf("SEQ %d------sizeof seq %d \n",d->dd_seq,sizeof(d->dd_seq));
printf("NB DD SEQ NO %ld",nb->ddseqno );
printf("FLAGS  %d --sizeof flags %d",d->flags,sizeof(d->flags));
*/
	if(nb->state == ExStart)
	{//THIS IS EMPTY-->WE HAVE TO NEGOCIATE
		if(d->flags.mast_slv == 1 && d->flags.init == 1
			&& nb->neighid.s_addr > inet_addr(ROUTERID))
		{
			nb->mast_slv = 0;//SET YOURSELF SLAVE
printf("\n");
			display(0,2,1,"I AM SLAVE");
			//negosiation done
			nb->ddseqno = d->dd_seq ;
			senddd(nb->neighip, nb->rxmtindex);
			nsm(nb, NegotiationDone);
		//	printf("in if negotiationdone\n");
		}
		else if(d->flags.mast_slv == 1
			&& nb->neighid.s_addr < inet_addr(ROUTERID))
		{//NB IS DEC ITSELF MASTer BUT HE SHOULD NOT
			nb->mast_slv = 1;//SET YOURSELF MASTER
			display(7,2,1,"I am MASTER");
			senddd(nb->neighip, nb->rxmtindex);
			/////////nsm(nb, NegotiationDone);
			//printf("in else negotiationdone\n");
		}
		else if(d->flags.mast_slv == 0 && d->flags.init==1
				&& d->dd_seq == nb->ddseqno)
		{
			nb->mast_slv= 1;//SET U'F MAST. THIS IS ACK
printf("\n");
			display(7,2,1,"I am MASTER");
			//printf("in else if negotiationdone\n");
			nsm(nb, NegotiationDone);
			senddd(nb->neighip, nb->rxmtindex);
		}
	}
	else if(nb->state == Exchange)
	{//PACKET CONTAIN LSA Header(s)
//		printf("INEXCHANGEDDREVEIVED\n");
//printf("INEXinitis=%d &&&&& condition %d\n", d->flags.init,d->flags.init == 1);
		if(d->flags.init == 1)
		{
			nsm(nb,SeqNumberMismatch);
		}
		//u'r master----|
		//		\/
		else if(d->flags.mast_slv == 0 &&
				d->dd_seq == nb->ddseqno)
		{
		printf("\n");
		display(7,2,1,"I'm MASTER");
		//printf("// PACKET IS NEXT IN SEQ//process further//see whether we have copy of this LSA//if not or our copy is older add to//request list\n");
	
	
			delete_lss_node(nb->dbsl)	;
//	printf("NEXT IN SEQ calling sen**********%d\n",nb->dbsl->count);
		//printf("BEFORESWITCH CASE : %d \n",d->dd_lsa[0].lsa_type);	
			switch (d->dd_lsa[0].lsa_type)
			{	
			case 1://ROUTER ADV.....
			//	printf("IN CASE 1:\n");
				if(search_rla_list(area[1].rla_list,
				d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
					d->dd_lsa[0].lsa_type))
				{
	  			 if(new_rla(area[1].rla_list,
				 d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
				 d->dd_lsa[0].lsa_type,	d->dd_seq))/*IF NEW*/
				 {
				 //ADD TO RETX LIST--I AM PRINTING
display(3,0,1,"New Router Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));
				// printf("TYPE %d\n",d->dd_lsa[0].lsa_type);
				// printf("SEQ %ld\n",d->dd_seq);
				if(nb->lsreq == NULL)
					nb->lsreq = init_lss_llist();
				lss_insert(nb->lsreq, d->dd_lsa[0]);
				 }
				}
				else
				{
				//ADD TO RETX LIST--I AM PRINTING
display(3,0,1,"First TIme Router Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));
//	printf("elseLSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//	printf("elseRID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//				printf("elseTYPE %d\n",d->dd_lsa[0].lsa_type);
//				printf("elseSEQ %ld\n",d->dd_seq);
				
				if(nb->lsreq == NULL)
					nb->lsreq = init_lss_llist();
				lss_insert(nb->lsreq, d->dd_lsa[0]);
				}	
				break;

			case 2:
		//		printf("IN CASE 2:\n");
                                if(search_nla_list(area[1].na_list,
                                d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
                                        d->dd_lsa[0].lsa_type))
                                {
                                 if(new_nla(area[1].na_list,
                                 d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
                                 d->dd_lsa[0].lsa_type, d->dd_seq))/*IF NEW*/
                                 {
display(3,0,1,"New Network Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));
                 //ADD TO RETX LIST--I AM PRINTING
//         printf("LSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//         printf("LSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//                                 printf("TYPE %d\n",d->dd_lsa[0].lsa_type);
//                                 printf("SEQ %ld\n",d->dd_seq);
                                if(nb->lsreq == NULL)
                                        nb->lsreq = init_lss_llist();
                                lss_insert(nb->lsreq, d->dd_lsa[0]);
                                 }
                                }
                                else
                                {
                                //ADD TO RETX LIST--I AM PRINTING
display(3,0,1,"First TIme Network Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));
//        printf("elseLSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//       printf("elseRID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//				printf("elseTYPE %d\n",d->dd_lsa[0].lsa_type);
//                                printf("elseSEQ %ld\n",d->dd_seq);

                                if(nb->lsreq == NULL)
                                        nb->lsreq = init_lss_llist();
                                lss_insert(nb->lsreq, d->dd_lsa[0]);
                                }
					break;
			case 3:
					break;
			case 4:
					break;
			case 5:
					break;
			default:
				//	printf("BADTYPE");
			};
			if(nb->dbsl->count ==0 && d->flags.more == 0)
				nsm(nb, ExchangeDone);
			else
				senddd(nb->neighip,nb->rxmtindex);//sendnext LSS
		}
		else if(d->flags.mast_slv == 0 &&
				d->dd_seq -1 == nb->ddseqno)
		{// PACKET IS DUPLICATE --MAST. SHOULD REJECT
		
		}
		else if(d->flags.mast_slv == 1 &&
		      		d->dd_seq == nb->ddseqno + 1 )
		{// NEXT IN SEQ
			display(0,2,1," I'm SLAVE ");
//			printf("process further//see whether we have copy of this LSA//if not or our copy is older add to//request list");
			nb->ddseqno = d->dd_seq;
//		printf("NEXT IN SEQ calling sen**********%d\n",nb->dbsl->count);
			if(d->flags.more == 0)	
			{
			nb->more = 0;
			senddd(nb->neighip, nb->rxmtindex);//send next L-S-S
			}
			else
			{
			senddd(nb->neighip, nb->rxmtindex);//send next L-S-S
			}
			delete_lss_node(nb->dbsl);
//		printf("BEFORESWITCH CASE : %d \n",d->dd_lsa[0].lsa_type);
                        switch (d->dd_lsa[0].lsa_type)
                        {
                           case 1://ROUTER ADV.....
                                if(search_rla_list(area[1].rla_list,
                                d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
                                        d->dd_lsa[0].lsa_type))
                                {
                                 if(new_rla(area[1].rla_list,
                                 d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
                                 d->dd_lsa[0].lsa_type, d->dd_seq))/*IF NEW*/
                                 {
   display(3,0,1,"New Router Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));
           //ADD TO RETX LIST--I AM PRINTING
//                      printf("LSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//                      printf("LSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//                                 printf("TYPE %d\n",d->dd_lsa[0].lsa_type);
//                                 printf("SEQ %ld\n",d->dd_seq);
				if(nb->lsreq == NULL)
					nb->lsreq = init_lss_llist();
				lss_insert(nb->lsreq, d->dd_lsa[0]);
                                 }
                                }
                                else
                                {
   display(3,0,1,"First Time Router Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));                             //ADD TO RETX LIST--I AM PRINTING
//                     printf("elseLSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//                     printf("elseLSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//				printf("elseTYPE %d\n",d->dd_lsa[0].lsa_type);
//                                printf("elseSEQ %ld\n",d->dd_seq);
				if(nb->lsreq == NULL)
					nb->lsreq = init_lss_llist();
				lss_insert(nb->lsreq, d->dd_lsa[0]);
                                }
                                break;
                           case 2:
///////////////////				printf("IN CASE 2:\n");
                                if(search_nla_list(area[1].na_list,
                                d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
                                        d->dd_lsa[0].lsa_type))
                                {
                                 if(new_nla(area[1].na_list,
                                 d->dd_lsa[0].lsa_lsid,d->dd_lsa[0].lsa_rid,
                                 d->dd_lsa[0].lsa_type, d->dd_seq))/*IF NEW*/
                                 {
display(3,0,1,"New Network Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));                         //ADD TO RETX LIST--I AM PRINTING
//         printf("LSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//         printf("LSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//                                 printf("TYPE %d\n",d->dd_lsa[0].lsa_type);
//                                 printf("SEQ %ld\n",d->dd_seq);
                                if(nb->lsreq == NULL)
                                        nb->lsreq = init_lss_llist();
                                lss_insert(nb->lsreq, d->dd_lsa[0]);
                                 }
                                }
                                else
                                {
display(3,0,1,"New Network Advertisement Recieved Adding To Request List:\n Link State ID :  %s\nRouter ID : %s",inet_ntoa(d->dd_lsa[0].lsa_lsid),inet_ntoa(d->dd_lsa[0].lsa_rid));
		//ADD TO RETX LIST--I AM PRINTING
//        printf("elseLSID %s\n",inet_ntoa(d->dd_lsa[0].lsa_lsid));
//        printf("elseRID %s\n",inet_ntoa(d->dd_lsa[0].lsa_rid));
//				printf("elseTYPE %d\n",d->dd_lsa[0].lsa_type);
//                                printf("elseSEQ %ld\n",d->dd_seq);

                                if(nb->lsreq == NULL)
                                        nb->lsreq = init_lss_llist();
                                lss_insert(nb->lsreq, d->dd_lsa[0]);
                                }
                                        break;
                           case 3:
                                        break;
                           case 4:
                                        break;
                           case 5:
                                         break;
                           default:
  //                                      printf("BADTYPE");
                        };
			if(d->flags.more == 0)	
			{
			nb->more = 0;
			nsm(nb, ExchangeDone);
			}
		}
		else if(d->flags.mast_slv == 1 &&
				d->dd_seq == nb->ddseqno)
		{
//printf("//THIS IS DUPLICATE & SLAVE SHOULD RESEND");
			senddd(rip, nb->rxmtindex);//RESEND PREVIOUS PACKET
		}
		////////////else
		///////////	nsm(nb, SeqNumberMismatch);
	}
	else if(nb->state == Loading || nb->state == Full)
	{
		if(d->flags.init == 1)
			nsm(nb, SeqNumberMismatch);
		else if(d->flags.mast_slv == 1 &&
				d->dd_seq == nb->ddseqno)
		{//THIS IS DUPLICATE & SLAVE SHOULD RESEND
		
		}
		else if(d->flags.mast_slv == 0 &&
				d->dd_seq -1 == nb->ddseqno)
		{// PACKET IS DUPLICATE --MAST. SHOULD REJECT
		
		}
	}
//	printf("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\n");
//	printf("        IN  PROCESS DD                DDDDDDD\n");
//	printf("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\n");
}

void flood()
{
/*	printf("INFLOODING\n");
	printf("INFLOODING\n");
	printf("INFLOODING\n");
	printf("INFLOODING\n");
	printf("INFLOODING\n");
	printf("INFLOODING\n");
*/
}

void process_lsu(struct ospf_lsu * d,struct in_addr rip)
{
	/*struct ospf_nb * nb=nb_search(rip);
	struct ospf_lsa_header *h;
        struct ospf_ra ra;
        struct ospf_na * na;
        long areaid;
        int interface,i,non;
	interface = search_if(rip);
	non = phyif[interface].if_nblist.count;
        printf("After non\n");
	if(nb->state >= Exchange)
        {
                memcpy(h,&d->lsu_data,LSSHDRLEN);
    //    	printf("after mem\n");
                areaid = d->header.ospf_aid;
 //       	printf("after areaid     %d\n",h->lsa_type);
                //validate checksum........
                switch(h->lsa_type)
                {
                case 1:

 //       	printf("\n\n\n\n\n\n\nIN CASE 1\n");
		memcpy(&ra,&d->lsu_data,sizeof(struct ospf_ra));
		if( search_rla_list(area[areaid].rla_list,h->lsa_lsid,
                                        h->lsa_rid, h->lsa_type))
                {

  //      printf("After 1 if\n");
                        if(new_rla(area[areaid].rla_list,h->lsa_lsid,h->lsa_rid,                                h->lsa_type, ra.header.lsa_seq))
                        {
                                for(i = 0 ; i<MAXIF; i++)
                                if(phyif[i].if_state == IFS_DR &&
                                !(phyif[i].if_bipa.s_addr==rip.s_addr))
                                        flood();
                                replace_rla_list(area[areaid].rla_list,ra);
                                send_ack(d,rip);
                        }
                }
                else
                {
//printf("INELSE\n");
                                for(i = 0 ; i<MAXIF; i++)
                                if(phyif[i].if_state == IFS_DR &&
                                !(phyif[i].if_bipa.s_addr==rip.s_addr))
                                        flood();
                                rla_insert(area[areaid].rla_list,ra);
                                send_ack(d,rip);
                }
                break;

		case 2:

  //      	printf("\n\n\n\n\n\nIN CASE 2\n");
		memcpy(na,&d->lsu_data,sizeof(struct ospf_na));
		if(search_nla_list(area[areaid].na_list,h->lsa_lsid,
                                        h->lsa_rid, h->lsa_type))
                {
                        if(new_nla(area[areaid].na_list,h->lsa_lsid,h->lsa_rid
				,h->lsa_type, na->header.lsa_seq))
                        {
                                for(i = 0 ; i<MAXIF; i++)
                                if(phyif[i].if_state == IFS_DR &&
                                !(phyif[i].if_bipa.s_addr==rip.s_addr))
                                        flood();
                                replace_nla_list(area[areaid].na_list,na,non);
                                send_ack(d,rip);
                        }
                }
                else
                {
                                for(i = 0 ; i<MAXIF; i++)
                                if(phyif[i].if_state == IFS_DR &&
                                !(phyif[i].if_bipa.s_addr==rip.s_addr))
                                        flood();
                                nla_insert(area[areaid].na_list,*na,non);
                                send_ack(d,rip);
                }
                                break;
                        case 3:
                                break;
                        case 4:
                                break;
                        case 5:
                                break;
                        default:
                };
        }
*/
display(2,0,1,"Update Received Sending Acknowledgement");//exit(0);
send_ack(d,rip);

}

void process_lsack(struct ospf_lsack * d, struct in_addr rip)
{
	struct ospf_nb * nb=nb_search(rip);
	display(2,0,1,"Acknowledgement Received Sending Next Request");
	delete_lss_node(nb->lsreq);
	if(nb->lsreq->count != 0 )
        	send_request(nb);
//	printf("In Process Ack....\n");
}

void process_lsr(struct ospf_lsr * d,struct in_addr rip)
{
	struct ospf_nb * nb=nb_search(rip);
	struct ospf_ra ra;
	struct ospf_na na;

//printf("<- LSR -> TYPE ------------------- %d\n",d->lsr_type);
//printf("<- LSR -> lsid ------------------- %s\n",inet_ntoa(d->lsr_lsid));
//printf("<- LSR -> rid  ------------------- %s\n",inet_ntoa(d->lsr_rid));
	switch(d->lsr_type)
	{
		case 1:
		display(6,0,1,"Link State Request Received For Router");		
//				printf("IN CASE 1 OF LSR\n");
				if(search_rla_list(area[1].rla_list,
					d->lsr_lsid,d->lsr_rid,
					d->lsr_type))	
				{
					ra = get_rla_list(area[1].rla_list,
					d->lsr_lsid,d->lsr_rid,
					d->lsr_type);
				//display();
				//printf("~~~~~~ra Send UPDATE PACKET ~~~~~~~\n");
				send_update((char*)&ra,d->lsr_type,rip);	
				}
				else
					nsm(nb,BadLSReq);
				break;
		case 2:
		display(6,0,1,"Link State Request Received For Router");		

                              //  printf("IN CASE 2 OF LSR\n");
                                if(search_nla_list(area[1].na_list,
                                        d->lsr_lsid,d->lsr_rid,
                                        d->lsr_type))
                                {
                                        na = get_nla_list(area[1].na_list,
                                        d->lsr_lsid,d->lsr_rid,
                                        d->lsr_type);
//                               printf("~~~~~~na Send UPDATE PACKET ~~~~~~~\n");
                                      send_update((char *)&na,d->lsr_type,rip);
                                }
                                else
                                        nsm(nb,BadLSReq);

				break;
		case 3:
				break;
		case 4:
				break;
		case 5:
				break;
		default:	
	};
		
}



void process_packet(char * buf, struct in_addr from)
{
	char d[MINHDRLEN];
	struct ospf_header *header ;
	memcpy(d, buf,MINHDRLEN);
	header	= (struct ospf_header *)d;
	if(strcmp(header->ospf_auth, PASSWORD) != 0 )
	{
display(7,1,1,"Authentication Failed: Packet Rejected From %s",inet_ntoa(from));
		return;
	}
	switch (header->ospf_type)
	{
	case T_HELLO:
	if(from.s_addr != inet_addr(MIP))
		display(3,0,1,"HELLO PACKET received from %s ",inet_ntoa(from));
		process_hello((struct ospf_hello *)buf, from);
		break;
	case T_DATADESC:
display(5,0,1,"DATABASE DESCRIPTION PACKET received from %s ",inet_ntoa(from));
		if(from.s_addr != inet_addr(MIP))
			process_dd((struct ospf_dd *)buf,from);
		break;
	case T_LSREQ :
display(1,0,1,"LINK STATE REQUEST PACKET  received from %s ",inet_ntoa(from));
		process_lsr((struct ospf_lsr *)buf,from);
		break;
	case T_LSUPDATE:
display(6,0,1,"LINK STATE UPDATE PACKET received from %s ",inet_ntoa(from));
		process_lsu((struct ospf_lsu *)buf,from);
//		send_ack((struct ospf_lsu *)buf,from);
//		printf("after processing lsu\n");
		break;
	case T_LSACK :
display(6,0,1,"LINK STATE ACKNOWLEDGEMENT PACKET received from %s ",inet_ntoa(from));
		process_lsack((struct ospf_lsack*)buf,from);
		break;
	default:
		display(7,2,1,"\nInvalid OSPF Type\n");
	};
	
}

#endif // __PROCESS_PKT_H
