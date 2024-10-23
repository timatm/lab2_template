#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/command.h"

/**
 * @brief Read the user's input string
 * 
 * @return char* 
 * Return string
 */
char *read_line()
{
    char *buffer = (char *)malloc(BUF_SIZE * sizeof(char));
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        exit(1);
    }

	if (fgets(buffer, BUF_SIZE, stdin) != NULL) {
		if (buffer[0] == '\n' || buffer[0] == ' ' || buffer[0] == '\t') {
			free(buffer);
			buffer = NULL;
		} 
		else {
			buffer[strcspn(buffer, "\n")] = 0;
			strncpy(history[history_count % MAX_RECORD_NUM], buffer, BUF_SIZE);
			++history_count;
		}
	}

	return buffer;
}

/**
 * @brief Parse the user's command
 * 
 * @param line User input command
 * @return struct cmd* 
 * Return the parsed cmd structure
 */
struct cmd *split_line(char *line)
{
	int args_length = 10;
    struct cmd *new_cmd = (struct cmd *)malloc(sizeof(struct cmd));
    new_cmd->head = (struct cmd_node *)malloc(sizeof(struct cmd_node));
    new_cmd->head->args = (char **)malloc(args_length * sizeof(char *));
	for (int i = 0; i < args_length; ++i)
		new_cmd->head->args[i] = NULL;
    new_cmd->head->length = 0;
    new_cmd->head->next = NULL;
    new_cmd->in_file = NULL;
    new_cmd->out_file = NULL;
	new_cmd->pipe_num = 0;

	struct cmd_node *temp = new_cmd->head;
    char *token = strtok(line, " ");
    while (token != NULL) {
        if (token[0] == '|') {
            struct cmd_node *new_pipe = (struct cmd_node *)malloc(sizeof(struct cmd_node));
			new_pipe->args = (char **)malloc(args_length * sizeof(char *));
			for (int i = 0; i < args_length; ++i)
				new_pipe->args[i] = NULL;
			new_pipe->length = 0;
			new_pipe->next = NULL;
			temp->next = new_pipe;
			temp = new_pipe;
        } else if (token[0] == '<') {
			token = strtok(NULL, " ");
            new_cmd->in_file = token;
        } else if (token[0] == '>') {
			token = strtok(NULL, " ");
            new_cmd->out_file = token;
        } else {
			temp->args[temp->length] = token;
			temp->length++;
        }
        token = strtok(NULL, " ");
		new_cmd->pipe_num++;

    }

    return new_cmd;
}
/**
 * @brief Information used to test the cmd structure
 * 
 * @param cmd Command structure
 */
void test_cmd_struct(struct cmd *cmd)
{
	struct cmd_node *temp = cmd->head;
	int pipe_count = 0;
	printf("============ COMMAND INFO ============\n");
	while (temp != NULL) {
		printf("pipe %d: ", pipe_count);
		for (int i = 0; i < temp->length; ++i) {
			printf("%s ", temp->args[i]);
		}
		printf("\n");
		temp = temp->next;
		++pipe_count;
	}
	printf(" in: %s\n", cmd->in_file ? cmd->in_file : "none");
	printf("out: %s\n", cmd->out_file ? cmd->out_file : "none");
	printf("============ COMMAND INFO END ============\n");
}

/**
 * @brief Information used to test the cmd_node structure
 * 
 * @param temp cmd_node structure
 */
void test_pipe_struct(struct cmd_node *temp){
	printf("============ PIPE INFO ============\n");
	while (temp != NULL) {
		for (int i = 0; i < temp->length; ++i) {
			printf("temp->args[%d] :%s ",i, temp->args[i]);
		}
		printf("\n");
		temp = temp->next;
	}
	printf("============ PIPE INFO END ============\n");
}
