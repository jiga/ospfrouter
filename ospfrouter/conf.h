/***************************************************************************
                  conf.h  -  defines configuration parameters
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

#ifndef __CONF_H
#define __CONG_H

#define MCAST 1

#define OSPF_VERSION 		2
#define PASSWORD 			"aajaaj"
#define HELLOINTERVAL 		4
#define ROUTERDEADINTERVAL 	20
#define RXMTINTERVAL		50
#define OPTIONS  	   		0 // currently no options are supported
#define ROUTERPRIORITY 		1
#define AREAID 		   		1
#define ROUTERID	   		"192.168.1.44"

#ifndef MCAST
#define ALLSPFRouter		"127.0.0.1"
#define MIP 				"127.0.0.1"
#define NMASK 				"255.255.255.0"
#define ALLDRouter 			"127.0.0.1"
#else
#define ALLSPFRouter		"224.0.0.5"
#define MIP 				"192.168.1.44"
#define NMASK 				"255.255.255.0"
#define ALLDRouter 			"224.0.0.6"
#endif

#define MAXIF	1	
#define MAXNBR	10

#endif // __CONF_H
