/***************************************************************************
                  llist.h  -  Linked-List for neighbor data structure
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

#ifndef	__LLIST_H
#define	__LLIST_H

struct node
{
	struct node * prev;
	struct node * next;
	struct ospf_nb data;
};

struct llist
{
	struct node * start;
	struct node * end;
	int count;
};


struct llist* insert (struct llist* l, struct ospf_nb d)
{
	struct node * n = (struct node*)malloc(sizeof(struct node));
	n->data = d;
	n->prev = l->end;
	n->next = NULL;
	l->end->next = n;
	l->count++;
	l->end = n;
	return l;
}

void delete_node(struct llist *l,void * d)
{
/****	struct node * s;// = l->start;

	for(s =l->start ; s !=l->end || s->data == d;s = s->next);//FOR ENDS HERE

	s->prev->next = s->next;
	s->next->prev = s->prev;
	free(s);
***********************/
}
struct llist *initllist()
{	struct llist * l;
	l = (struct llist *)malloc(sizeof(struct llist));
	l->start       = (struct node*)malloc(sizeof(struct node));
	l->end         = l->start;
	l->count       = 0;

	l->start->prev = NULL;
	l->start->next = NULL;
	l->end->next = NULL;
	l->end->prev = NULL;
	return l;
}

int search_list(struct llist *l , struct in_addr neigh_ip)
{
	struct node *temp;
	
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	if(temp->data.neighip.s_addr == neigh_ip.s_addr)
		return 1;
	return 0;
}

void print(struct llist * l)
{
	struct node *temp;
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	printf("\n llist printing %s",inet_ntoa(temp->data.neighip));
	
}

#endif	// __LLIST_H

