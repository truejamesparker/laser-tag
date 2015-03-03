/*
 * lockoutTimer.h
 *
 *  Created on: Feb 21, 2015
 *      Author: JAMES
 */

#ifndef LOCKOUTTIMER_H_
#define LOCKOUTTIMER_H_

// Standard init function.
void lockoutTimer_init();

// Calling this starts the timer.
void lockoutTimer_start();

// Returns true if the timer is running.
bool lockoutTimer_running();

// Standard tick function.
void lockoutTimer_tick();

void lockoutTimer_runTest();



#endif /* LOCKOUTTIMER_H_ */
