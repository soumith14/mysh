#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void sigchld_handler(int sig) {
   (void)sig;
   while (waitpid(-1, NULL, WNOHANG) > 0) { }
}

pid_t shell_pgid;


#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_JOBS 64

typedef struct {
	pid_t pid;
	char cmdline[MAX_LINE];
	int active;
} job_t;
job_t jobs[MAX_JOBS];

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];


    shell_pgid = getpid(); 
    setpgid(shell_pgid, shell_pgid);
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, sigchld_handler);

    for (int i=0; i<MAX_JOBS; i++) {
         jobs[i].active = 0;
    }

    while (1) {

	printf("mysh> ");
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) {
	    break;
        }

	line[strcspn(line, "\n")] = 0;
	
	char raw_line[MAX_LINE];
	strncpy(raw_line, line, MAX_LINE);

	if (strlen(line) == 0) continue;

	
	int argc = 0;
	char *token = strtok(line, " ");
	while (token && argc < MAX_ARGS-1) {
	     args[argc++] = token;
	     token = strtok(NULL, " ");
	}
	args[argc] = NULL;
	
	int background = 0;
	if (argc > 0 && strcmp(args[argc-1], "&") == 0) {
	    background = 1;
	    args[--argc] = NULL;
        }

	int redirect_in = 0, redirect_out = 0;
	char *infile = NULL, *outfile = NULL;

	for (int i=0; i<argc; i++) {
	     if ( args[i] ==NULL)
		 continue;

	     if (strcmp(args[i], "<") == 0 && args[i+1] != NULL) {
		 redirect_in = 1;
		 infile      = args[i+1];
		 args[i]     = NULL;
		 args[i+1]   = NULL;
		 i++;
	     }
	     else if (strcmp(args[i], ">") == 0 && args[i+1] != NULL) {
	         redirect_out = 1;
		 outfile      = args[i+1] ;
		 args[i]      = NULL;
		 args[i+1]    = NULL;
		 i++;
	     }
	}

	if(strcmp(args[0], "exit")==0) {
	   exit(0);
	}

  	if (strcmp(args[0], "cd")==0) {
	    if (args[1]) {
		if (chdir(args[1])!=0) perror("cd failed");
	    } else {
		printf("cd: missing path\n");
	     }
	     continue;
  	}
	if (strcmp(args[0], "jobs") == 0) {
	    for (int i=0; i< MAX_JOBS; i++) {
	         if (jobs[i].active) {
		     printf("[%d] %d %s\n", i, jobs[i].pid, jobs[i].cmdline);
	         }
	    }
	    continue;
	}

	pid_t pid = fork();
	if (pid == 0) {
	    setpgid(0,0);
	    fprintf(stderr,
		"DBG: in=%d out=%d infile=%s outfile=%s\n",
                redirect_in,
	        redirect_out,
	        infile ? infile : "(null)",
		outfile ? outfile : "(null)"
	    );	

	    if (redirect_in) {
		int fd = open(infile, O_RDONLY);
	        if(fd<0) { perror("open infile"); exit(1); }
		dup2(fd, STDIN_FILENO);
		close(fd);
	    }
	    if (redirect_out) {
		int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd<0) { perror("open outfile"); exit(1); }
		dup2(fd, STDOUT_FILENO);
		close(fd);
	    }


	    execvp(args[0], args);
	    perror("exec failed");
	    exit(1);
	} else {
	    setpgid(pid, pid);
	    if (background) {
	        for (int i=0; i<MAX_JOBS; i++) {
		     if (!jobs[i].active) {
			 jobs[i].pid = pid;
			 strncpy(jobs[i].cmdline, raw_line, MAX_LINE);
			 jobs[i].active = 1;
			 printf("[bg %d] %d\n", i, pid);
			 break;
		     }
		}
	  } else {
		
		waitpid(pid, NULL, 0);
	   }
	}
}     
     printf("Goodbye!\n");
     return 0;
}


