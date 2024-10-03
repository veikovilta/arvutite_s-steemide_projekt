



#include <stdio.h>
#include <wiringPi.h>
#include <mcp3004.h>

#define BASE 100
#define SPI_CHAN 1

int main(void) {
	int adc;
	wiringPiSetup();
	mcp3004Setup(BASE, SPI_CHAN);

	while (1) {
		adc = analogRead(BASE);
		printf("%d \n", adc);
		delay(500);
	}

	return 0;
}#include <stdio.h>
#include <wiringPi.h>
#include <mcp3004.h>

#define BASE 100
#define SPI_CHAN 1

int main(void) {
	int adc;
	wiringPiSetup();
	mcp3004Setup(BASE, SPI_CHAN);

	while (1) {
		adc = analogRead(BASE);
		printf("%d \n", adc);
		delay(500);
	}

	return 0;
}