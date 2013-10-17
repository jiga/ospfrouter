/***************************************************************************
                  nalist.h  -  Linked-List for n/w advertisement
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

#ifndef	__NALIST_H
#define	__NALIST_H

struct nla_node
{
	struct nla_node * prev;
	struct nla_node * next;
	struct ospf_na data;
};

struct nla_llist
{
	struct nla_node * start;
	struct nla_node * end;
	int count;
};

struct nla_llist* nla_insert (struct nla_llist* l, struct ospf_na d,int non)
{
struct nla_node *n=(struct nla_node*)malloc(sizeof(struct nla_node)+4*(non-1));
	n->data = d;
	n->prev = l->end;
	n->next = NULL;
	l->end->next = n;
	l->count++;
//	printf(" in na INSERT ******* count = %d\n",l->count);
	l->end = n;
	return l;
}
void nla_free(struct nla_llist * l)
{
	struct nla_node * temp;
        if(l->start != NULL)
                for(temp = l->end; temp!= l->start;/**/ )
                {struct nla_node * t;
                        t = l->end->prev;
                        free(temp);
                        temp = t;
                }
}

void delete_nla_node(struct nla_llist *l,void * d)
{
/****	struct rla_node * s;// = l->start;

	for(s =l->start ; s !=l->end || s->data == d;s = s->next);//FOR ENDS HERE

	s->prev->next = s->next;
	s->next->prev = s->prev;
	free(s);
***********************/
}
struct nla_llist *init_nla_llist()
{	struct nla_llist * l;
	l = (struct nla_llist *)malloc(sizeof(struct nla_llist));
	l->start       = (struct nla_node*)malloc(sizeof(struct nla_node));
	l->end         = l->start;
	l->count       = 0;

	l->start->prev = NULL;
	l->start->next = NULL;
	l->end->next = NULL;
	l->end->prev = NULL;
	return l;
}
int search_nla_list(struct nla_llist *l ,struct in_addr lsid,struct in_addr rid, unsigned char type)
{
        struct nla_node *temp;
 //       printf("\nin search _list searching %s\n\n",inet_ntoa(rid));

        for(temp = l->start->next ;temp != NULL; temp = temp->next)
        if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
                   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
                      temp->data.header.lsa_type == type)
        {
    //            printf("nla searching\n");
                return 1;
        }
 //       printf("not found\n");
        return 0;
}

int replace_nla_list(struct nla_llist *l ,struct ospf_na * data, int non)
{
        struct in_addr lsid;
        struct in_addr rid;
        unsigned char type ;
        struct nla_node *temp;
	int interface;
	int numofnb  = non;
        lsid.s_addr = data->header.lsa_lsid.s_addr;
        rid.s_addr  = data->header.lsa_rid.s_addr;
        type       = data->header.lsa_type;
	
	interface = search_if(rid);
   //     printf("\nin search _list searching %s\n\n",inet_ntoa(rid));

        for(temp = l->start->next ;temp != NULL; temp = temp->next)
        if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
                   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
                      temp->data.header.lsa_type == type)
        {
		int size = sizeof(struct ospf_na) + (numofnb - 1) * 4 ;
      //          printf("nla replacing\n");
                memcpy(&temp->data,data,size);
                return 1;
        }
 //       printf("not found\n");
        return 0;
}

struct ospf_na get_nla_list(struct nla_llist *l ,struct in_addr lsid,struct in_addr rid, unsigned char type)
{
        struct nla_node *temp;
//        printf("\nin get nla_list searching %s\n\n",inet_ntoa(rid));

        for(temp = l->start->next ;temp != NULL; temp = temp->next)
        if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
                   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
                      temp->data.header.lsa_type == type)
        {
       //         printf("get nla searching\n");
                return temp->data;
        }
//        printf("not found\n");
        return ;
}

int new_nla(struct nla_llist *l ,struct in_addr lsid,struct in_addr rid,

        unsigned char type,long seq)
{
        struct nla_node *temp;
  //      printf("\nin new_nla searching %s\n\n",inet_ntoa(lsid));

        for(temp = l->start->next ;temp != NULL; temp = temp->next)
        if(temp->data.header.lsa_lsid.s_addr == lsid.s_addr &&
                   temp->data.header.lsa_rid.s_addr == rid.s_addr &&
                      temp->data.header.lsa_type == type &&
                                temp->data.header.lsa_seq < seq)
        {
   //             printf("nla_searching for new_nla\n");
                return 1;
        }
        return 0;
}

void print_nla_llist(struct nla_llist * l)
{
	struct nla_node *temp;
	for(temp = l->start->next ;temp != NULL; temp = temp->next)
	printf("\n na printing %s",inet_ntoa(temp->data.na_rid[0])); // ????
	
}

#endif	// __NALIST_H

