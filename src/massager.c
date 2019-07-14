/*
 ============================================================================
 Name        : massager.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ==========================================================================

 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <ncursesw/curses.h>
#include <sys/ioctl.h>

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
	box(wt, 0, 0);
	box(wm, 0, 0);
	wprintw(wm, "Massage\n");
	wprintw(wt, "Time\n");
	wrefresh(wnd);
	wrefresh(wm);
	wrefresh(wt);
	keypad(stdscr, TRUE);
	//refresh();
}

void ex(int sig)
{
	shmctl(id, IPC_RMID, 0);
	semctl(id_sem, 1, 0);
	curs_set(1);
	keypad(stdscr, 0);
	delwin(wnd);
	delwin(wm);
	delwin(wt);
	endwin();
	echo();
	exit(1);

}




void printmsg()
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
	}

}



int main(void)

{
	signal(SIGINT, ex);
	pthread_t tid;
	key_t key;
	key=ftok(".", 'b');
	id=shmget(key, sizeof(struct msgbuf)*10, IPC_CREAT|0666);
	id_sem=semget(key, 1, IPC_CREAT|0666);
	getwindow();
	printmsg();




    return 0;

}

