#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

typedef struct Command_struct {
    unsigned int command;
    unsigned int param_count;
    unsigned int* param;
    struct Command_struct* next;
} Command;

typedef struct CommandQueue_struct {
    Command* root_command;
    Command* tail_command;
    unsigned int length;
} CommandQueue;

CommandQueue* CommandQueue_new();
void CommandQueue_insert(CommandQueue* queue, Command* command);
Command* CommandQueue_pull(CommandQueue* queue);
Command* Command_new(unsigned int command, unsigned int param_count);
void Command_delete(Command* command);
void CommandQueue_delete(CommandQueue* queue);

#endif //COMMANDQUEUE_H
