/***************************************************************************
                  lsslist.h  -  Linked-List for Link state summary
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

#ifndef	__LSSLIST_H
#define	__LSSLIST_H

struct lss_node
{
	struct lss_node * prev;
	struct lss_node * next;
	struct ospf_lsa_header data;
};

struct lss_llist
{
	struct lss_node * start;
	struct lss_node * end;
	int count;
};

struct lss_llist* lss_insert (struct lss_llist* l, struct ospf_lsa_header d)
{
	struct lss_node * n = (struct lss_node*)malloc(sizeof(struct lss_node));
	n->data = d;
	n->prev = l->end;
	n->next = NULL;
	l->end->next = n;
	l->count++;
	//printf(" in lss INSERT ****** count = %d\n",l->count);
	l->end = n;
	return l;
}

void delete_lss_node(struct lss_llist *l)
{	if(l->start != NULL)
           if(l->start->next != NULL)
           {
	          struct lss_node * tofree= l->start->next;
        	  if(l->start->next->next != NULL)
                	   l->start->next
                        	= l->start->next->next;
	         else
        	           l->start->next = NULL;
	         if(l->end == tofree)
        	           l->end = l->start;
	         l->count = l->count -1;
        	 free(tofree);
          }

}
struct lss_llist *init_lss_llist()
{	struct lss_llist * l;
	l = (struct lss_llist *)malloc(sizeof(struct lss_llist));
	l->start       = (struct lss_node*)malloc(sizeof(struct lss_node));
	l->end         = l->start;
	l->count       = 0;

	l->start->prev = NULL;
	l->start->next = NULL;
	l->end->next = NULL;
	l->end->prev = NULL;
	return l;
}

void lss_fill_next(struct lss_llist *l, unsigned long * type,struct in_addr *lsid, struct in_addr * rid )
{
	if(l->start != NULL)
        *type        = l->start->next->data.lsa_type;
	(*lsid).s_addr = l ->start->next->data.lsa_lsid.s_addr;
	(*rid).s_addr  = l->start->next->data.lsa_rid.s_addr;
}

void lss_free(struct lss_llist * l)
{struct lss_node * temp;
	if(l->start != NULL)
		for(temp = l->end; temp!= l->start;/**/ )
		{struct lss_node * t;
			t = l->end->prev;
			free(temp);
			temp = t;
		}
}

int search_lss_list(struct lss_llist *l , struct in_addr id)
{
	/****************************struct rla_node *temp;
	printf("\nin search _list searching %s\n\n",inet_ntoa(neigh_ip));
	printf("fjhfdsjhkfsgdjh  %d\n",l->count);
	
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	if(temp->data.neighip.s_addr == neigh_ip.s_addr)
	{		
		printf("searching\n");
		return 1;
	}
	printf("searching\n");
	return 0;**************************************/
}

void print_lss_llist(struct lss_llist * l)
{
	struct lss_node *temp;
	for(temp = l->start->next ;temp != NULL; temp = temp->next);
//	printf("\n lss printing %s",inet_ntoa(temp->data.lsa_rid)); // ????
	
}

#endif	// __LSSLIST_H

