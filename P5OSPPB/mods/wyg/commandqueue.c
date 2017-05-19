#include "commandqueue.h"
#include "../include/memory.h"

CommandQueue* CommandQueue_new() {

    CommandQueue* out_queue;

    if(!(out_queue = (CommandQueue*)malloc(sizeof(CommandQueue))))
        return (CommandQueue*)0;

    out_queue->length = 0;
    out_queue->root_command = out_queue->tail_command = (Command*)0;

    return out_queue;
}

void CommandQueue_insert(CommandQueue* queue, Command* command) {

    if(!queue)
        return;

    command->next = (Command*)0;

    if(!queue->tail_command) {
        
        queue->root_command = queue->tail_command = command;
    } else {

        queue->tail_command->next = command;
        queue->tail_command = command;
    }

    queue->length++;
}

Command* CommandQueue_pull(CommandQueue* queue) {

    Command* out_command;

    if(!queue || !queue->length || !queue->root_command || !queue->tail_command)
        return (Command*)0;

    out_command = queue->root_command;
    queue->root_command = queue->root_command->next;
    queue->length--;

    if(!queue->root_command)
        queue->tail_command = queue->root_command;

    return out_command;
}

Command* Command_new(unsigned int command, unsigned int param_count) {

    Command* out_command;

    if(!(out_command = (Command*)malloc(sizeof(Command))))
        return (Command*)0;

    out_command->command = command;
    out_command->param_count = param_count;
    out_command->next = (Command*)0;

    if(out_command->param_count == 0) {

        out_command->param = (unsigned int*)0;
    } else if(!(out_command->param = (unsigned int*)malloc(sizeof(unsigned int) * out_command->param_count))) {

        free(out_command);
        return (Command*)0;
    }

    return out_command;
}

void Command_delete(Command* command) {

    if(!command)
        return;

    if(command->param)
        free(command->param);

    free(command);
}

void CommandQueue_delete(CommandQueue* queue) {

    Command* command;

    if(!queue)
        return;

    while(command = CommandQueue_pull(queue))
        free(command);

    free(queue);
}
