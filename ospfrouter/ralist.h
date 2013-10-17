/***************************************************************************
                  ralist.h  -  Linked-List for router advertisement
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

#ifndef	__RALIST_H
#define	__RALIST_H

struct rla_node
{
	struct rla_node * prev;
	struct rla_node * next;
	struct ospf_ra data;
};

struct rla_llist
{
	struct rla_node * start;
	struct rla_node * end;
	int count;
};

struct rla_llist* rla_insert (struct rla_llist* l, struct ospf_ra d)
{
	struct rla_node * n = (struct rla_node*)malloc(sizeof(struct rla_node));
	n->data = d;
	n->prev = l->end;
	n->next = NULL;
	l->end->next = n;
	l->count++;
//	printf("in rla INSERT ********* count = %d\n",l->count);
	l->end = n;
	return l;
}

void delete_rla_node(struct rla_llist *l,void * d)
{
/****	struct rla_node * s;// = l->start;

	for(s =l->start ; s !=l->end || s->data == d;s = s->next);//FOR ENDS HERE

	s->prev->next = s->next;
	s->next->prev = s->prev;
	free(s);
***********************/
}
struct rla_llist *init_rla_llist()
{	struct rla_llist * l;
	l = (struct rla_llist *)malloc(sizeof(struct rla_llist));
	l->start       = (struct rla_node*)malloc(sizeof(struct rla_node));
	l->end         = l->start;
	l->count       = 0;

	l->start->prev = NULL;
	l->start->next = NULL;
	l->end->next = NULL;
	l->end->prev = NULL;
	return l;
}

int search_rla_list(struct rla_llist *l ,struct in_addr lsid,struct in_addr rid, unsigned char type)
{
	struct rla_node *temp;
//	printf("\nin search _list searching %s\n\n",inet_ntoa(rid));
	
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
		   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
		      temp->data.header.lsa_type == type)
	{		
//		printf("rla searching\n");
		return 1;
	}
//	printf("not found\n");
	return 0;
}

int replace_rla_list(struct rla_llist *l ,struct ospf_ra data)
{
	struct in_addr lsid;
	struct in_addr rid;
	unsigned char type ;
	struct rla_node *temp;

	lsid.s_addr = data.header.lsa_lsid.s_addr;
	rid.s_addr  = data.header.lsa_rid.s_addr;
	type 	   = data.header.lsa_type;
//	printf("\nin search _list searching %s\n\n",inet_ntoa(rid));
	
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
		   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
		      temp->data.header.lsa_type == type)
	{		
//		printf("rla replacing\n");
		memcpy(&temp->data,&data,LSSHDRLEN+MINRLLEN+4);
		return 1;
	}
//	printf("not found\n");
	return 0;
}
struct ospf_ra get_rla_list(struct rla_llist *l ,struct in_addr lsid,struct in_addr rid, unsigned char type)
{
        struct rla_node *temp;
 //       printf("\nin get rla_list searching %s\n\n",inet_ntoa(rid));

        for(temp = l->start->next ;temp != NULL; temp = temp->next)
        if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
                   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
                      temp->data.header.lsa_type == type)
        {
  //              printf("get rla searching\n");
		return temp->data;
        }
 //       printf("not found\n");
        return ;
}

int new_rla(struct rla_llist *l ,struct in_addr lsid,struct in_addr rid,
											unsigned char type,long seq)
{
	struct rla_node *temp;
//	printf("\nin new_rla searching %s\n\n",inet_ntoa(lsid));
	
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
		   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
		      temp->data.header.lsa_type == type &&
				temp->data.header.lsa_seq < seq)
	{		
//		printf("rla_searching for new_rla\n");
		return 1;
	}
	return 0;
}

void print_rla_llist(struct rla_llist * l)
{
	struct rla_node *temp;
	for(temp = l->start->next ;temp != NULL; temp = temp->next);
//printf("\nrla printing %s",inet_ntoa(temp->data.links[0].ra_lid)); // ????
	
}

#endif	// __RLALIST_H

