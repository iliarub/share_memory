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
	    char time;
	    char mmsg[255];

};
WINDOW* wnd;
WINDOW* wm;
WINDOW* wt;
int STR;
int STOLB;
void sig_winch(int signo)
{
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
	resizeterm(size.ws_row, size.ws_col);
	initwindow();
}

void initwindow()

{

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	STR=w.ws_row;
	STOLB=w.ws_col;
}


}

void getwindow()
{
	initscr();
	signal(SIGWINCH, sig_winch);
	cbreak();
	curs_set(0);
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_GREEN);
	wnd =newwin(STR, STOLB, 0, 0);
	wbkgd(wnd, COLOR_PAIR(1));
	wm= derwin(wnd, STR, STOLB-(2*STOLB/10), 0, 0);
	wt=derwin(wnd, STR, STOLB-(8*STOLB/10), 0, STOLB*8/10);
	wprintw(wm, "size\n");
	wprintw(wt, "name\n");
	wrefresh(wnd);
	wrefresh(wm);
	wrefresh(wt);
	keypad(stdscr, TRUE);
	refresh();
}

void printmsg(void* msgt)
{
	struct msgbuf *msg;
	msg=(struct msgbuf*)msgt;
	while(msg->mmsg[0]!=0)
	{
		wprintw(wm, "%s", msg->mmsg);
		wprintw(wm, "%s", msg->time);
	}

}

int main(void)

{
	int id, id_sem;
	key_t key;
	struct sembuf semlock[2]={0, 0, 0,  0, 1, 0};
	struct sembuf semcheck={0, 0, 0};
	struct sembuf semunlock={0, -1, 0};
	struct msgbuf buf[10];
	key=ftok("massager.c", 'b');
	id=shmget(key, sizeof(struct msgbuf), IPC_CREAT);
	id_sem=semget(key, 1, IPC_CREAT);
	void *mas = shmat(id, NULL, SHM_RDONLY);
	getwindow();
	while(1)
	{
		if(semop(id_sem, semcheck , 1))
			printmsg();
	}

    return 0;

}

