/***************************************************************************
                  ifsm.h  -  Interface State Machine & DR/BDR election
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

#ifndef	__IFSM_H
#define	__IFSM_H


extern void send_hello(int);
extern void a(int);
extern enum neighbour_state nsm(struct ospf_nb *, enum neighbour_event);

/*\ OSPF interface FSM actions.
 * Defined in Section 9.3 of the OSPF specification.
 *
 *  -  Interface Finite State Machine functions
 *     ----------------------------------------
 *	void elect_drbdr();
 *	int ifsm(int e,struct ospf_if *pif)
\*/


//########################################
//##	ELECT DR & BDR OF NETWORK	##
//########################################

void elect_drbdr(struct ospf_if * pif)
{	struct node * temp;
	int flag = 0,flag1 = 0,dflag = 0,dflag1 = 0;
	struct ospf_nb bdr;//,me;	
	struct llist *neighbours;
//	struct llist *tlist;

	struct in_addr old_dr_id;
	struct in_addr old_bdr_id;
	struct in_addr old_dr_ip;
	struct in_addr old_bdr_ip;
	
	struct in_addr cur_dr_ip;
	struct in_addr cur_bdr_ip;
	struct in_addr cur_dr_id;
	struct in_addr cur_bdr_id;
	
	struct in_addr myid;
	struct in_addr myip;

	int cur_bdr_prio;
	int cur_dr_prio;
	int old_dr_prio;
	int old_bdr_prio;

	inet_aton(ROUTERID, &myid);
	inet_aton(MIP, &myip);

	inet_aton(ROUTERID,&bdr.neighid);
	old_bdr_id.s_addr = pif->if_bipa.s_addr;
	old_dr_id.s_addr = pif->if_dipa.s_addr;
	bdr.neigh_prio  = pif->if_rtrprio ;
	neighbours = &(pif->if_nblist);
	
	/***************me.neigh_prio = ROUTERPRIORITY;
	me.neighid.s_addr = inet_addr(ROUTERID);
	me.neighip.s_addr = pif->if_ipaddr.s_addr;
	tlist = initllist();////pif->if_nblist; //?
	insert(tlist, me);
	for(temp = pif->if_nblist.start->next; temp != NULL;
							temp = temp->next)
		insert(tlist, temp->data);
	neighbours = tlist;
	******************/
	
//	printf("In Elect BDR %s\n",inet_ntoa(old_bdr_id));

// ELECTING BDR FIRST..............
	flag = 0;flag1 = 0;dflag = 0;dflag1 = 0;
	for(temp=neighbours->start->next;temp !=NULL;temp=temp->next)
	{
	  if(temp->data.state >= TWO_Way)
	  {
		if((temp->data.neighbdr.s_addr==temp->data.neighid.s_addr)
		      &&!(temp->data.neighdr.s_addr==temp->data.neighid.s_addr)
			&&!(cur_dr_id.s_addr==temp->data.neighid.s_addr))
		{ //HERE NEIGHBOUR IS DECLARING ITSELF BDR AND NOT DR
//			printf("<*> DECLARING itself BDR not DR\n");	
			if(flag == 0)
			{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
			}
			else if(cur_bdr_prio < temp->data.neigh_prio)
			{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
			}
			else if((cur_bdr_prio == temp->data.neigh_prio)
			      &&(cur_bdr_id.s_addr < temp->data.neighid.s_addr))
			{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
			}			
			flag = 1;	
		}
	  }//END IF >= two-way
	}//END FOR temp

  // NO ONE DECLARING ITSELF BDR
	if(flag == 0)
	{
		for(temp=neighbours->start->next;temp !=NULL;temp=temp->next)
		{
	  	if(temp->data.state >= TWO_Way)
	  	  {
			if(!(temp->data.neighdr.s_addr
					==temp->data.neighid.s_addr)
				&&!(cur_dr_id.s_addr
					==temp->data.neighid.s_addr))
			{	
				if(flag1 == 0)
				{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
				}
				else if(cur_bdr_prio < temp->data.neigh_prio)
				{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
				}
				else if((cur_bdr_prio==temp->data.neigh_prio) &&
				(cur_bdr_id.s_addr < temp->data.neighid.s_addr))
				{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
				}
				flag1 = 1;
			}
		}
	  }
	}
   // ELECTING DR ..............
	for(temp=neighbours->start->next;temp !=NULL;temp=temp->next)
	{
	  if(temp->data.state >= TWO_Way)
	  {
		if(temp->data.neighdr.s_addr==temp->data.neighid.s_addr)
		{ //HERE NEIGHBOUR IS DECLARING ITSELF DR
			if(dflag == 0)
			{
				cur_dr_ip.s_addr = temp->data.neighip.s_addr;
				cur_dr_id.s_addr = temp->data.neighid.s_addr;
				cur_dr_prio = temp->data.neigh_prio;
			}
			else if(cur_dr_prio < temp->data.neigh_prio)
			{
				cur_dr_ip.s_addr = temp->data.neighip.s_addr;
				cur_dr_id.s_addr = temp->data.neighid.s_addr;
				cur_dr_prio = temp->data.neigh_prio;
			}
			else if((cur_dr_prio == temp->data.neigh_prio) &&
				(cur_dr_id.s_addr < temp->data.neighid.s_addr))
			{
				cur_dr_ip.s_addr = temp->data.neighip.s_addr;
				cur_dr_id.s_addr = temp->data.neighid.s_addr;
				cur_dr_prio = temp->data.neigh_prio;
			}			
			dflag = 1;	
		}
	  }//end if >= 2 way
	}//end for

  // NO ONE DECLARING ITSELF DR
if(dflag == 0)
{
	cur_dr_ip.s_addr = cur_bdr_ip.s_addr;//MAKE CURRENT BDR AS DR
	cur_dr_id.s_addr = cur_bdr_id.s_addr;
	cur_dr_prio = cur_bdr_prio ;
	if(cur_dr_id.s_addr != old_bdr_id.s_addr)
	{
	cur_bdr_id.s_addr = old_bdr_id.s_addr;
	cur_bdr_ip.s_addr = old_bdr_ip.s_addr;
	cur_bdr_prio = old_bdr_prio;
	}
	else
	{
	cur_bdr_id.s_addr = 0;
	cur_bdr_ip.s_addr = 0;
	cur_bdr_prio = 0;
	old_bdr_id.s_addr = 0;
	old_bdr_ip.s_addr = 0;
	old_bdr_prio = 0;
	}
//printf("UPGRADING BDR TO DR\n");	

}
//@@@@@@@@@@@@@@        REELECT      @@@@@@@@@@@@@@@@@@@@
	

if(((old_bdr_id.s_addr == myid.s_addr) && (cur_bdr_id.s_addr != myid.s_addr))||
((old_dr_id.s_addr == myid.s_addr) && (cur_dr_id.s_addr != myid.s_addr))||
((old_bdr_id.s_addr != myid.s_addr) && (cur_bdr_id.s_addr == myid.s_addr))||
((old_dr_id.s_addr != myid.s_addr) && (cur_dr_id.s_addr == myid.s_addr)))
	{	
		   // ELECTING BDR FIRST..............
		flag = 0;flag1 = 0;dflag = 0;dflag1 = 0;
		for(temp=neighbours->start->next;temp !=NULL;temp=temp->next)
		{
	  	  if(temp->data.state >= TWO_Way)
	  	  {
//			printf("In fOR %d \n",neighbours->count);
			if(((temp->data.neighbdr.s_addr
				==temp->data.neighid.s_addr)
			&& !(temp->data.neighdr.s_addr
				==temp->data.neighid.s_addr))||
			((cur_bdr_id.s_addr == temp->data.neighid.s_addr)
			&& !(cur_dr_id.s_addr == temp->data.neighid.s_addr)))
			{ //NEIGHBOUR DECLARING ITSELF BDR ! DR	
				if(flag == 0)
				{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
				}
				else if(cur_bdr_prio < temp->data.neigh_prio)
				{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
				}
				else if((cur_bdr_prio
					== temp->data.neigh_prio) &&
				(cur_bdr_id.s_addr < temp->data.neighid.s_addr))
				{
				cur_bdr_ip.s_addr = temp->data.neighip.s_addr;
				cur_bdr_id.s_addr = temp->data.neighid.s_addr;
				cur_bdr_prio = temp->data.neigh_prio;
				}			
				flag = 1;	
			}
		  }//end if >=2 way
		}//end for

  	// NO ONE DECLARING ITSELF BDR
		if(flag == 0)
		{
			for(temp=neighbours->start->next;temp !=NULL;
							temp=temp->next)
			{
	  		  if(temp->data.state >= TWO_Way)
	  		  {
			   if(!(temp->data.neighdr.s_addr
					==temp->data.neighid.s_addr)
				&& !(cur_dr_id.s_addr
					== temp->data.neighid.s_addr))
				{
//	printf("2. NOT DECLARING itself BDR\n");
				   if(flag1 == 0)
				   {
				   cur_bdr_ip.s_addr=temp->data.neighip.s_addr;
				   cur_bdr_id.s_addr=temp->data.neighid.s_addr;
			           cur_bdr_prio = temp->data.neigh_prio;
				   }
				   else if(cur_bdr_prio < temp->data.neigh_prio)
				   {
				   cur_bdr_ip.s_addr=temp->data.neighip.s_addr;
				   cur_bdr_id.s_addr=temp->data.neighid.s_addr;
				   cur_bdr_prio = temp->data.neigh_prio;
	  			   }
				   else if((cur_bdr_prio==temp->data.neigh_prio)
					 &&(cur_bdr_id.s_addr
						< temp->data.neighid.s_addr))
				   {
				   cur_bdr_ip.s_addr= temp->data.neighip.s_addr;
				   cur_bdr_id.s_addr= temp->data.neighid.s_addr;
				   cur_bdr_prio = temp->data.neigh_prio;
				   }
				   flag1 = 1;
				}
			   }//end if 2 way
			}//end for
		}//end if dflag==0
  	 // ELECTING DR ..............
			for(temp=neighbours->start->next;temp !=NULL;temp=temp->next)
		{
	  	  if(temp->data.state >= TWO_Way)
	  	  {
			if(temp->data.neighdr.s_addr==temp->data.neighid.s_addr)
			{ //HERE NEIGHBOUR IS DECLARING ITSELF DR 	
				if(dflag == 0)
				{
				cur_dr_ip.s_addr = temp->data.neighip.s_addr;
				cur_dr_id.s_addr = temp->data.neighid.s_addr;
				cur_dr_prio = temp->data.neigh_prio;
				}
				else if(cur_dr_prio < temp->data.neigh_prio)
				{
				cur_dr_ip.s_addr = temp->data.neighip.s_addr;
				cur_dr_id.s_addr = temp->data.neighid.s_addr;
				cur_dr_prio = temp->data.neigh_prio;
				}
				else if((cur_dr_prio == temp->data.neigh_prio)
				&&(cur_dr_id.s_addr< temp->data.neighid.s_addr))
				{
				cur_dr_ip.s_addr = temp->data.neighip.s_addr;
				cur_dr_id.s_addr = temp->data.neighid.s_addr;
				cur_dr_prio = temp->data.neigh_prio;
				}			
				dflag = 1;	
			}
		}
	  }//end if > 2 way
  	// NO ONE DECLARING ITSELF BDR
	}
	
	pif->if_bipa.s_addr = cur_bdr_id.s_addr;
	pif->if_dipa.s_addr = cur_dr_id.s_addr;

	display(3,5,5,"#######################################");
	display(7,4,1,"NEW BDR IS %s ",inet_ntoa(cur_bdr_id));
	display(7,4,1,"NEW DR IS  %s ",inet_ntoa(cur_dr_id));
	display(3,5,5,"#######################################");
	if(cur_bdr_id.s_addr == myid.s_addr)
	{
		pif->if_state = IFS_BACKUP;
		area[1].na_list = init_nla_llist();
		display(7,3,5,"I am BackUp Designated Router ");
	}
	else if(cur_dr_id.s_addr == myid.s_addr)
	{	
		struct ospf_na * nl;
		int i;
		struct node *temp;
		display(7,3,5,"I am Designated Router ");
nl=(struct ospf_na*)malloc(sizeof(struct ospf_na)+4*(pif->if_nblist.count-1));
		nl->header.lsa_age   = 0;
		nl->header.lsa_opts = 0 ;
		nl->header.lsa_type = LST_NLINK;
		nl->header.lsa_lsid.s_addr = pif->if_ipaddr.s_addr ;
		nl->header.lsa_rid.s_addr = myid.s_addr ;
		nl->header.lsa_seq  = 0;
		nl->header.lsa_cksum = 0;
	nl->header.lsa_len=  sizeof(struct ospf_na)+4*(pif->if_nblist.count-1);
		nl->na_mask.s_addr = pif->if_inmask.s_addr;
		for(i = 0 ,temp=pif->if_nblist.start->next;
			i<pif->if_nblist.count && temp != NULL ;
				i++,temp=temp->next)
		{
			nl->na_rid[i].s_addr=temp->data.neighip.s_addr;
		}
		pif->if_state = IFS_DR;
		area[1].na_list = init_nla_llist();
		nla_insert(area[1].na_list,*nl, pif->if_nblist.count);
	}
	else
	{
		if(area[1].na_list != NULL)
			nla_free(area[1].na_list);
		area[1].na_list = init_nla_llist();
		pif->if_state = IFS_DROTHER;
	}
	stopitimer(pif->if_twait[1]);
	if((old_dr_ip.s_addr != cur_dr_ip.s_addr)
		||(old_bdr_ip.s_addr != cur_bdr_ip.s_addr))
	{
		for(temp=neighbours->start->next; temp!= NULL; temp= temp->next)
		{	if(temp->data.state >= TWO_Way)
			nsm(&(temp->data), AdjOK);	
		}
	}	
}	

void reset_if(struct ospf_if *pif)
{
	inet_aton("0.0.0.0",&pif->if_dipa);
	inet_aton("0.0.0.0",&pif->if_bipa);

	// stop Wait timer
	// stop Hello timer
	// stop Router Dead timer
	// Generate KillNbr event for each Nieghbour
	
}

void a(int inum)	//inum == interface number.
{
	display(2,0,5,"\nWaitTimer Fired : It's Time To Elect DR/BDR");
	timerinterval[phyif[inum].if_twait[1]].status = FIRED;
	if(ifsm(IFE_WTIMER,&phyif[inum]) == -1)display(7,1,1,"ERROR in IFSM");
}

int ifsm(int e,struct ospf_if *pif)
{
	int state = pif->if_state;
	int event = e;
display(3,6,1,"IN IFSM State = %s event = %s",getIstate(state),getIevent(e));
	switch(state)
	{
		case IFS_DOWN:
			if(event == IFE_UP)
			{
//				printf("state=DOWN,event=InterfaceUP\n");
			// iniatialize HelloTimer......
			pif->if_thello[1]=addintervaltimer
				(pif->if_helloin,&send_hello);
				if((pif->if_type&
					(IFT_PT2PT|IFT_PT2M|IFT_VIRTUAL))!=0)
				{	
					pif->if_state = IFS_PP;
					return pif->if_state;
				}
				else if(!pif->if_rtrprio) // !ELIGIBLE TO BE DR
				{	
					pif->if_state = IFS_DROTHER;printf("j");
					return pif->if_state;
				}
				else
				{
					pif->if_state = IFS_WAITING;
					// initialize WaitTimer......
				pif->if_twait[1]=addsstimer(pif->if_rdin,&a);
					return pif->if_state;
				}
				if(pif->if_type == IFT_NBMA)
				{/* this condition is not handled */}
				break;
			}
			goto all;

		case IFS_WAITING:
			if(event == IFE_BSEEN)
			{
//printf("state=WAITING,event=BackupSeen\n");
				elect_drbdr(pif);
			break;
			}
			if(event == IFE_WTIMER)	
			{
//printf("state=WAITING,event=WaitTimer\n");
				elect_drbdr(pif);
			break;
			}
			goto all;

		case IFS_DROTHER:			//DROTHER, BACKUP and DR
			if(event == IFE_NCHG)
      			{
//printf("state=DROTHER,event=Neighbor Change\n");	
			elect_drbdr(pif);
			break;
			}
		case IFS_BACKUP:
			if(event == IFE_NCHG)
      		{printf("state=BACKUP,event=Neighbor Change\n"); 		
				elect_drbdr(pif);
			break;
			}
		case IFS_DR:
			if(event == IFE_NCHG)
      			{
//printf("state=DR,event=Neighbor Change\n");		
				elect_drbdr(pif);
			break;
			}
			goto all;

		case IFS_LOOPBACK:
			if(event == IFE_UNLOOPIND)
			{
//printf("state=LoopBack,event=UnloopInd\n");
				pif->if_state = IFS_DOWN;
			break;
			}
			
		default:
	all:		if(event==IFE_DOWN)
			{
//printf("state=ANY,event=InterfaceDown\n");
			   //Reset Interface variables & disable Timers......
				reset_if(pif);
				pif->if_state = IFS_DOWN;
				return pif->if_state;
			}
			if(event == IFE_LOOPIND)
			{
//printf("state=ANY,event=LoopInd\n");
			// Reset Interface variables & disable Timers......
				reset_if(pif);
				pif->if_state = IFS_LOOPBACK;
				return pif->if_state;
			}
		return pif->if_state;
	}
	return pif->if_state;
}


#endif // __IFSM_H

