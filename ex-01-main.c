#include <stdio.h>
#include <unistd.h>  // para fork() e sleep()
#include <sys/types.h> // para pid_t
#include <stdlib.h>  // para exit()

int main() {
	pid_t pid = fork();  // cria o processo filho

	if (pid < 0) {  // Se fork() retornar um valor negativo, houve erro ao criar o processo
    	printf("Erro ao criar processo filho.\n");
    	return 1;
	} else if (pid == 0) {  // Código do processo filho
    	printf("Processo filho: Meu PID é %d.\n", getpid());
    	for (int i = 1; i <= 5; i++) {
        	printf("Processo filho: Mensagem %d\n", i);
        	sleep(1);  // Espera 1 segundo
    	}
    	exit(0);  // Termina o processo filho
	} else {  // Código do processo pai
    	printf("Processo pai: Meu PID é %d.\n", getpid());
    	printf("Processo pai: O PID do meu filho é %d.\n", pid);
    	wait(NULL);  // Aguarda o término do processo filho
	}

	return 0;
}
