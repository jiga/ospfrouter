/***************************************************************************
                  area.h  -  defines area data structure
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

#ifndef __AREA_H
#define __AREA_H

struct ospf_area
{
	int area_id;
	struct in_addr ipaddr;
	struct in_addr mask;
	struct ospf_if * thisif;	
	struct rla_llist * rla_list;//Router Link Advertisement
	struct nla_llist * na_list;//List of n/w LINKS ADVERTISEMENT
	struct ospf_lsa_header * ll;//LIST OF SUMMARY LINK ADVERTISEMENT
	//----SHORTEST TREE ------
	int au_type;
	int options;
	int default_cost;
	
};
struct ospf_area area[2];

#endif // __AREA_H


