// dllmain.cpp : Defines the entry point for the DLL application.
/*
This is the expected json associated to the challenge:
{
	"FileName": "hash_challenge.dll",
	"Description": "This is a challenge that verifies the integrity of a file by checking its hash against a known value",
	"Props": {
		"validity_time": 3600,
		"refresh_time": 3000
	},
	"Requirements": "none"
}
*/


/////  FILE INCLUDES  /////

#include "pch.h"
#include "context_challenge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>





/////  GLOBAL VARIABLES  /////



/////  FUNCTION DEFINITIONS  /////

void getChallengeProperties();

/////  CUSTOM FUNCTIONS /////


char host[255] = "172.26.40.14";  //Change accordingly


//Extracted from other challenge
BOOL ping(char* url) {
	BOOL result = FALSE;
	BOOL retry = TRUE;

	// Send ping to the url and check if destination was reachable
	FILE* fp = NULL;
	char cmd_input_base[] = "ping -n 3 -w 1000 ";
	char* cmd_input = NULL;
	char* tmp_ptr = NULL;
	char cmd_result_line[1025];

	size_t url_len = 0;
	size_t cmd_input_base_len = 0;
	size_t cmd_input_size = 0;

	// Create the command
	url_len = strlen(url);
	cmd_input_base_len = strlen(cmd_input_base);
	cmd_input_size = (cmd_input_base_len + url_len + 1) * sizeof(char);
	cmd_input = (char*)malloc(cmd_input_size);
	if (cmd_input == NULL) {
		printf("Failed to launch a ping to %s (not enough memory)\n", url);
		return FALSE;
	}
	strcpy_s(cmd_input, cmd_input_size, cmd_input_base);
	strcpy_s(cmd_input + cmd_input_base_len, cmd_input_size - cmd_input_base_len, url);

	while (retry) {
		retry = FALSE;

		// Open a process with the output piped to a read stream
		fp = _popen(cmd_input, "r");
		if (fp == NULL) {
			printf("Failed to launch a ping to %s\n", url);
			exit(1);
		}

		// Read oputput line by line
		printf("Pinging %s\n", url);
		while (fgets(cmd_result_line, sizeof(cmd_result_line), fp) != NULL) {
			// In theory the statistics line is like this: "    (33% lost),"  and it is the only one with parenthesis and percentage characters
			tmp_ptr = strstr(cmd_result_line, "(");
			if (tmp_ptr != NULL) {
				tmp_ptr = strstr(tmp_ptr, "%");
				if (tmp_ptr != NULL) {
					if (tmp_ptr[-1] == '0' && tmp_ptr[-2] == '(') {                                                     // Looking for:   "(0%"
						//CHPRINT("All responses received\n");
						result = TRUE;
					}
					else if (tmp_ptr[-1] == '0' && tmp_ptr[-2] == '0' && tmp_ptr[-3] == '1' && tmp_ptr[-4] == '(') {  // Looking for: "(100%"
					 //CHPRINT("No responses received\n");
						result = FALSE;
					}
					else {
						printf("Inconsistent responses, retrying...\n");
						retry = TRUE;
						//result = TRUE;	If it was accessible at least one time, it is "accessible" at least in general
					}
				}
			}
		}

		// Close the piped process
		_pclose(fp);
	}

	printf("Result: %s\n", (result ? "OK" : "UNREACHABLE"));
	return result;
}



/////  FUNCTION IMPLEMENTATIONS  /////

int init(struct ChallengeEquivalenceGroup* group_param, struct Challenge* challenge_param){

	int result;


	// It is mandatory to fill these global variables
	group = group_param;
	challenge = challenge_param;
	if (group == NULL || challenge == NULL) {
		printf("---  Group or challenge are NULL \n");
		return -1;
	}
	printf("---  Initializing (%ws) \n", challenge->file_name);

	// Process challenge parameters
	getChallengeProperties();

	// It is optional to execute the challenge here
	result = executeChallenge();

	// It is optional to launch a thread to refresh the key here, but it is recommended
	if (result == 0) {
		launchPeriodicExecution();
	}

	return result;
}

int executeChallenge() {
	byte* new_key_data= (byte*)malloc(1);
    
	printf("---  Executing challenge (%ws)\n", challenge->file_name);

	// Nullity check
	if (group == NULL || challenge == NULL)
		return -1;

    // Calculate new key (size, data and expire date)
    int new_size = 1;
	
	new_key_data[0] = ping(host);

	time_t new_expires = time(NULL) + validity_time;

	// Update KeyData inside critical section
	EnterCriticalSection(&(group->subkey->critical_section));
	if ((group->subkey)->data != NULL) {
		free((group->subkey)->data);
	}
	group->subkey->data = new_key_data;
	group->subkey->expires = new_expires;
	group->subkey->size = new_size;
	LeaveCriticalSection(&(group->subkey->critical_section));
	return 0;	// Always 0 means OK.
}


void getChallengeProperties() {
	printf("---  Getting challenge parameters\n");
	json_value* value = challenge->properties;
	for (int i = 0; i < value->u.object.length; i++) {
		if (strcmp(value->u.object.values[i].name, "validity_time") == 0) {
			validity_time = (int)(value->u.object.values[i].value->u.integer);
		}
		else if (strcmp(value->u.object.values[i].name, "refresh_time") == 0) {
			refresh_time = (int)(value->u.object.values[i].value->u.integer);
		}
		else fprintf(stderr, "---  WARNING: the field '%s' included in the json configuration file is not registered and will not be processed.\n", value->u.object.values[i].name);
	}
	printf("---  Challenge properties: \n  validity_time = %d \n  refresh_time = %d \n ",
		validity_time, refresh_time);
}


BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
