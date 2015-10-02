void main(void) {

    //Spin forever and become the idle process
    //Probably a better way to do an idle process in the
    //future, but we need something that can always be
    //swapped in in case all other threads are sleeping
    //for messages, because if they're all sleeping then
    //we'll be locked into the kernel forever looping
    //through the scheduler trying to find a process to
    //swap in
    while(1);
}
