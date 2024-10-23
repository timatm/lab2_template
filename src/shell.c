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
void redirection(int in ,int out ,struct cmd_node *p){
	printf("in: %d , out: %d\n",in,out);
	printf("p->out_file : %s\n",p->out_file);
	printf("p->in_file : %s\n",p->in_file);
	int fd;
	if (in != 0) {
          	dup2(in, 0);
          	close(in);
	} 
	else {
		if (p->in_file) {
			fd = open(p->in_file, O_RDONLY);
			dup2(fd, 0);
			close(fd);
		}
	}
	if (out != 1) {
		dup2(out, 1);
		close(out);
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
int spawn_proc(int in, int out, struct cmd *cmd, struct cmd_node *p)
{
  	pid_t pid;
  	int status;
  	if ((pid = fork()) == 0) { //child process
		redirection(in,out,p);
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
  	int in = 0, fd[2];
	struct cmd_node *temp = cmd->head;
  	while (temp->next != NULL) {
      	pipe(fd);
      	spawn_proc(in, fd[1], cmd, temp);
      	close(fd[1]);
      	in = fd[0];
      	temp = temp->next;
  	}
  	if (in != 0) {
    	spawn_proc(in, 1, cmd, temp);
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
				redirection(0,1,temp);
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
				status = spawn_proc(0, 1, cmd, cmd->head);
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
