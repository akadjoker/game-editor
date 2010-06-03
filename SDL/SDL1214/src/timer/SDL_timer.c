/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_timer.c,v 1.3 2002/03/06 11:23:02 slouken Exp $";
#endif

#include <stdlib.h>
#include <stdio.h>			/* For the definition of NULL */

#include "SDL_error.h"
#include "SDL_timer.h"
#include "SDL_timer_c.h"
#include "SDL_mutex.h"
#include "SDL_systimer.h"

#if 0 //AKR
#include "../../../../gameEngine/dlmalloc.h" //maks
#endif
///////////////////////////////////////////////////////
//maks: timer pause functions
#include "SDL_audio.h"

static Uint32 pauseInitTime = 0;
static int pauseMode = 0, pauseTotalTime = 0;
#if 0
Uint32 SDL_GetTicks(void) 
{
	if(pauseMode)
	{
		return pauseInitTime - pauseTotalTime;
	}

	return SDL_GetSystemTicks() - pauseTotalTime;
}
#endif
void SDL_PauseOn(void)
{
	if(!pauseMode)
	{
		pauseMode = 1;
		pauseInitTime = SDL_GetSystemTicks();
		SDL_PauseAudio(1);
	}
}

void SDL_PauseOff(void)
{
	if(pauseMode)
	{
		pauseMode = 0;
		pauseTotalTime += SDL_GetSystemTicks() - pauseInitTime;
		SDL_PauseAudio(0);
	}
}
int SDL_IsPaused(void)
{
	return pauseMode;
}
///////////////////////////////////////////////////////

/* #define DEBUG_TIMERS */

int SDL_timer_started = 0;
int SDL_timer_running = 0;

/* Data to handle a single periodic alarm */
Uint32 SDL_alarm_interval = 0;
SDL_TimerCallback SDL_alarm_callback;

static SDL_bool list_changed = SDL_FALSE;

/* Data used for a thread-based timer */
static int SDL_timer_threaded = 0;

/*struct _SDL_TimerID { //maks: define at .h file
	Uint32 interval;
	SDL_NewTimerCallback cb;
	void *param;
	Uint32 last_alarm;
	struct _SDL_TimerID *next;
};*/

static SDL_TimerID SDL_timers = NULL;
static Uint32 num_timers = 0;


#ifdef APPLICATION_THREAD_TIMERS //maks
#	define APPLICATION_THREAD_INVALID_MUTEX APPLICATION_THREAD_TIMERS
	static SDL_mutex *SDL_timer_mutex = APPLICATION_THREAD_TIMERS;
#else
#	define APPLICATION_THREAD_INVALID_MUTEX 0
	SDL_mutex *SDL_timer_mutex = 0;
#endif

/* Set whether or not the timer should use a thread.
   This should not be called while the timer subsystem is running.
*/
int SDL_SetTimerThreaded(int value)
{
	int retval;

	if ( SDL_timer_started ) {
		SDL_SetError("Timer already initialized");
		retval = -1;
	} else {
		retval = 0;
		SDL_timer_threaded = value;
	}
	return retval;
}

int SDL_TimerInit(void)
{
	int retval;

	SDL_timer_running = 0;
	SDL_SetTimer(0, NULL);
	retval = 0;
	if ( ! SDL_timer_threaded ) {
		retval = SDL_SYS_TimerInit();
	}
	if ( SDL_timer_threaded ) {
		SDL_timer_mutex = SDL_CreateMutex();
	}
	SDL_timer_started = 1;
	return(retval);
}

void SDL_TimerQuit(void)
{
	SDL_SetTimer(0, NULL);
	if ( SDL_timer_threaded < 2 ) {
		SDL_SYS_TimerQuit();
	}
	if ( SDL_timer_threaded ) {
		SDL_DestroyMutex(SDL_timer_mutex);
	}
	SDL_timer_started = 0;
	SDL_timer_threaded = 0;
}

int SDL_ThreadedTimerCheck(void) //maks
{
	Uint32 now, ms;
	SDL_TimerID t, prev, next;
	int removed, tRemoved;
	int executed = 0; //maks

	now = SDL_GetTicks();

	if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexP(SDL_timer_mutex); //maks
	for ( prev = NULL, t = SDL_timers; t; t = next ) {
		removed = 0;
		ms = t->interval - SDL_TIMESLICE;
		next = t->next;
		if ( (t->last_alarm < now) && ((now - t->last_alarm) > ms) ) {
			if ( (now - t->last_alarm) < t->interval ) {
				t->last_alarm += t->interval;
			} else {
				t->last_alarm = now;
			}
			list_changed = SDL_FALSE;
#ifdef DEBUG_TIMERS
			printf("Executing timer %p (thread = %d)\n",
						t, SDL_ThreadID());
#endif


			if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexV(SDL_timer_mutex); //maks
			ms = t->cb(t->interval, t->param);
			if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexP(SDL_timer_mutex); //maks

			tRemoved = 0; //maks
			executed = 1; //maks

			if(list_changed) 
			{
				if(prev) //maks: solve the crash in stars.ged when remove the timer
				{
					if(prev->next != t)
					{
						//t was removed
						tRemoved = 1;
					}
				}
				else if(SDL_timers != t)
				{
					//t was removed
					tRemoved = 1;
				}
			}

			if (!tRemoved && ms && ms != t->interval) //maks
			{
				//Solve the bug when create a new timer in cb function (list_changed = TRUE)
				t->interval = ROUND_RESOLUTION(ms);
			}

			if ( list_changed ) {
				/* Abort, list of timers has been modified */
				break;
			}

			if(!ms) //( ms != t->interval ) //maks
			{
				/*if ( ms ) 
				{
					t->interval = ROUND_RESOLUTION(ms);
				} 
				else*/ 
				{ /* Remove the timer from the linked list */
#ifdef DEBUG_TIMERS
					printf("SDL: Removing timer %p\n", t);
#endif
					if ( prev ) {
						prev->next = next;
					} else {
						SDL_timers = next;
					}
					free(t);
					-- num_timers;
					removed = 1;
				}
			}
		}
		/* Don't update prev if the timer has disappeared */
		if ( ! removed ) {
			prev = t;
		}
	}
	if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexV(SDL_timer_mutex); //maks

	return executed;
}

SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_NewTimerCallback callback, void *param)
{
	SDL_TimerID t;

	if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) //maks
	{
		if ( ! SDL_timer_mutex ) 
		{
			if ( SDL_timer_started ) 
			{
				SDL_SetError("This platform doesn't support multiple timers");
			} 
			else 
			{
				SDL_SetError("You must call SDL_Init(SDL_INIT_TIMER) first");
			}

			return NULL;
		}

		if ( ! SDL_timer_threaded ) 
		{
			SDL_SetError("Multiple timers require threaded events!");
			return NULL;
		}

		SDL_mutexP(SDL_timer_mutex);
	}

	t = (SDL_TimerID) malloc(sizeof(struct _SDL_TimerID));
	if ( t ) {
		t->interval = ROUND_RESOLUTION(interval);
		t->cb = callback;
		t->param = param;
		t->last_alarm = SDL_GetTicks();
		t->next = SDL_timers;
		SDL_timers = t;
		++ num_timers;
		list_changed = SDL_TRUE;
		SDL_timer_running = 1;
	}


#ifdef DEBUG_TIMERS
	printf("SDL_AddTimer(%d) = %08x num_timers = %d\n", interval, (Uint32)t, num_timers);
#endif
	if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexV(SDL_timer_mutex); //maks
	return t;
}

SDL_bool SDL_RemoveTimer(SDL_TimerID id)
{
	SDL_TimerID t, prev = NULL;
	SDL_bool removed;

	
	removed = SDL_FALSE;
	if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexP(SDL_timer_mutex); //maks
	/* Look for id in the linked list of timers */
	for (t = SDL_timers; t; prev=t, t = t->next ) {
		if ( t == id ) {
			if(prev) {
				prev->next = t->next;
			} else {
				SDL_timers = t->next;
			}			

			free(t);
			-- num_timers;
			removed = SDL_TRUE;
			list_changed = SDL_TRUE;
			break;
		}
	}
#ifdef DEBUG_TIMERS
	printf("SDL_RemoveTimer(%08x) = %d num_timers = %d thread = %d\n", (Uint32)id, removed, num_timers, SDL_ThreadID());
#endif
	if(SDL_timer_mutex != APPLICATION_THREAD_INVALID_MUTEX) SDL_mutexV(SDL_timer_mutex); //maks
	return removed;
}

static void SDL_RemoveAllTimers(SDL_TimerID t)
{
	SDL_TimerID freeme;

	/* Changed to non-recursive implementation.
	   The recursive implementation is elegant, but subject to 
	   stack overflow if there are lots and lots of timers.
	 */
	while ( t ) {
		freeme = t;
		t = t->next;
		free(freeme);
	}
}

/* Old style callback functions are wrapped through this */
static Uint32 callback_wrapper(Uint32 ms, void *param)
{
	SDL_TimerCallback func = (SDL_TimerCallback) param;
	return (*func)(ms);
}

int SDL_SetTimer(Uint32 ms, SDL_TimerCallback callback)
{
	int retval;

#ifdef DEBUG_TIMERS
	printf("SDL_SetTimer(%d)\n", ms);
#endif
	retval = 0;
	if ( SDL_timer_running ) {	/* Stop any currently running timer */
		SDL_timer_running = 0;
		if ( SDL_timer_threaded ) {
			SDL_mutexP(SDL_timer_mutex);
			SDL_RemoveAllTimers(SDL_timers);
			SDL_timers = NULL;
			SDL_mutexV(SDL_timer_mutex);
		} else {
			SDL_SYS_StopTimer();
		}
	}
	if ( ms ) {
		if ( SDL_timer_threaded ) {
			retval = (SDL_AddTimer(ms, callback_wrapper,
					       (void *)callback) != NULL);
		} else {
			SDL_timer_running = 1;
			SDL_alarm_interval = ms;
			SDL_alarm_callback = callback;
			retval = SDL_SYS_StartTimer();
		}
	}
	return retval;
}
