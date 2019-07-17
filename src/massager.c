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
#include <sys/msg.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <ncursesw/curses.h>
#include <sys/ioctl.h>

struct msgbuf
{
	    char time[50];
	    char mmsg[255];

};

struct pidmsg
{
	    long type;
	    char msg[255];

};

struct pidmsg pid_temp[5];

sigset_t set;

struct sembuf semlock[2]={0, 0, 0,  0, 1, 0};
struct sembuf semcheck={0, 0, 0};
struct sembuf semunlock={0, -1, 0};
int id, id_sem, ds;
	int pid;

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
	semctl(id_sem, 2, 0);
	msgctl(ds, IPC_RMID, 0);
	curs_set(1);
	keypad(stdscr, 0);
	delwin(wnd);
	delwin(wm);
	delwin(wt);
	endwin();
	echo();
	exit(1);

}

void* get_pid(void* ptr)
{

	struct pidmsg tmp;
	key_t key;

	printf("%d\n", pid);
	sprintf(tmp.msg,"%d", pid);
	key=ftok(".", 'b');
	ds=msgget(key, IPC_CREAT|0666);
	for (int i=0; i<5; i++)
	{
			if(pid_temp[i].msg[0]==0)
			{
				msgrcv(ds, &pid_temp[i], sizeof(struct pidmsg)-sizeof(long), 1, 0);
				tmp.type=atol(pid_temp[i].msg);
				msgsnd(ds, &tmp, sizeof(struct pidmsg)-sizeof(long), 0);
			}
	}
}

void kill_pid()
{
	for (int i=0; i<5; i++)
		if(pid_temp[i].msg[0]!=0)
			kill(SIGUSR1, atoi(pid_temp[i].msg));
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
		int signo;
	sigwait(&set, &signo);
	kill_pid();
	}

}



int main(void)

{
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);
	signal(SIGINT, ex);
	pthread_t tid;
	key_t key;
	key=ftok(".", 'b');
	id=shmget(key, sizeof(struct msgbuf)*10, IPC_CREAT|0666);
	id_sem=semget(key, 1, IPC_CREAT|0666);
	getwindow();
	pid=getpid();
	pthread_create(&tid, NULL, get_pid, 0);
	printmsg();




    return 0;

}

