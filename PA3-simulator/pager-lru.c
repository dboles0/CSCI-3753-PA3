/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];


    /* Loocal vars */
    int proctmp;
    int pagetmp;
    int pc;
    int proc;
    int page;
    int min_page;
    int chosen_proc;
    int chosen_page;

    /* initialize static vars on first run */
    /* zero-fill-on-deman */
    if(!initialized){
    	/* Select first active process */
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
		for(pagetmp=0; pagetmp < MAXPROCESSES; pagetmp++){
			timestamps[proctmp][pagetmp] = 0;
		}
	}
    initialized = 1;
    }	


    /* TODO: Implement LRU Paging */
    /* loop through process frames*/	
    for(proc=0; proc < MAXPROCESSES; proc++){
	
		/* Is process active? = Yes */
		if(q[proc].active) {
		    /* Dedicate all work to first active process*/
		    pc = q[proc].pc;	 	        // program counter for process
		    page = pc/PAGESIZE; 		// page the program counter needs

		    /* Is Page Swapped In? = No */
		    /* if we do not find the given page in memory then continue*/
		    if(!q[proc].pages[page]) {

			/* Call pagin() = Failure */
			/* if we cannot swap the page then we need to do some work*/
			if(!pagein(proc,page)) {
			
				/* Select a Page to Evict */	


				/* initialize min_page and chosen proc/page*/
				min_page = timestamps[0][0];
				chosen_proc = 0;
				chosen_page = 0;

				/* find the page that has not been used for the longest time period */
				/* look throught all processes to find lowest timestamp */
				for(int check_proc = 0; check_proc < MAXPROCESSES; check_proc++){
					for(int check_page = 0; check_page < MAXPROCPAGES; check_page++){

						//if we have a memory block that is smaller and exists in memeory update 
						if(min_page > timestamps[check_proc][check_page] && q[proc].pages[check_page]){
							min_page = timestamps[check_proc][check_page];
							chosen_proc = check_proc;
							chosen_page = check_page;
						}
					}
				}
				
    		    		timestamps[chosen_proc][chosen_page] = tick;      // set time stamp for swaped process

				/* Call pageout() = Failure */
				if(!pageout(chosen_proc, chosen_page)){
					//investigate failure
					//perror("pageout error");
					break;
				} /* Call pageout() = Success*/

			} /* Call pagin() = Success */
			// in the event swap success increment tick 
			else {
    		    		timestamps[proc][page] = tick;      
			}

		    } /* Is Page Swapped In? = Yes */

		} /* Is process active? = No */

    // advance the tick count for the next processes
    tick++;

    } /* Another Process? = Yes/No */

}

// pool of process empty spaces 
// keep a pointer at the end of each process 
// when it is time to look for memory choose the process with the smallest poitner counter
// find out how much large of space each program is allocated 
// use this process memory spots until we reach a threashold of 
