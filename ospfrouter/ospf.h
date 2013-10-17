/***************************************************************************
                  ospf.h  -  Library files & includes
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

#ifndef __OSPF_H
#define __OSPF_H

#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<syslog.h>
#include<net/if.h>
#include<sys/time.h>
#include<signal.h>

#include "conf.h"
#include "packet.h"
#include "display.h"

#include "ospf_nb.h"
#include "llist.h"
#include "ospf_if.h"

#include "lsslist.h"
#include "ralist.h"
#include "nalist.h"

#include "area.h"
#include "timer.h"

#include "send_pkt.h"
#include "process_pkt.h"

#include "ifsm.h"
#include "nsm.h"

#endif// __OSPF_H


