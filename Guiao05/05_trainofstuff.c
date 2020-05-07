/**
 * Programa que emula o funcionamento do interpretador de comandos na execucao
 * encadeada de 'grep -v ^# /etc/passwd | cut -f7 -d: | uniq | wc -l'.
 *
 * @author (Pirata)
 * @version (2018.06)
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define COMMAND_LINE {{"grep", "-v", "^#", "/etc/passwd", NULL},{"cut", "-f7", "-d:", NULL},{"uniq",NULL},{"wc","-l", NULL}}
#define COMMAND_SIZE 4
#define COMMAND_MAXW 5
#define PREVIOUS (i % 2)
#define CURRENT ((i + 1) % 2)

int main (int argc, char** argv)
{
    /* Just an easy way to store the command line already split */
    char *cmd[COMMAND_SIZE][COMMAND_MAXW] = COMMAND_LINE;

    pid_t pid;
    int err, pipe_desc[2][2], i;
    /* pipe descriptors is an array with 2 ints:
     *              - indice 0 for output;
     *              - indice 1 for input;   */
    
    if ((argc != 1) && (argv != NULL)) {
        if (write(STDERR_FILENO, "No need for extra arguments\n", 28)) {}
        _exit(EXIT_FAILURE);
    }

    i = 0;
    while ((i < 4) && (pid > 0)) {
        err = pipe(pipe_desc[CURRENT]);

        if (err < 0) {
            if (write(STDERR_FILENO, "One of the communication pipes went rogue at some point\n", 56)) {}
            _exit(EXIT_FAILURE);
        }

        pid = fork();

        if (pid < 0) {
            if (write(STDERR_FILENO, "Can't create a new family member", 30)) {}
            _exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Child process - 'close' and 'dup2' made here won't affect the other 'fork's
            if (i != 0) {
                /* Start of the chained execution
                 * Reads from STDIN so won't need redirect of input before execution
                 * All the other iterations need it */
                close(pipe_desc[PREVIOUS][1]);                      // Closes previous pipeline output - We won't write to the pipe
                err = dup2(pipe_desc[PREVIOUS][0],STDIN_FILENO);    // Redirects input from the previous pipe - We read from the pipe
                close(pipe_desc[PREVIOUS][0]);                      // Closes previous pipeline input after redirecting

                if (err < 0) {
                    if (write(STDERR_FILENO, "Redirection Kaboom!\n", 20)) {}
                    _exit(EXIT_FAILURE);
                }
            }
            
            /* Executions output are redirect to current pipeline
             * Except on the last element of the chained execution
             */
            close(pipe_desc[CURRENT][0]);                           // Close current pipeline input - We won't read from the pipe
            if (i != (COMMAND_SIZE - 1)) {
                // Last element -> i == COMMAND_SIZE - 1
                err = dup2(pipe_desc[CURRENT][1], STDOUT_FILENO);   // Redirects output to the current pipeline - We write to the pipe

                if (err < 0) {
                    if (write(STDERR_FILENO, "Redirection Kaboom!\n", 20)) {}
                    _exit(EXIT_FAILURE);
                }
            }
            close(pipe_desc[CURRENT][1]);                           // Close current pipeline output - Either after redirect or cause last element
            
            execvp(cmd[i][0], cmd[i]);      // Executes command
            _exit(EXIT_FAILURE);            // Only runs in case of error running execvp
        } else {
            // Parent process
            if (i > 0) {
                /* Closes the previous pipeline (so execution ends with EOF signal)
                 * BUT keeps the current pipeline 'open' so in can be replicated
                 * on the next iteration as 'PREVIOUS' pipeline
                 */
                close(pipe_desc[PREVIOUS][0]);      // Close input
                close(pipe_desc[PREVIOUS][1]);      // Close output
            }
            // Needed to control parent won't overrun child processes - May cause pipelines to modify midway
            waitpid(pid,NULL,0);
            i++;
        }
    }
    _exit(EXIT_SUCCESS);
}