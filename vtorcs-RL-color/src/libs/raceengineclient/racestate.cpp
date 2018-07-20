/***************************************************************************

file        : racestate.cpp
created     : Sat Nov 16 12:00:42 CET 2002
copyright   : (C) 2002 by Eric Espi�
email       : eric.espie@torcs.org
version     : $Id: racestate.cpp,v 1.5 2005/08/17 20:48:39 berniw Exp $

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/** @file

@author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
@version	$Id: racestate.cpp,v 1.5 2005/08/17 20:48:39 berniw Exp $
*/

/* The Race Engine State Automaton */

#include <stdlib.h>
#include <stdio.h>
#include <tgfclient.h>
#include <raceman.h>
#include <racescreens.h>

#include "racemain.h"
#include "raceinit.h"
#include "raceengine.h"
#include "racegl.h"
#include "raceresults.h"
#include "racemanmenu.h"

#include "racestate.h"
#include "json.h"

//dosssman
// Using time for dump file name creation
// Also folder creation dependencies
#include "time.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tgf.h>
// end dosssman

static void *mainMenu;

// dosssman
// Counting eisode to name the dumped json play data
static int ep_counter = 0;
// Static var for dump file name
static char filepath[1024];
// end dosssman

/* State Automaton Init */
void ReStateInit(void *prevMenu) {
	mainMenu = prevMenu;
}

/* State Automaton Management         */
/* Called when a race menu is entered */
void ReStateManage(void) {
	int mode = RM_SYNC | RM_NEXT_STEP;

	do {
		switch (ReInfo->_reState) {
			case RE_STATE_CONFIG:
				//				printf("RE_STATE_CONFIG\n");
				if (getTextOnly()==false)
				{
					GfOut("RaceEngine: state = RE_STATE_CONFIG\n");
					/* Display the race specific menu */
					mode = ReRacemanMenu();
					if (mode & RM_NEXT_STEP) {
						ReInfo->_reState = RE_STATE_EVENT_INIT;
					}
				}
				// GIUSE - VISION HERE!!
				else if (getVision())
				{
					//					GfOut("RaceEngine: state = RE_STATE_CONFIG\n");
					/* Display the race specific menu */
					//					mode =
					ReRacemanMenu();
					//					if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
					//					}
				}
				else
				{
					ReInfo->_reState = RE_STATE_EVENT_INIT;
					ReRacemanMenu();
				}
				break;

			case RE_STATE_EVENT_INIT:
				//				printf("RE_STATE_EVENT_INIT\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_EVENT_INIT\n");
				/* Load the event description (track and drivers list) */
				mode = ReRaceEventInit();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				break;

			case RE_STATE_PRE_RACE:
				//				printf("RE_STATE_PRE_RACE\n");
				if (getTextOnly()==false)
				GfOut("RaceEngine: state = RE_STATE_PRE_RACE\n");
				mode = RePreRace();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_RACE_START;
				}
				break;

			case RE_STATE_RACE_START:
				//				printf("RE_STATE_RACE_START\n");
				if (getTextOnly()==false)
				GfOut("RaceEngine: state = RE_STATE_RACE_START\n");
				mode = ReRaceStart();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_RACE;
				}
				break;

			case RE_STATE_RACE:
				//				printf("RE_STATE_RACE\n");
				mode = ReUpdate();
				// GfOut( mode);
				if (ReInfo->s->_raceState == RM_RACE_ENDED) {
					/* race finished */
					ReInfo->_reState = RE_STATE_RACE_END;
				} else if (mode & RM_END_RACE) {
					/* interrupt by player */
					ReInfo->_reState = RE_STATE_RACE_STOP;
				}
				break;

			case RE_STATE_RACE_STOP:
				//				printf("RE_STATE_RACE_STOP\n");
				if (getTextOnly()==false)
				GfOut("RaceEngine: state = RE_STATE_RACE_STOP\n");
				/* Interrupted by player */
				// dosssman
				// TODO: Dump player data
				mode = ReRaceStop();
				if (mode & RM_NEXT_STEP) {
					if (RESTART==1)
					{
						RESTART=0;
						ReRaceCleanup();
						ReInfo->_reState = RE_STATE_PRE_RACE;
						if (getTextOnly()==false)
							GfuiScreenActivate(ReInfo->_reGameScreen);
						// GIUSE - VISION HERE!!
						else if (getVision())
							GfuiScreenActivate(ReInfo->_reGameScreen);
					}
					else
					{
						ReInfo->_reState = RE_STATE_RACE_END;
					}
				}
				break;

			case RE_STATE_RACE_END:
				//				printf("RE_STATE_RACE_END\n");
				if (getTextOnly()==false)
				GfOut("RaceEngine: state = RE_STATE_RACE_END\n");
				// mode = ReRaceEnd();
				// dosssman
				// dumping data
				dump_play_data();
				ep_counter++;

				mode = ReRaceEventInit();
				if(mode == RE_STATE_EXIT)
				{
					// dosssman
					// TODO: Dump player data

					// Original
					ReInfo->_reState=RE_STATE_EXIT;
					// End Original
					// GfOut( "RaceEngine: Reached the restart point\n");
					// ReRaceCleanup();
					// ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				else
				{
					if (mode & RM_NEXT_STEP) {
						// Dump play data use ReInfo params rquired probably
						// dosssman
						// TODO: Dump player data

						ReRaceCleanup();
						// end dosssman
						ReInfo->_reState = RE_STATE_PRE_RACE;
						// ReInfo->_reState = RE_STATE_POST_RACE;
					} else if (mode & RM_NEXT_RACE) {
						ReInfo->_reState = RE_STATE_RACE_START;
					}
				}
				break;

			case RE_STATE_POST_RACE:
				//				printf("RE_STATE_POST_RACE\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_POST_RACE\n");

				mode = RePostRace();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVENT_SHUTDOWN;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				break;

			case RE_STATE_EVENT_SHUTDOWN:
				//				printf("RE_STATE_EVENT_SHUTDOWN\n");
				if (getTextOnly()==false)
				GfOut("RaceEngine: state = RE_STATE_EVENT_SHUTDOWN\n");
				mode = ReEventShutdown();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_SHUTDOWN;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
				}
				break;

			case RE_STATE_SHUTDOWN:
				//				printf("RE_STATE_SHUTDOWN\n");
				case RE_STATE_ERROR:
				printf("RE_STATE_ERROR\n");
				if (getTextOnly()==false)
				GfOut("RaceEngine: state = RE_STATE_SHUTDOWN\n");
				/* Back to race menu */
				ReInfo->_reState = RE_STATE_CONFIG;
				mode = RM_SYNC;
				break;

			case RE_STATE_EXIT:
				//				printf("RE_STATE_EXIT\n");
				if (getTextOnly()==false)
					GfScrShutdown();
				// GIUSE - VISION HERE!!
				else if (getVision())
					GfScrShutdown();
				exit (0);		/* brutal isn't it ? */
				break;
		}


	} while ( ((mode & (RM_SYNC | RM_QUIT)) == RM_SYNC) || getTextOnly()) ;


	if (mode & RM_QUIT) {
		GfScrShutdown();
		exit (0);		/* brutal isn't it ? */
	}

	if (mode & RM_ACTIVGAMESCR) {
		GfuiScreenActivate(ReInfo->_reGameScreen);
	}
}

/* Change and Execute a New State  */
void ReStateApply(void *vstate) {

	long state = (long)vstate;

	ReInfo->_reState = (int)state;
	ReStateManage();
}

// dosssman
// Dumping firsts car play_data
// May be upgraded to all cars
void dump_play_data() {

	// Get car and play data
	tSituation *s = ReInfo->s;
	tCarElt *car = *(s->cars); // Takes the first car, mind you
	JsonNode *play_data = car->play_data;

	struct stat st = {0};
	// Generate respective folder name
	sprintf( filepath, "/tmp/torcs_play_data");

	// Creating root folder for torcs_play_data
	if (stat( filepath, &st) == -1) {
    mkdir( filepath, 0777);
	}

	//Creating folder for currentr game session
	sprintf( filepath, "%s/%s", filepath, getRecSessionStartStr());

	if (stat( filepath, &st) == -1) {
    mkdir( filepath, 0777);
	}

	// Generating filename for play_data;
	sprintf( filepath, "%s/%d.json", filepath, ep_counter);

	printf( "### DEBUG: Creating file\n");
	printf( "### DEBUG: %s\n", filepath);

	FILE *f = fopen( filepath, "w");

	if( f == NULL)
		printf("### DEBUG: Failed to created file\n");

	fprintf( f, json_encode( play_data));

	printf( "### DEBUG: Dumped JSON play_data: %s\n\n", filepath);

	fclose(f);
}
// end dosssman
