/***************************************************************************
                  ospf_if.h  -  Interface Data Structure
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

#ifndef __OSPF_IF_H
#define __OSPF_IF_H

/*
 * OSPF Interface Data Structures as specified in Section 9 of the rfc2328.
 */

struct ospf_if {
	u_int	 	if_type;	/* one of IFT_* below		*/
	u_int	 	if_state;	/* Interface State as defined in ifsm.h */
	struct in_addr	if_ipaddr;	/* Interface IP Address	*/
	struct in_addr	if_inmask;	/* Interface subnet Mask	*/
	uint32_t	if_areaid;	/* Area ID of attached n/w	*/
	u_int	 	if_helloin;	/* Hello Packet Interval	*/
	u_int	 	if_rdin;	/* Router Dead Interval		*/
	u_int	 	if_txdelay;	/* Interface Transmission Delay	*/
	uint8_t		if_rtrprio; /* Router Priority	*/	
	int		if_thello[2];	/* Hello Timer	0->interval,1->status*/
	int		if_twait[2];	/* Wait Timer for WAITING state	*/
	struct llist	if_nblist; /* Neighbor list of n/w	*/
	struct in_addr	if_dipa;	/* IP Address of desig. router	*/
	struct in_addr	if_bipa;	/* IP address of backup router	*/
	u_int	 	if_metric;	/* Cost to use this Interface	*/
	u_int		if_rxmtin;	/* Retransmit Interval	*/
	u_int		if_aytype;	/* Authentication Type	*/
	u_char	 	if_auth[AUTHLEN]; /* Authentication Key	*/

//	u_int	 	if_event;	/* Interface Event defined in ifsm.h */
//	struct ospf_if	*if_next;	/* Next Interface for Area	*/
};

struct ospf_if phyif[MAXIF];
/* Interface Types */

#define	IFT_PT2PT	0x01		/* Interface is Point-to-point	*/
#define	IFT_BROADCAST	0x02	/* Interface Supports Broadcast	*/
#define IFT_MULTICAST	0x04	/* Interface Supports Multicast	*/
#define	IFT_NBMA	0x08		/* Interface is Non Broadcast Multiaccess	*/
#define IFT_PT2M	0x10		/* Interface is Point-to-MultiPoint	*/
#define	IFT_VIRTUAL	0x20		/* Interface is a Virtual Link	*/

/*\
 *  OSPF uses a reference bandwidth of 100Mbps for cost calculation
\*/	

#define	ETHERNET_COST	10		// 10Mbps Ethernet n/w
#define	PT2PT_COST		1562	// 64Kbps Point-to-point n/w

/* The possible OSPF interface states.
 * These are defined in Section 9.1 of the rfc2328.
 * Defined as separate bits so that they can be grouped together
 * in FSM transitions.
 */
enum interface_state
{
    IFS_DOWN = 0,		// Interface is down
    IFS_LOOPBACK,	// Interface is looped
    IFS_WAITING,		// Waiting to learn Backup DR
    IFS_PP,			// Terminal state for P-P interfaces
    IFS_DROTHER,		// Mult-access: neither DR nor Backup
    IFS_BACKUP,		// Router is Backup on this interface
    IFS_DR,			// Router is DR on this interface

    N_IF_STATES	= 7,		/* # OSPF interface states */
};
// enum interface_state

/* OSPF interface events.
 * Defined in Section 9.2 of the rfc2328.
 */

enum interface_event
{
    IFE_UP  = 0,			// Interface has come up
    IFE_WTIMER,				// Wait timer has fired
    IFE_BSEEN,				// Backup DR has been seen
    IFE_NCHG,				// Associated neighbor has changed state
    IFE_LOOPIND,			// Interface has been looped
    IFE_UNLOOPIND,			// Interface has been unlooped
    IFE_DOWN,				// Interface has gone down

    N_IF_EVENTS	= 7,	// # OSPF interface events
};

char * getIstate(int state)
{
	switch(state)
	{
	case 0:
	 	return    "IFS_DOWN";
	case 1:
		return    "IFS_LOOPBACK";
	case 2:
		return    "IFS_WAITING";
	case 3:
		return    "IFS_PP";
	case 4:
		return    "IFS_DROTHER";
	case 5:
		return    "IFS_BACKUP";
	case 6:
		return    "IFS_DR";
  default :
  	return		"";
	};
}
char * getIevent(int state)
{
	switch(state)
	{
	case 0:
		return    "IFE_UP";
	case 1:
		return    "IFE_WTIMER";
	case 2:
		return    "IFE_BSEEN";
	case 3:
		return    "IFE_NCHG";
	case 4:
		return    "IFE_LOOPIND";
	case 5:
		return    "IFE_UNLOOPIND";
	case 6:
		return    "IFE_DOWN";
  default :
  	return		"";
	};
}

int search_if(struct in_addr rip)
{	int i;
	for(i = 0 ; i < MAXIF; i++)
	{
		if((phyif[i].if_ipaddr.s_addr & phyif[i].if_inmask.s_addr) ==
				(rip.s_addr & phyif[i].if_inmask.s_addr))
		return i ;
	}
return -1;
}
//SEARCHING FOR NB------>
struct ospf_nb * nb_search(struct in_addr nb_ip)
{	
	int index = search_if(nb_ip);
	if(index >= 0)
	{
		struct node *temp;	
		struct llist * l = &(phyif[index].if_nblist);

		for(temp= l->start->next ;temp != NULL; temp = temp->next)
		if(temp->data.neighip.s_addr == nb_ip.s_addr)
		{		
			return &(temp->data);
		}
	}
	return 0;
}

#endif //__OSPF_IF_H


