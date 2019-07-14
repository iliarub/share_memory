/*
 ============================================================================
 Name        : massager.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ==========================================================================

 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <ncursesw/curses.h>
#include <sys/ioctl.h>
#include <termios.h>
struct msgbuf
{
	    char time[50];
	    char mmsg[255];

};

struct sembuf semlock[2]={0, 0, 0,  0, 1, 0};
struct sembuf semcheck={0, 0, 0};
struct sembuf semunlock={0, -1, 0};
int id, id_sem;

WINDOW* wnd;
WINDOW* wm;
WINDOW* wt;
WINDOW* winwrite;

int STR;
int STOLB;

void initwindow()

{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	STR=w.ws_row;
	STOLB=w.ws_col;
}

void sig_winch(int signo)
{
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
	resizeterm(size.ws_row, size.ws_col);
	initwindow();
}

void getwindow()
{
	initwindow();
	initscr();
	signal(SIGWINCH, sig_winch);
	cbreak();
	curs_set(0);
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_GREEN);
	wnd =newwin(STR, STOLB, 0, 0);
	wbkgd(wnd, COLOR_PAIR(1));
	wm= derwin(wnd, STR*8/10, STOLB-(2*STOLB/10), 0, 0);
	wt=derwin(wnd, STR*8/10, STOLB-(8*STOLB/10), 0, STOLB*8/10);
	winwrite=derwin(wnd, STR*2/10, STOLB, STR*8/10, 0);
	box(wt, 0, 0);
	box(wm, 0, 0);
	box(winwrite, 0, 0);
	wprintw(wm, "Massage\n");
	wprintw(wt, "Time\n");
	wprintw(winwrite, "Get massage:\n");
	wrefresh(wnd);
	wrefresh(wm);
	wrefresh(wt);
	wrefresh(winwrite);
	keypad(stdscr, TRUE);
	//refresh();
}

void* printmsg(void* ptr)
{
	struct msgbuf *msg;
	void *msgt = shmat(id, NULL, SHM_RDONLY);
	msg=(struct msgbuf*)msgt;
	while(1)
	{
		if(semop(id_sem, &semcheck , 1)==0)
		{

			for(int i=0; msg[i].mmsg[0]!='\0'; i++)
			{
				wmove(wm, i+1, 2);
				wprintw(wm, "%s", msg[i].mmsg);
				wmove(wt, i+1, 2);
				wprintw(wt, "%s", msg[i].time);
			}
		wrefresh(wm);
		wrefresh(wt);
		}
		else break;
	}


}

void writemsg()
{
	int j=0;
	long int ttime;
	struct msgbuf *msg;
	void *msgt = shmat(id, NULL, 0);
	msg=(struct msgbuf*)msgt;
	while(1)
	{
		char buf[255]="0";
		wmove(winwrite, 1, 1);
		int i=0;
		for(int ch=0;(ch=wgetch(winwrite))!=10&&i<255; i++)
		{
			buf[i]=ch;
		}
		i++;
		wmove(winwrite, 1, 1);
		while(i>0)
		{
			wechochar(winwrite, ' ');
			i--;
		}
		if(semop(id_sem, semlock, 2)==0)
		{

			for(;msg[j].mmsg[0]!=0;j++);

			ttime=time(NULL);
			strcpy(msg[j].mmsg, buf);
			strcpy(msg[j].time, ctime(&ttime));
			semop(id_sem, &semunlock, 1);

		}


	}
}

int main(void)

{
	pthread_t tid;
	key_t key;
	key=ftok(".", 'b');
	id=shmget(key, sizeof(struct msgbuf)*10, 0);
	while ((id_sem=semget(key, 1, 0))==-1);
	getwindow();
	pthread_create(&tid, NULL, printmsg, 0);
	writemsg();



    return 0;

}

