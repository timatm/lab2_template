#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/command.h"
#include "../include/builtin.h"

// ======================= requirement 2.3 =======================
/**
 * @brief 
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" in the cmd structure
 * If you want to implement ( | ), use "in" and "out" in the arguments.
 * @param in 
 * The file descriptor where stdin should be redirected
 * Used by pipe so that two processes can communicate
 * If pipe is not used, in is "0" ( stdin ( fd[0]) )
 * @param out
 * The file descriptor where stdout should be redirected
 * Used by pipe so that two processes can communicate
 * If pipe is not used, out is "1" ( stdout ( fd[1]) )
 * @param cmd Command structure
 */
void redirection(struct cmd_node *p){
	int fd;
	if (p->in != 0) {
          	dup2(p->in, 0);
          	close(p->in);
	} 
	else {
		if (p->in_file) {
			fd = open(p->in_file, O_RDONLY);
			dup2(fd, 0);
			close(fd);
		}
	}
	if (p->out != 1) {
		dup2(p->out, 1);
		close(p->out);
	} 
	else {
		if (p->out_file) {
			fd = open(p->out_file, O_RDWR | O_CREAT, 0644);
			dup2(fd, 1);
			close(fd);
		}
	}
}
// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief Execute external command
 * 
 * @note 
 * The external command is mainly divided into the following two steps:
 * 1. Create child process
 * 2. Call execute to " execute() " the corresponding executable file
 * @param in
 * The file descriptor where stdin should be redirected
 * Used by pipe so that two processes can communicate
 * If pipe is not used, in is stdin(fd 0)
 * @param out 
 * The file descriptor where stdout should be redirected
 * Used by pipe so that two processes can communicate
 * If pipe is not used, out is stdout(fd 1)
 * @param cmd Command structure
 * @param p cmd_node structure
 * @return int 
 * Return execution status
 */
int spawn_proc(struct cmd_node *p)
{
  	pid_t pid;
  	int status;
  	if ((pid = fork()) == 0) { //child process
		redirection(p);
		status = execvp(p->args[0], p->args);
    	if (status == -1){
			perror("lsh");
		}
    	exit(EXIT_FAILURE);
    } 
	else { //parent process
		waitpid(pid, &status, WUNTRACED);
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
  	return 1;
}
// ===============================================================


// ======================= requirement 2.4 =======================
/**
 * @brief Call "spawn_proc()" in order according to the number of cmd_node
 * 
 * @param cmd Command structure  
 * @return int
 * Return execution status 
 */
int fork_cmd_node(struct cmd *cmd)
{
  	int fd[2];
	struct cmd_node *temp = cmd->head;
  	while (temp->next != NULL) {
      	pipe(fd);
		temp->out = fd[1];
      	spawn_proc(temp);
      	close(fd[1]);
      	temp->next->in = fd[0];
		test_pipe_struct(temp);
      	temp = temp->next;
  	}
  	if (temp->in != 0) {
		temp->out = 1;
		test_pipe_struct(temp);
    	spawn_proc(temp);
    	return 1;
  	}
	return 1;
}
// ===============================================================


void shell()
{
	while (1) {
		printf(">>> $ ");
		char *buffer = read_line();
		if (buffer == NULL)
			continue;

		struct cmd *cmd = split_line(buffer);
		
		int status = -1;
		// only a single command
		struct cmd_node *temp = cmd->head;
		
		if(temp->next == NULL){
			status = searchBuiltInCommand(temp);
			if (status != -1){
				int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);
				if( in == -1 | out == -1)
					perror("dup");
				redirection(temp);
				status = execBuiltInCommand(status,temp);

				// recover shell stdin and stdout
				if (temp->in_file)  dup2(in, 0);
				if (temp->out_file){
					dup2(out, 1);
				}
				close(in);
				close(out);
			}
			else{
				//external command
				status = spawn_proc(cmd->head);
			}
		}
		// There are multiple commands ( | )
		else{
			
			status = fork_cmd_node(cmd);
		}
		// free space
		while (cmd->head) {
			
			struct cmd_node *temp = cmd->head;
      		cmd->head = cmd->head->next;
			free(temp->args);
   	    	free(temp);
   		}
		free(cmd);
		free(buffer);
		
		if (status == 0)
			break;
	}
}
