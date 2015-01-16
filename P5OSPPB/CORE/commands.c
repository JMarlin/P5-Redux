#include "commands.h"
#include "../ASCII_IO/ascii_o.h"
#include "../obj/lists.h"
#include "../obj/variant.h"

void testvars() {

    varList* vars;
    variant* tmpVar;
    listItem* tmpItem;

    prints("Testing variant system:\n");
        
    prints("Allocating a new variant list.\n");
    vars = newVarList();
    prints("Allocating variable X, ");
    allocVar(vars, "X");
    prints("two, ");
    allocVar(vars, "two");
    prints("and var. ");
    allocVar(vars, "var");
    prints("Done.\n");
    
    //Set up x var
    prints("Setting up X var:\n   -Retrieving by name...");
    tmpVar = getVar(vars, "X");
    prints("Done.\n   -Setting variant mode to 'string'...");
    tmpVar->mode = VAR_MODE_STRING;
    prints("Done.\n   -Assigning value to variant...");
    variantAssignString(tmpVar, "Hello, world!");
    prints("Done.\n");

    //Set up two var
    prints("Setting up two var:\n   -Retrieving by name...");    
    tmpVar = getVar(vars, "two");
    prints("Done.\n   -Setting variant mode to 'number'...");
    tmpVar->mode = VAR_MODE_NUMBER;
    prints("Done.\n   -Assigning double value to variant...");
    variantAssignNum(tmpVar, 123456.78910);
    prints("Done.\n   -Casting assigned value...");
    variantInternalCast(tmpVar);
    tmpVar->mode = VAR_MODE_STRING;
    prints("Done.\n");

    //Set up var var
    prints("Setting up var var:\n   -Retrieving by name...");
    tmpVar = getVar(vars, "var");
    prints("Done.\n   -Setting variant mode to 'string'...");
    tmpVar->mode = VAR_MODE_STRING;
    prints("Done.\n   -Assigning string value to variant...");
    variantAssignString(tmpVar, "123456.78910");
    prints("Done.\n   -Casting assigned value...");
    variantInternalCast(tmpVar);            
    tmpVar->mode = VAR_MODE_NUMBER;
    prints("Done.\n");

    prints("Rewinding list...");
    listRewind((list*)vars);
    prints("Done.\nList root:");
    printHexDword((int)((list*)vars)->rootItem);
    prints("\nPrinting current vars:\n");
    while(tmpItem = listNext((list*)vars)) {
        tmpVar = (variant*)tmpItem->data;
        prints(tmpVar->name);
        prints(": ");
        variantInternalCast(tmpVar);
        printHexDword((int)tmpVar->strVal);
        prints("\n");
    }

}

void clear(void)
{
  int i;
  setCursor(0, 0);
  for(i = 0; i < 2000; i++)
        pchar(0);
  setCursor(0, 0);
}

void chprompt(void)
{
   prints("Sorry, not yet implemented.");        
}

void help(void)
{
   clear();
   
   prints("Protical5 PinkCI command interpreter               ");pchar(10);
   prints("version R0                                         ");pchar(10);
   prints("Written by Joseph Marlin (stithyoshi@sbcglobal.net)");pchar(10);
   prints("---------------------------------------------------");pchar(10);
   prints("   Commands are:                                   ");pchar(10);
   prints("                                                   ");pchar(10);
   prints("        CLR..................Clear the screen.     ");pchar(10);
   prints("        CHPROMPT.............Not yet implemented.  ");pchar(10);
   prints("        HELP.................Print this message.   ");pchar(10);
   prints("                                                   ");pchar(10);
           
}
