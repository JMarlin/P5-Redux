#include "commands.h"
#include "../ASCII_IO/ascii_o.h"
#include "../obj/lists.h"
#include "../obj/variant.h"

void testvars() {

    varList* vars;
    variant* tmpVar;
    listItem* tmpItem;

    prints("Testing variant system:\n");
        
    vars = newVarList();
    allocVar(vars, "X");
    allocVar(vars, "two");
    allocVar(vars, "var");
    
    //Set up x var
    tmpVar = getVar(vars, "X");
    tmpVar->mode = VAR_MODE_STRING;
    variantAssignString(tmpVar, "Hello, world!");

    //Set up two var
    tmpVar = getVar(vars, "two");
    tmpVar->mode = VAR_MODE_STRING;
    variantAssignNum(tmpVar, 123456.78910);
    variantInternalCast(tmpVar);

    //Set up var var
    tmpVar = getVar(vars, "var");
    tmpVar->mode = VAR_MODE_NUMBER;
    variantAssignString(tmpVar, "123456.78910");
    variantInternalCast(tmpVar);            

    listRewind((list*)vars);
    while(tmpItem = (listItem*)listNext((list*)vars)) {
        tmpVar = (variant*)tmpItem->data;
        prints(tmpVar->name);
        prints(": ");
        variantInternalCast(tmpVar);
        prints(tmpVar->strVal);
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
