/***************************************************************************
                  nsm.h  -  Neighbor State Machine
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

#ifndef __NSM_H
#define __NSM_H

extern int search_if(struct in_addr rip);

int indexnb;

void inactivityhandler(struct in_addr nb_ip, int index)
{
//	int expectedfiringtime = numofticks + ROUTERDEADINTERVAL ;
	struct ospf_nb * neigh;
	
	nb_timer[index].status = FIRED;
	nb_timer[index].cur_interval = ROUTERDEADINTERVAL;
	neigh = nb_search(nb_ip);
	nsm(neigh, InactivityTimer);
//	printf("in inactivityhandler\n");
}

void startinactivitytimer(struct in_addr nb_ip)
{	struct ospf_nb * nb;
//	printf("in startinactivitytimer\n");
	indexnb = add_nb_sstimer(ROUTERDEADINTERVAL, &inactivityhandler,nb_ip);
	nb = nb_search(nb_ip);
	nb->inactiv_timer[1] = indexnb;
//	printf("Index is %d\n",indexnb);
}

void restartinactivitytimer(struct in_addr nb_ip)
{	
	struct ospf_nb * nb;	
//	printf("in restartinactivitytimer\n");
	nb = nb_search(nb_ip);
	indexnb = nb->inactiv_timer[1];
	restart_nb_sstimer(indexnb);
}

void disableinactivitytimer(struct in_addr nb_ip)
{
	struct ospf_nb * nb = nb_search(nb_ip);
	nb_timer[ nb->inactiv_timer[1] ].status = FIRED;
//	printf("disabling inactivity timer %d \n",nb->rxmtindex);
}

/***********restartrxmttimer(struct in_addr nb_ip)
{	
restart_nb_itimer(indexnb);
}**********************/

void negotiate(struct ospf_nb *nb)
{	int index;
//	printf("===============================  %d\n",nb->rxmtindex);
	if(nb->rxmtindex == -1)
	{
		index = add_nb_itimer(RXMTINTERVAL, &senddd, nb->neighip);
//		printf("IN NEGOTIATE >>%s  >>>>>>>>>\n",inet_ntoa(nb->neighip));
		nb->rxmtindex = index;
	}
	if(nb->neighip.s_addr != inet_addr(MIP))
	senddd(nb->neighip,nb->rxmtindex);
}

enum neighbour_state nsm(struct ospf_nb *nb, enum neighbour_event e)
{	int index = -1;
	enum neighbour_state ncur_state = nb->state;
	index = search_if(nb->neighip);
	if(e != TWO_WayReceived && nb->neighip.s_addr != inet_addr(MIP))
	display(0,0,1,"Neighbour %s Current Event %s Current State %s",inet_ntoa(nb->neighip),getNevent(e),getNstate(nb->state));
	switch(ncur_state)	
	{
		case Down:
			if(e == Start)
			{	nb->state= Attempt;//IGNORED-FOR PTP
				//PT-2-PT--Send Hello
				startinactivitytimer(nb->neighip);
				return nb->state;
			}
			if(e == HelloReceived)
			{
//printf("nsm.c: STATE DOWN --event Helloreceived\n");
				nb->state = Init;
				startinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == InactivityTimer)
			{
				nb->state = Down;
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson lis
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case Attempt://USED ONLY IN CASE P-O-P
			if(e == HelloReceived)
			{
				nb->state = Init;
				restartinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == InactivityTimer)
			{
				nb->state = Down;
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case Init:
			if(e == HelloReceived)
			{//NO STATE CHANGE
		//	printf("nsm.c: State INIT --event HelloRECEIVED\n");
				restartinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == ONE_WayReceived)
			{	
				//printf("nsm.c: State INIT --event 1_WAY RECEIVED\n");
				return nb->state;
			}
			if(e == TWO_WayReceived)
			{	
//			printf("nsm.c: State INIT --event 2_WAY RECEIVED\n");
				if((phyif[index].if_dipa.s_addr
						== nb->neighip.s_addr )
				||(phyif[index].if_bipa.s_addr ==
							nb->neighip.s_addr )
				||(phyif[index].if_dipa.s_addr
						== inet_addr(MIP))
				||(phyif[index].if_bipa.s_addr
						== inet_addr(MIP)))
				{
//printf("nsm.c: -Adjencency should be formed-- init-2way ---%d---\n",index);
					nb->state = ExStart;
					negotiate(nb);
				}
				else
				{	
		//			printf("nsm.c: ELSEELSE\n");
					nb->state = TWO_Way;
				}	
				
				return nb->state;
			}
			if(e == InactivityTimer)
			{
				nb->state = Down;
//printf("nsm.c: State Init --event InactivityTimer \n");
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson lis
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case TWO_Way:
			if(e == HelloReceived)
			{//NO STATE CHANGE
				restartinactivitytimer(nb->neighip);
///////			printf("nsm.c: State 2_WAY --event helloRECEIVED\n");
				return nb->state;
			}	
			if(e == AdjOK)
			{
			//WHETHER TO BECOME ADJ
				if((phyif[index].if_dipa.s_addr
						== nb->neighip.s_addr )
				||(phyif[index].if_bipa.s_addr ==
							nb->neighip.s_addr )
				||(phyif[index].if_dipa.s_addr
						== inet_addr(MIP))
				||(phyif[index].if_bipa.s_addr
						== inet_addr(MIP)))
				{
//printf("nsm.c: -Adjencency should be formed--2way-AdjOK--%d---\n",index);
					nb->state = ExStart;
					negotiate(nb);
				}
				else
				{	
//					printf("nsm.c: ELSEELSE\n");
					nb->state = TWO_Way;
				}	
				return nb->state;
			}
			if(e == ONE_WayReceived)
			{
				nb->state = Init;
//			printf("nsm.c: State 2_WAY --event 1wayRECEIVED\n");
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == TWO_WayReceived)
			{	return nb->state;
			}
			if(e == InactivityTimer)
			{
				nb->state = Down;
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case ExStart:
			if(e == HelloReceived)
			{	
//printf("nsm.c: State exstart --event HelloReceived\n");
				restartinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == NegotiationDone)
			{
			struct rla_node * temp;
			struct nla_node * temp1;
			nb->dbsl = init_lss_llist();
//			printf("nsm.c: state exstart --event negodone\n");
				nb->state = Exchange;
			
//	printf("???????????%d???????\n",phyif[index].if_areaid);
print_rla_llist(area[phyif[index].if_areaid].rla_list);
			for(temp =
			area[phyif[index].if_areaid].rla_list->start->next;
				temp != NULL; temp = temp->next)
			{	
//printf("ADDING IN DBLIST	\n");
				lss_insert(nb->dbsl , temp->data.header) ;
			}
print_lss_llist(nb->dbsl);
			if(area[phyif[index].if_areaid].na_list !=NULL)
			for(temp1 =
			area[phyif[index].if_areaid].na_list->start->next;
				temp1 != NULL; temp1 = temp1->next)
			{	
//printf("ADDING IN DBLIST	\n");
				lss_insert(nb->dbsl , temp1->data.header) ;
			}
			return nb->state;
//list area link state in dbsummary list
			}
			if(e == AdjOK)
			{
			//WHETHER TO BECOME ADJ
				if((phyif[index].if_dipa.s_addr
						== nb->neighip.s_addr )
				||(phyif[index].if_bipa.s_addr ==
							nb->neighip.s_addr )
				||(phyif[index].if_dipa.s_addr
						== inet_addr(MIP))
				||(phyif[index].if_bipa.s_addr
						== inet_addr(MIP)))
				{
//printf("nsm.c: -Adjencency should be formed---exstart-AdjOK---%d---\n",index);
				}
				else
				{
//					printf("nsm.c: ELSEELSE\n");
					nb->state = TWO_Way;
					// free memory of list
					if(nb->dbsl != NULL)
					lss_free(nb->dbsl);
					if(nb->lsretrx != NULL)
					lss_free(nb->lsretrx);
				}	
				return nb->state;
			}
			if(e == ONE_WayReceived)
			{
				nb->state = Init;
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == TWO_WayReceived)
			{	return nb->state;
			}
			if(e == InactivityTimer)
			{
				nb->state = Down;
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case Exchange:
			if(e == HelloReceived)
			{//NO STATE CHANGE
//				printf("nsm.c: State exchange --event HelloReceived\n");
				restartinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == ExchangeDone)
			{
//printf("//ifrequestlist is empty stateis full otherwise loading\n");
				stop_nb_itimer(nb->rxmtindex);
				if(nb->lsreq->start == NULL)
					nb->state = Full;
				else
				{	nb->state = Loading;
				send_request(nb);//start sending request packet
				}
				return nb->state;	
			}
			if(e == AdjOK)
			{
			//WHETHER TO BECOME ADJ
				if((phyif[index].if_dipa.s_addr
						== nb->neighip.s_addr )
				||(phyif[index].if_bipa.s_addr ==
							nb->neighip.s_addr )
				||(phyif[index].if_dipa.s_addr
						== inet_addr(MIP))
				||(phyif[index].if_bipa.s_addr
						== inet_addr(MIP)))
				{
//printf("nsm.c: -Adjencency should be formed---exchange-adjok---%d---\n",index);
				}
				else
				{	
//					printf("nsm.c: ELSEELSE\n");
					nb->state = TWO_Way;
					// free memory of list
					if(nb->dbsl != NULL)
					lss_free(nb->dbsl);
					if(nb->lsretrx != NULL)
					lss_free(nb->lsretrx);
				}	
				return nb->state;
			}
			if(e == ONE_WayReceived)
			{
				nb->state = Init;
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == TWO_WayReceived)
			{	return nb->state;
			}
			if(e == SeqNumberMismatch)
			{
				nb->state = ExStart;
				return nb->state;
			}
			if(e == BadLSReq)
			{
				nb->state = ExStart;
				return nb->state;
			}
			if(e == InactivityTimer)
			{
				nb->state = Down;
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case Loading:
			if(e == HelloReceived)
			{  //NO STATE CHANGE
				restartinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == LoadingDone)
			{
				nb->state = Full;
				return nb->state;
			}
			if(e == AdjOK)
			{
			//WHETHER TO BECOME ADJ
				if((phyif[index].if_dipa.s_addr
						== nb->neighip.s_addr )
				||(phyif[index].if_bipa.s_addr ==
							nb->neighip.s_addr )
				||(phyif[index].if_dipa.s_addr
						== inet_addr(MIP))
				||(phyif[index].if_bipa.s_addr
						== inet_addr(MIP)))
				{
//printf("nsm.c: -Adjencency should be formed---Loading-AdjOK---%d---\n",index);
				}
			else
				{	
//					printf("nsm.c: ELSEELSE\n");
					nb->state = TWO_Way;
						// free memory of list
					if(nb->dbsl != NULL)
					lss_free(nb->dbsl);
					if(nb->lsretrx != NULL)
					lss_free(nb->lsretrx);
				}	
			return nb->state;
			}
			if(e == ONE_WayReceived)
			{
				nb->state = Init;
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == TWO_WayReceived)
			{	return nb->state;
			}
			if(e == SeqNumberMismatch)
			{
				nb->state = ExStart;
				return nb->state;
			}
			if(e == BadLSReq)
			{
				nb->state = ExStart;
				return nb->state;
			}
			if(e == InactivityTimer)
			{
				nb->state = Down;
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
		case Full:
			if(e == HelloReceived)
			{  //NO STATE CHANGE
				restartinactivitytimer(nb->neighip);
				return nb->state;
			}	
			if(e == AdjOK)
			{
			//WHETHER TO BECOME ADJ
				if((phyif[index].if_dipa.s_addr
						== nb->neighip.s_addr )
				||(phyif[index].if_bipa.s_addr ==
							nb->neighip.s_addr )
				||(phyif[index].if_dipa.s_addr
						== inet_addr(MIP))
				||(phyif[index].if_bipa.s_addr
						== inet_addr(MIP)))
				{
//printf("nsm.c: -Adjencency should be formed---Full-AdjOK---%d---\n",index);
				}
				else
				{	
//					printf("nsm.c: ELSEELSE\n");
					nb->state = TWO_Way;
						// free memory of list
					if(nb->dbsl != NULL)
					lss_free(nb->dbsl);
					if(nb->lsretrx != NULL)
					lss_free(nb->lsretrx);
				}	
				return nb->state;
			}
			if(e == ONE_WayReceived)
			{
				nb->state = Init;
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == TWO_WayReceived)
			{	return nb->state;
			}
			if(e == BadLSReq)
			{
				nb->state = ExStart;
				return nb->state;
			}
			if(e == SeqNumberMismatch)
			{
				nb->state = ExStart;
				return nb->state;
			}
			if(e == InactivityTimer)
			{
				nb->state = Down;
display(7,1,5,"Neighbour %s is DEAD",inet_ntoa(nb->neighip));
				ifsm(IFE_NCHG, &phyif[index]);
				return nb->state;
			}
			if(e == LLDown)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			if(e == KillNbr)
			{
				nb->state = Down;
				disableinactivitytimer(nb->neighip);
				nb->lsretrx;//lsa retransmisson list
				nb->dbsl;//database summary list
				nb->lsreq;
				return nb->state;
			}
			break;
	};
}


#endif  // __NSM_H

