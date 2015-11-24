#include <stdio.h>

//This way we get access to the entry point of WYG
extern void WYG_main(void);

int main(int argc, char** argv) {
	
	WYG_main();
	
	return 1;
}

void testMain() {
	
	unsigned int window_a = newWindow(200, 200, 0, 0);
	
	installWindow(window_a, 1);
	moveWindow(window_a, 100, 100);
	markHandleVisible(window_a, 1);	
	moveWindow(window_a, 150, 150);
	
	while(1);
}
