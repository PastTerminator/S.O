#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h>  // fork() and exec()
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h>
#include<dirent.h>

#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 
  
// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J")

void init_shell() 
{ 
    clear(); 
    printf("\n\n\n\n******************"
        "************************"); 
    printf("\n\n\n\t****PROTO TERMINAL****"); 
    printf("\n\n\t   -João Vicente-"); 
    printf("\n\n\n\n*******************"
        "***********************"); 
    char* username = getenv("USER"); 
    printf("\n\n\nUSER is: @%s", username); 
    printf("\n"); 
    sleep(1); 
    clear(); 
} 
  

int takeInput(char* str) { 
    char* buf; 
  
    buf = readline("\n$"); 
  
    if (strlen(buf) != 0) { 
  
        add_history(buf); 
  
        strcpy(str, buf); 
        return 0; 
    } else { 
        return 1; 
    } 
} 

void printDir() 
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("\nDiretório Atual: %s", cwd); 
}

void pwd() {
	char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("%s", cwd); 
}

void parseSpace(char* str, char** parsed) // split the string
{ 
    int i; 
  
    for (i = 0; i < MAXLIST; i++) { 
        parsed[i] = strsep(&str, " "); 
  
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
}

void changeDirectory(char **parsedArgs) {

	char  *gdir;
    char  *dir;
    char  *to;
    char buf[MAXCOM];

	gdir = getcwd(buf, sizeof(buf));
    dir = strcat(gdir, "/");
    to = strcat(dir, parsedArgs[1]);

    chdir(to);
}

void ls() {

	struct dirent **namelist;
	int n;

	n = scandir(".", &namelist, NULL, alphasort);

	if(n < 0)
	{
		perror("scandir");
	} else {
		while (n--) {
			printf("%s\n",namelist[n]->d_name);
			free(namelist[n]);
		}
	free(namelist);
	}
}

void runCommand(char** parsed) 
{ 
    // Forking a child 
    pid_t pid = fork();  
  
    if (pid < 0) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (!pid) {
    	int execQ = execvp(parsed[0], parsed); 
        
        if (execQ < 0) { 
            printf("Comando inexistente"); 
        }

        exit(0); 
    } else { // pid true 
        // waiting for child to terminate (o wait força a volta para o filho, cujo pid é 0,
        //levando-o ao primeiro if, sendo assim é ) 
        wait(NULL);  
        return; 
    } 
}

void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            // parent executing, waiting for two children 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 

int parsePipe(char* str, char** strpiped) 
{ 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0; // returns zero if no pipe is found. 
    else { 
        return 1; 
    } 
}

int verifiyIfIsPipe(char *inputString, char **parsedArgs) {

	if (!(parsePipe(inputString, parsedArgs))) {
		parseSpace(inputString, parsedArgs);
		return 0; // false, no pipe
	}
	return 1;
}


int main() {
	char inputString[MAXCOM];
 	char* parsedArgs[MAXLIST];
	init_shell();

	while(1) {
		printDir();

		takeInput(inputString);
		int vp = verifiyIfIsPipe(inputString, parsedArgs);

		if (vp) {

			if (!(strcmp(parsedArgs[0], "cd"))) {
				changeDirectory(parsedArgs);
			} else if (!(strcmp(parsedArgs[0], "pwd"))) {
				pwd();
			} else if (!(strcmp(parsedArgs[0], "ls"))) {
				ls();
			} else {
				runCommand(parsedArgs);
			}
		} else {
			execArgsPiped(&inputString, parsedArgs);
		}
        continue;
	}
}
