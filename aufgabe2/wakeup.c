#include <stdio.h>
#include <unistd.h>
#include <string.h>



int main(void){
	const char *WaitForMe ="/home/paddex/aufgabe2/waitforme"; //Programm welches aufgerufen wird

	pid_t forkResult;
	int stdinPipe[2];
	int stdoutPipe[2];
	char readBuffer[1024];
	int readPos = 0;
	int readResult;	
	char *message = "5";
	char massage;
	
	

	//message = "4";
	massage=getchar();
	printf("Die eingabe war %c\n",  &massage);
	//scanf("%s", massage);
	//message = &massage;
	//scanf("%s", massage);
	printf("Die eingabe war %c und %s \n", message);
	
	const size_t messageLen = strlen(message);
	//Pipe erzeugen
	if(pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
		perror("pipe");
		return 3;
	}
	
	

	//Kindprozess erstellen
	forkResult = fork();
	if(forkResult < 0){
		perror("fork");
		return 1;
	}
	else if(forkResult == 0){

		//Standardeingabe umleiten
		if(dup2(stdinPipe[0], STDIN_FILENO) == -1){
			perror("dub2(stdinPipe[0], STDIN_FILENO)");
			return 4;
		}
		//Standardausgabe umleiten
		if(dup2(stdoutPipe[1], STDOUT_FILENO) == -1){
			perror("stdoutPipe[1], STDOUT_FILENO");
			return 5;
		}

		//unnötigen deskriptoren schließen
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		close(stdoutPipe[0]);
		close(stdoutPipe[1]);

		execl(WaitForMe, WaitForMe, NULL); //neues Programm wird gestartet
		perror("exec");
		return 2;
	}
	else {
		//unnötige deskriptoren des Elternprozesses schließen
		close(stdinPipe[0]);
		close(stdoutPipe[1]);

		//Daten von stdinPipe an Kindprozess senden
		if(write(stdinPipe[1], message, messageLen) < messageLen){
			perror("write");
			return 6;
		}
		
		//Filedeskriptor schließen da wakeup erst am ende beginnt
		close(stdinPipe[1]);
		
		//Daten aus der stdoutPipe des Kindprozesses lesen
		while(readPos < sizeof(readBuffer)-1 &&(readResult = read(stdoutPipe[0], readBuffer+readPos, sizeof(readBuffer)-1-readPos)) > 0){
			readPos+= readResult;
		}

		if(readResult < 0){
			perror("read");
			return 7;
		}

		//Lesepuffer mit nullen beschreiben
		readBuffer[readPos] = '\0';

		//Ausgabe des Ergebisses
		printf("Ausgabe des Kindprozesses: %s\n", readBuffer);
		return 0;

		
	}
}
