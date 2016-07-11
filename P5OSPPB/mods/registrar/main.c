#include "../include/p5.h"
#include "../include/registrar.h"

/*
 * This is the Registrar, one of the most vital parts of a working P5 system
 * It serves two major functions. The first is that it spins up the init process
 * which will spawn all the other servers and startup processes. When it has done
 * that, it proceeds to enter a message loop waiting for registration commands.
 * Registration commands add or remove entries into the service table which the
 * registrar maintains and which allows for applications to look up the PID of
 * a registered server based on its service code, which specifies the service
 * that server serves
 *
 * Note that this whole system works because the registrar is always guaranteed
 * to have PID 1 as it is the first process started by the kernel
 *
 * The registrar understands the following messages:
 * ---------------------------------------------------
 *    Command: REG_REGISTER
 *    Payload: <requested service code>
 *      The register command inserts a new entry into the table matching the PID
 *      of the calling process to the supplied service code. At the moment,
 *      if there is a collision of service codes the registrar overwrites the
 *      previous server PID with the new one. If everything worked, we send
 *      back the command REG_STATUS with 1 in the payload. If we're full up
 *      on registered processes, we do the same but put 0 in the payload
 *
 *    Command: REG_LOOKUP
 *    Payload: <requested service code>
 *      The lookup command fetches the registered PID (or zero if the service is
 *      not registered) and sends a message back to the sender with command
 *      REG_PID and the PID in the payload
 *
 *    Command: REG_DEREGISTER
 *    Payload: ignored
 *      Finds the PID of the caller in the service table and zeros that entry
 *      --This message does not send a response
 */


//This can and should be replaced by a malloc in the future
unsigned int reg_table_pid[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned int reg_table_svc[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void printTable() {

    int i;

    prints("\nSERVICE  |  PID\n---------|---------\n");

    for(i = 0; i < 20; i++) {

        printHexDword(reg_table_svc[i]);
        prints(" | ");
        printHexDword(reg_table_pid[i]);
        pchar('\n');
    }
}

void main(void) {

    message temp_msg;
    int i;

    //Make sure reg table is cleared
    for(i = 0; i < 20; i++)
        reg_table_pid[i] = reg_table_svc[i] = 0;

    prints("[registrar] Starting init process...\n");

    //Start the init process
    startSuperProc(":init.mod");

    //Enter the message loop
    while(1) {
        getMessage(&temp_msg); //Putting this in a loop shouldn't actually matter now since the OS sleeps procs waiting for a message

        switch(temp_msg.command) {

            case REG_REGISTER:
                //Check to see if we need to replace a PID
                for(i = 0; (i < 20) && (reg_table_svc[i] != temp_msg.payload); i++);

                if(i == 20) {

                    //Nothing matched, so we make a new one
                    for(i = 0; i < 20 && reg_table_pid[i]; i++);

                    //Couldn't find an empty table entry
                    if(i == 20) {

                        //Tell 'em we're booked up and go back to the loop
                        postMessage(temp_msg.source, REG_STATUS, 0);
                        break;
                    }

                    //Set the entry
                    reg_table_pid[i] = temp_msg.source;
                    reg_table_svc[i] = temp_msg.payload;

                } else {

                    //Got a collision, overwrite the PID
                    reg_table_pid[i] = temp_msg.source;
                }

                //Tell 'em they've been welcomed to the family
                postMessage(temp_msg.source, REG_STATUS, 1);
            break;

            case REG_LOOKUP:

                //Search for the passed service code
                for(i = 0; (i < 20) && (reg_table_svc[i] != temp_msg.payload); i++);

		        if(i == 20) {

                    //printTable();
                }

                //Give the sender the PID, or zero if we didn't find it
                postMessage(temp_msg.source, REG_PID, i == 20 ? 0 : reg_table_pid[i]);
            break;

            case REG_DEREGISTER:
                //Search for any matches on the PID and service code
                for(i = 0; (i < 20) && (reg_table_svc[i] != temp_msg.payload) && (reg_table_pid[i] != temp_msg.source); i++);

                //If we got a match, clear it
                if(i != 20)
                    reg_table_pid[i] = reg_table_svc[i] = 0;
            break;

            default:
            break;
        }
    }
}
