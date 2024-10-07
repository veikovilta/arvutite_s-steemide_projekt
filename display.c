#include <stdio.h>

void * displayInfo(void* arg)
{
    displayToScreen();

    return NULL;
}

void displayToScreen(void)
{
	printf("Hello"); 
}
 
