/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"

#define InterruptMode

static void isr(void* context, alt_u32 id){

	generateResponse();
	// Clear IRQ
	IOWR(STIMULUS_IN_BASE, 3, 0x0);
}

int background()
{
	IOWR(LED_PIO_BASE, 0, 0x1);
	int j;
	int x = 0;
	int grainsize = 4;
	int g_taskProcessed = 0;

	for(j = 0; j < grainsize; j++)
	{
		g_taskProcessed++;
	}
	IOWR(LED_PIO_BASE, 0, 0x0);
	return x;
}

void setUpEGM(int period){
	IOWR(EGM_BASE, 2, 2*period);
	IOWR(EGM_BASE, 3, period);
	IOWR(EGM_BASE, 0, 1);
}

void generateResponse(){
	// Generate response pulse
	IOWR(RESPONSE_OUT_BASE, 0, 0x1);
	IOWR(RESPONSE_OUT_BASE, 0, 0x0);
}

int main()
{

#ifdef InterruptMode

	printf("Using Interrupt:\n");
	while(1){
		if(IORD(BUTTON_PIO_BASE, 0) == 14){
			break;
		}
	}
	int background_count = 0;
	int latency = 0;
	int missed_pulses = 0;
	int duty_cycle = 50;

	alt_irq_register(STIMULUS_IN_IRQ, (void*) 0, isr);

	// turn on stimulus
	IOWR(STIMULUS_IN_BASE, 2, 0x1);

	int i = 0;
	for(i = 1; i <= 2500; i++){

		background_count = 0;
		setUpEGM(i);


		while(IORD(EGM_BASE, 1) != 0){
			if (background() == 0) {
				background_count++;
			}
		}

		latency = IORD(EGM_BASE, 4);
		missed_pulses = IORD(EGM_BASE, 5);
		printf("%d, %d, %d, %d, %d\n", i*2, latency, missed_pulses, background_count, duty_cycle);

		IOWR(EGM_BASE, 0, 0);
	}

#else
	printf("Using Tight Polling:\n");
	while(1){
		if(IORD(BUTTON_PIO_BASE, 0) == 14){
			break;
		}
	}
	//enable stimulus
	IOWR(STIMULUS_IN_BASE, 2, 1);

	int j = 0;
	for(j = 1; j <= 2500; j++){
		int background_count = 0;
		int cycles = 0;
		int max_task_count = 0;

		int duty_cycle = 50;
		int latency = 0;
		int missed_pulses = 0;

		int count = 0; // count of running tasks in one cycle
		int current_val = -10;
		int previous_val = -10;

		setUpEGM(j);

		previous_val = IORD(STIMULUS_IN_BASE, 0);

		while(IORD(EGM_BASE, 1)!=0){
			current_val = IORD(STIMULUS_IN_BASE, 0);

			if(previous_val == 0 && current_val == 1){	//rising edge detects
				generateResponse();
				count = 0;
				cycles++;
			}

			if(cycles == 1){
				if(background() == 0){
					background_count ++;
					max_task_count ++;
				}
			} else {
				if(count < max_task_count){
					if(background() == 0){
						background_count++;
						count++;
					}
				}
			}

			previous_val = current_val;
		}

		latency = IORD(EGM_BASE, 4);
		missed_pulses = IORD(EGM_BASE, 5);
		printf("%d,%d, %d, %d, %d,%d\n", j*2, latency, missed_pulses, background_count, duty_cycle, max_task_count);

		IOWR(EGM_BASE, 0, 0);
	}


	return 0;
#endif
}


