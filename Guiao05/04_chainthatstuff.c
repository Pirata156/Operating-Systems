/**
 * Programa que emula o funcionamento do interpretador de comandos na execucao
 * encadeada de 'ls /etc | wc -l'.
 *
 * @author (Pirata)
 * @version (2018.06)
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
	pid_t pid1, pid2;
	int err, pipe_desc[2];
	/* pipe descriptors is an array with 2 ints:
	 * 		- indice 0 for output;
	 * 		- indice 1 for input; */

	if ((argc != 1) && (argv != NULL)) {
		if (write(STDERR_FILENO, "No need for extra arguments\n", 28)) {}
		_exit(EXIT_FAILURE);
	}

	err = pipe(pipe_desc);
	if (err < 0) {
		if (write(STDERR_FILENO, "This communication pipe went wrong at some point", 48)) {}
		_exit(EXIT_FAILURE);
	}

	pid1 = fork();
	if (pid1 < 0) {
		if (write(STDERR_FILENO, "Can't create new family member", 30)) {}
		_exit(EXIT_FAILURE);
	}

	if (pid1 == 0) {
		// child process 1
		/*  this child will execute the 'ls /etc' command
         *  output will be redirected to the pipe */
		close(pipe_desc[0]);
		err = dup2(pipe_desc[1], STDOUT_FILENO);
		if (err < 0) {
			// Error during dup2
			if (write(STDERR_FILENO, "Well, that happened\n", 20)) {}
			_exit(EXIT_FAILURE);
		} else {
			// dup2 succeded so can close the pipe_desc now since it's redirected to stdout
			close(pipe_desc[1]);
		}
		execlp("ls", "ls", "/etc",  NULL);	// exec will close the rest of the descriptors on exit
		_exit(EXIT_FAILURE);
	} else {
		// parent process
		/* parent will create the second son and wait for their termination
         * it will also close the pipe file descriptors since it does nothing with it */
        pid2 = fork();
        if (pid2 < 0) {
            if (write(STDERR_FILENO, "Can't create a new family member", 30)) {}
            _exit(EXIT_FAILURE);
        }

        if (pid2 == 0) {
            // child process 2
            /*  this child will execute the 'wc -l' command
             *  input will be from the pipe */
            close(pipe_desc[1]);
            err = dup2(pipe_desc[0], STDIN_FILENO);
            if (err < 0) {
                // Error during dup2
                if (write(STDERR_FILENO, "Well, that happened\n", 20)) {}
                _exit(EXIT_FAILURE);
            } else {
                // dup2 succeded so can close the pipe_desc now since it's redirected to stdin
                close(pipe_desc[0]);
            }
            execlp("wc", "wc", "-l", NULL);     // exec will close the rest of the descriptors on exit
            _exit(EXIT_FAILURE);
        } else {
            // parent process
            close(pipe_desc[0]);
            close(pipe_desc[1]);
            wait(NULL);
            wait(NULL);
            /* This is pretty much as saying to the other processes that this won't be
             * using the pipe at all. Neither writing or reading. */
        }
	}
	exit(EXIT_SUCCESS);
}
