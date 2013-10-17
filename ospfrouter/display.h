/***************************************************************************
                  display.h  -  displaying colored messages
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

#ifndef __DISPLAY_H
#define __DISPLAY_H

void display(int fg,int bg,int ib,char *fmt,...)
{
	va_list args;
	char *e = "\033[0m",*f,*b,*i;

	va_start(args,fmt);
	
	switch(fg)
	{
		case 0:
				f = "\033[30;1m";break;//DARK GRAY
		case 1:
				f = "\033[31;1m";break;//RED
		case 2:
				f = "\033[32;1m";break;//GREEN
		case 3:
				f = "\033[33;1m";break;// YELLOW
		case 4:
				f = "\033[34;1m";break;//DARK BLUE
		case 5:
				f = "\033[35;1m";break;//PURPLE
		case 6:
				f = "\033[36;1m";break;//CYAN
		case 7:
				f = "\033[37;1m";break;//WHITE
		case 8:
				f = "\033[38;1m";break;//LIGHT GRAY
		case 9:
				f = "\033[39;1m";break;//WHITE WITHHIGHINTENSITY
		default:
				f = "\033[0m;";break;
	};
	switch(bg)
	{
		case 0:
				b = "\033[40;1m";break;//BLACK
		case 1:
				b = "\033[41;1m";break;//RED
		case 2:
				b = "\033[42;1m";break;//GREEN
		case 3:
				b = "\033[43;1m";break;//ORANGE
		case 4:
				b = "\033[44;1m";break;//BLUE
		case 5:
				b = "\033[45;1m";break;//PURPLE
		case 6:
				b = "\033[46;1m";break;//CYAN
		case 7:
				b = "\033[47;1m";break;//WHITE
		case 8:
				b = "\033[48;1m";break;//BLACK
		case 9:
				b = "\033[49;1m";break;//BLACK
		default:
				b = "\033[0m;";break;
	};
	switch(ib)
	{
		case 0:
				i = "\033[0m";break;
		case 1:
				i = "\033[1m";break;
		case 2:
				i = "\033[2m";break;
		case 3:
				i = "\033[3m";break;
		case 4:
				i = "\033[4m";break;
		case 5:
				i = "\033[5m";break;
		case 6:
				i = "\033[6m";break;
		case 7:
				i = "\033[7m";break;
		case 8:
				i = "\033[8m";break;
		case 9:
				i = "\033[9m";break;
		default:
				i = "\033[0m";break;
	};

	printf(f);
	printf(b);
	printf(i);
	vprintf(fmt,args);
	printf(e);
	printf("\n");
	va_end(args);
	
}

#endif // __DISPLAY_H

