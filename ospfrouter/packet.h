/***************************************************************************
                  packet.h  -  OSPF packet formats
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

#ifndef __PACKET_H
#define __PACKET_H

#define AUTHLEN 8//8 bytes
/* OSPF packet format */

struct ospf_header
{
	unsigned char	ospf_version;	/* Version Number		*/
	unsigned char	ospf_type;	/* Packet Type			*/
	unsigned int	ospf_len;	/* Packet Length		*/
	struct in_addr ospf_rid;	/* Router Identifier		*/
	unsigned long	ospf_aid;	/* Area Identifier		*/
	unsigned int	ospf_cksum;	/* Check Sum			*/
	unsigned int	ospf_autype;/* Authentication Type	*/
	char            ospf_auth[AUTHLEN];/*Authentication Field	*/
};

#define	MINHDRLEN	32		/* OSPF base header length	*/

/* OSPF Packet Types */

#define	T_HELLO		1	/* Hello packet		*/
#define	T_DATADESC	2	/* Database Description		*/
#define	T_LSREQ		3	/* Link State Request	*/
#define	T_LSUPDATE	4	/* Link State Update		*/
#define	T_LSACK		5	/* Link State Acknowledgement//NA*/

/* OSPF Authentication Types */

#define	AU_NONE		0	/* No Authentication	*/
#define	AU_PASSWD	1	/* Simple Password	*/

/* OSPF Hello Packet */

struct ospf_hello
{
	struct ospf_header header;      /* Header			*/
	struct in_addr  oh_netmask;
	unsigned int	oh_helloin ;	/* Hello Interval (seconds)	*/
	unsigned char	oh_opts;	/* Options			*/
	unsigned char	oh_rtrprio;	/* Sender's Router Priority	*/
	unsigned long	oh_rdin;	/* Seconds Before Declare Dead*/
	struct in_addr  oh_drid;
	struct in_addr  oh_bdrid;
	struct in_addr  oh_neighbor[1]; /* Living Neighbors<---	*/
};
#define	MINHELLOLENGTH	(MINHDRLEN + 20) //includeing auth ==== 52

//struct ospf_lss;
/* OSPF Link State Summary */

struct ospf_lsa_header
{
	unsigned int	lsa_age;	/* Time (secs) Since Originated*/
	unsigned char	lsa_opts;	/* Options Supported		*/
	unsigned char	lsa_type;	/* LST_* below			*/
	struct in_addr  lsa_lsid;	/* Link State Identifier	*/
	struct in_addr  lsa_rid;	/* Advertising Router Identifier*/
	unsigned long	lsa_seq;	/* Link State Adv. Sequence #	*/
	unsigned int	lsa_cksum;	/* Fletcher Checksum of LSA	*/
	unsigned int	lsa_len;	/* Length of Advertisement	*/
};

struct ospf_dd
{
	struct ospf_header header;      /* Header			*/
	unsigned int	dd_mtu;		/* Interface MTU		*/
	unsigned char	dd_opts;	/* Options			*/
	struct{
		unsigned  char mast_slv :1;
		unsigned  char more :1;
		unsigned  char init :1;
	      }flags;
	char notusing[3];
	char dd_seq;	/* Sequence Number	*/
	struct ospf_lsa_header	dd_lsa[1]; /* Link State Advertisements	*/
};


#define	MINDDLEN	(MINHDRLEN + 8)

/* ospf_ls.h */

#define	LSRLEN		12
#define MAXAGE		100
#define	LSSHDRLEN	20
/* Link State Advertisement Types */

#define	LST_RLINK	1		/* Router Link			*/
#define	LST_NLINK	2		/* Network Link			*/
#define	LST_SLINK	3  //	NA	/* IP Network Summary Link	*/
#define	LST_BRSLINK	4 //	NA	/* AS Border Router Summary	*/
#define	LST_EXTERN	5 //	NA	/* AS External Link		*/

/* Link State Advertisement (min) Lengths */

#define	LSA_RLEN	(LSSHDRLEN + 4)
#define	LSA_NLEN	(LSSHDRLEN + 4)

#define	LSA_ISEQ	0x80000001

/* OSPF Link State Advertisement */

#define	MAXLSDLEN	64	/* Max LS Data Len (configurable)	*/

/* Router Links Advertisement */
struct router_link
{
	struct in_addr  ra_lid;		/* Link ID			*/
	struct in_addr	ra_data;	/* Link Data			*/
	unsigned char	ra_type;	/* Link Type (RAT_* Below)	*/
	unsigned char	ra_ntos;	/* # of Types-of-Service Entries*/
	unsigned int	ra_metric;	/* TOS 0 Metric			*/
};

struct  ospf_ra
{
	struct ospf_lsa_header header;	/* Header		*/
	struct{
		unsigned char abr  :1;
		unsigned char asbr :1;
		unsigned char virt :1;
	      }flags;
	unsigned char	ra_mbz;         /* Must Be Zero		*/
	unsigned int	ra_nlinks;	/* # of Links This Advertisement*/
	struct router_link links[1];
};
#define	MINRLLEN	12

#define	RAO_ABR		0x01	/* Router is Area Border Router*/
#define	RAO_EXTERN	0x02		/* Router is AS Boundary Router*/

#define	RAT_PT2PT	1		/* Point-Point Connection	*/
#define	RAT_TRANSIT	2		/* Connection to Transit Network*/
#define	RAT_STUB	3		/* Connection to Stub Network	*/
#define	RAT_VIRTUAL	4		/* Virtual Link			*/

/* Network Links Advertisement */

struct ospf_na
{
	struct ospf_lsa_header header;  /* Header		      */
	struct in_addr	na_mask;        /* Network Mask		      */
	struct in_addr	na_rid[1];      /* <------na_rid[2]-----------*/
					/* IDs of All Attached Routers*/
};

/* Summary Link State Advertisement (type 3,4) */

struct ospf_sa
{
	struct ospf_lsa_header header;	/* Header		      */
	struct in_addr	sa_mask;	/* Network Mask		      */
	unsigned char	sa_mbz;         /* Must Be Zero		      */
	unsigned char	sa_metric[3];   /* Metric		      */
	unsigned char	sa_tos;         /* TOS			      */
	unsigned char	sa_tosmetric[3]; /* TOSMetric		      */
};

/* AS-External LSA */

struct ospf_ASE
{
	struct ospf_lsa_header header;	/* Header		      */
	struct in_addr	ase_mask;       /* Network Mask		      */
	struct {
		unsigned int mbz :7;
		unsigned int e :1;
	       }e1;
	unsigned char	ase_metric[3];
	struct in_addr  ase_fwd_addr;	/* Forwarding Address	      */
	unsigned long	ase_route_tag;	/* External Route Tag	      */
	struct {
		unsigned int tos :7;
		unsigned int e :1;
	       }e2;
	unsigned char	ase_tosmetric[3];
	struct in_addr  ase_fwd_addr2;	/* Forwarding Address	      */
	unsigned long	ase_route_tag2;	/* External Route Tag	      */
};

/* OSPF Database Description Packet */
/* OSPF Link State Request Packet */

struct ospf_lsr
{
	struct ospf_header header;      /* Header			*/
	unsigned long	lsr_type;	/* Link State Type		*/
	struct in_addr	lsr_lsid;	/* Link State Identifier	*/
	struct in_addr	lsr_rid;	/* Advertising Router		*/
};

/* Link State Update Packet Format */

struct ospf_lsu
{
	struct ospf_header header;      /* Header			*/
	unsigned long	lsu_nads;	/* # Advertisments This Packet*/
	char		lsu_data[1];	/* 1 or more struct ospf_lsa's*/
};

/* Link State Acknowledgement Packet Format */

struct ospf_lsack
{
	struct ospf_header header;      /* Header			*/
	struct ospf_lsa_header lsheader; /* LSA Header			*/
};

#define	MINLSULEN	(MINHDRLEN + 4)	/* Base LSU Length	*/

#endif// __PACKET_H


