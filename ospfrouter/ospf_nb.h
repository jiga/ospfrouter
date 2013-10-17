/***************************************************************************
                  ospf_nb.h  -  Neighbor Data Structure
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

#ifndef __OSPF_NB_H
#define __OSPF_NB_H

struct ospf_nb
{
	int state;                              // State
	int inactiv_timer[2];//[0]==interval [1]=index//Inactivity Timer
	int mast_slv;//=dd.flags.mast_slv;		//Master Slave<-boolean
	int more ;
	long ddseqno;//=dd.dd_seq;			//DD Sequence No.
	struct ospf_dd lastddrecv;//=lastdd;	//Last DD Received
	struct in_addr neighid;//=hello.oh_neighbour[1];//Neighbour ID
	unsigned char neigh_prio;//=hello.oh_prio;	// Neighbour Priority
	struct in_addr neighip;			// Neighbour IP
	unsigned char neigh_opts;//=hello.oh_opts;	// Options
	struct in_addr neighdr;			// Neighbour DR
	struct in_addr neighbdr;		// Neighbour BDR
	struct lss_llist *lsretrx;	//lsa retransmisson list
	struct lss_llist *dbsl;		//database summary list
//	struct ospf_ra  *ralist;		//router adv list
//	struct ospf_na  *nalist;		//network list
	struct lss_llist *lsreq;		//link state request list
	int rxmtindex ;
};

enum neighbour_state
{	Down     =0,
	Attempt  =1,
	Init     =2,
	TWO_Way  =3,
	ExStart  =4,
	Exchange =5,
	Loading  =6,
	Full     =7
};

enum neighbour_event
{	Start             =0,
	HelloReceived     =1,
	ONE_WayReceived   =2,
	TWO_WayReceived   =3,
	NegotiationDone   =4,
	ExchangeDone      =5,
	BadLSReq          =6,
	LoadingDone       =7,
	AdjOK             =8,
	SeqNumberMismatch =9,
	ONE_Way           =10,
	KillNbr           =11,
	InactivityTimer   =12,
	LLDown            =13
};

char * getNevent(int state)
{
	switch(state)
	{
	case 0:
	 	return     "Start ";
	case 1:
		return     "HelloReceived";
	case 2:
		return     "ONE_WayReceived";
	case 3:
		return     "TWO_WayReceived";
	case 4:
		return     "NegotiationDone";
	case 5:
		return     "ExchangeDone";
	case 6:
		return     "BadLSReq";
	case 7:
		return 	   "LoadingDone";
	case 8:
		return     "AdjOK";
	case 9:
		return	   "SeqNumberMismatch";
	case 10:
		return     "ONE_Way";
	case 11:
		return     "KillNbr";
	case 12:
		return	   "InactivityTimer";
	case 13:
		return     "LLDown";
  default :
  	return		"";
	};
}

char * getNstate(int e)
{
	switch(e)
	{
	case 0:
		return    "Down";
	case 1:
		return    "Attempt";
	case 2:
		return    "Init";
	case 3:
		return    "TWO_Way";
	case 4:
		return    "ExStart";
	case 5:
		return    "Exchange";
	case 6:
		return    "Loading";
	case 7:
		return   "Full";
  default :
  	return		"";
	};
}

#endif // __OSPF_NB_H

