#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <sys/stat.h>
#include <dirent.h>

// Funcao para separar melhor a logica do request
void operacoes_cliente(int client_socket);

char current_dir[1024] = ".";

int main(int argc, char *argv[])
{
    // Declaracao dos sockets
    int listenfd = 0, connfd = 0, processo;
    // Declaracao endereco do servidor
    struct sockaddr_in serv_addr; 

    // Criação e controle de erro pro socket do servidor
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1) {
	    printf("Erro no socket\n");
	    exit(1);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    // Criação do endereco (IP + porta)
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(2121); 

    // Bind do servidor na porta
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
	   printf("Nao deu para fazer o bind\n");
	    exit(1);
    } 

    // Servidor comeca a ouvir no socket
    listen(listenfd, 10); 

    // Mensagem indicando que servidor esta no ar
    printf("Servidor FTP inicializado. Porta: 2121\n");

    // Loop para o servidor não parar
    while(1)
    {
        // Conexão no socket
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        // printf("SERVIDOR: Pai-accepted\n");
        processo = fork();	
        if (!processo) {
            // Handle do request
            // printf("SERVIDOR: Filho-fork\n");
            close(listenfd);
            operacoes_cliente(connfd);
            exit(1);
        }
        close(connfd);
    }
}

void operacoes_cliente(int client_socket)
{
    // Declaracao Buffers
    char receiveBuffer[1024], sendBuffer[1024];
    // Declaracao variavel de controle
    int n;

    while((n = recv(client_socket, receiveBuffer, sizeof(receiveBuffer), 0)) > 0)
    {
        // Character terminator para ler a string no if
        receiveBuffer[n] = '\0';

        // Se request for UPLOAD
        if(strncmp(receiveBuffer, "UPLOAD", 6) == 0)
        {
            // Pega o nome do arquivo
            char filename[256];
            sscanf(receiveBuffer + 7, "%s", filename);
            char path[1024];
            strcpy(path, current_dir);
            strcat(path, "/");
            strcat(path, filename);

            // Abre o arquivo com o novo nome para escrita
            FILE *file = fopen(path, "wb");
            // Tratamento de erro
            if(file == NULL)
            {
                perror("Erro ao abrir o arquivo para upload");
                close(client_socket);
                return;
            }

            // Informar ao cliente que o servidor esta pronto para receber dados
            send(client_socket, "READY", 5, 0);

            // Enquanto estiver recebendo pacotes no socket
            while((n = recv(client_socket, receiveBuffer, 1024, 0)) > 0)
            {
                // Caso seja final de arquivo: break
                if(strncmp(receiveBuffer, "EOF", 3) == 0)
                {
                    break;
                }
                // Se nao, escreva no arquivo
                fwrite(receiveBuffer, sizeof(char), n, file);
                send(client_socket, "READY", 5, 0);
            }

            fclose(file);

            send(client_socket, "Upload concluido com sucesso.", 29, 0);
        }
        // Se request for DOWNLOAD
        else if (strncmp(receiveBuffer, "DOWNLOAD", 8) == 0)
        {
            // Pega o nome do arquivo enviado pelo cliente
            char filename[256];
            sscanf(receiveBuffer + 9, "%s", filename);

            char path[1024];
            strcpy(path, current_dir);
            strcat(path, "/");
            strcat(path, filename);

            // Tenta abrir o arquivo e trata em caso de erro
            FILE *file = fopen(path, "rb");
            if(file == NULL)
            {
                send(client_socket, "Arquivo nao encontrado!\n", 24, 0);
                continue;
            }

            // Cria um objeto stat e tenta escrever nele as estatisticas de filename
            struct stat file_stat;
            if (stat(path, &file_stat) < 0)
            {
                perror("Erro ao obter o tamanho do arquivo");
                fclose(file);
                close(client_socket);
                return;
            }

            // Envia o tamanho do arquivo para o cliente
            long file_size = file_stat.st_size;
            sprintf(sendBuffer, "%ld", file_size);
            send(client_socket, sendBuffer, strlen(sendBuffer), 0);

            // Espera a resposta de ready
            n = recv(client_socket, receiveBuffer, 1024, 0);
            receiveBuffer[n] = '\0';

            // Envia os dados do arquivo
            if(strcmp(receiveBuffer, "READY") == 0)
            {
                while ((n = fread(sendBuffer, sizeof(char), 1024, file)) > 0)
                {
                    send(client_socket, sendBuffer, n, 0);
                }
            }
            
            // Fecha arquivo
            fclose(file);
        }
        else if (strncmp(receiveBuffer, "LIST", 4) == 0)
        {
            // Tenta abrir o diretorio local
            DIR *dir = opendir(current_dir);
            if(dir == NULL)
            {
                perror("Erro ao abrir o diretorio!");
                send(client_socket, "Erro ao abrir diretorio", 23, 0);
                close(client_socket);
                return;
            }

            
            // Cria uma struct entrada para imprimir o nome de cada arquivo e pasta
            struct dirent *entry;
            
            memset(receiveBuffer, '\0', sizeof(receiveBuffer));
            while((entry = readdir(dir)) != NULL)
            {
                if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                {
                    strcat(receiveBuffer, entry->d_name);
                    strcat(receiveBuffer, " ");
                }
            }
            send(client_socket, receiveBuffer, strlen(receiveBuffer), 0);
			
            // Fecha arquivo e envia o termino
            closedir(dir);
        }
        else if (strncmp(receiveBuffer, "RMDIR", 5) == 0) 
        {
            char dirname[256];
            sscanf(receiveBuffer + 6, "%s", dirname);

            char path[1024];
            strcpy(path, current_dir);
            strcat(path, "/");
            strcat(path, dirname);

            if (rmdir(path) == 0) {
                send(client_socket, "Pasta removida com sucesso.\n", 29, 0);

                strcpy(current_dir, ".");
            } else {
                send(client_socket, "Erro ao remover pasta.\n", 24, 0);
            }
        }
        else if (strncmp(receiveBuffer, "MKDIR", 5) == 0)
        {
            char dirname[256];
            sscanf(receiveBuffer + 6, "%s", dirname);

            char path[1024];
            strcpy(path, current_dir);
            strcat(path, "/");
            strcat(path, dirname);

            if (mkdir(path, 0755) == 0) { // 755 é o chmod de permissão geral para dono e execução e leitura para o resto
                send(client_socket, "Pasta criada com sucesso.\n", 27, 0);
            } else {
                send(client_socket, "Erro ao criar pasta.\n", 22, 0);
            }
        }
        else if (strncmp(receiveBuffer, "REMOVE", 6) == 0)
        {
            char filename[256];
            sscanf(receiveBuffer + 7, "%s", filename);

            char path[1024];
            strcpy(path, current_dir);
            strcat(path, "/");
            strcat(path, filename);

            if (remove(path) == 0) {
                send(client_socket, "Arquivo removido com sucesso.\n", 31, 0);
            } else {
                send(client_socket, "Erro ao remover arquivo.\n", 26, 0);
            }
        }
        else if (strncmp(receiveBuffer, "RENAME", 6) == 0)
        {
            char oldName[256];
            char newName[256];

            sscanf(receiveBuffer + 7, "%s", oldName);
            sscanf(receiveBuffer + 8 + strlen(oldName), "%s", newName);

            char pathOldName[1024];
            strcpy(pathOldName, current_dir);
            strcat(pathOldName, "/");
            strcat(pathOldName, oldName);

            char pathNewName[1024];
            strcpy(pathNewName, current_dir);
            strcat(pathNewName, "/");
            strcat(pathNewName, newName);

            if (rename(pathOldName, pathNewName) == 0) {
                send(client_socket, "Renomeado com sucesso.\n", 24, 0);
            } else {
                send(client_socket, "Erro ao renomear.\n", 19, 0);
            }
        }
        else if (strncmp(receiveBuffer, "CD", 2) == 0)
        {
            char dirname[256];
            sscanf(receiveBuffer + 3, "%s", dirname);

            char tmp_current_dir[1024];
            strcpy(tmp_current_dir, current_dir);
            
            strcat(tmp_current_dir, "/");
            strcat(tmp_current_dir, dirname);

            DIR * dir = opendir(tmp_current_dir);
            if(dir == NULL)
            {
                send(client_socket, "Erro ao abrir o diretorio!\n", 28, 0);
            }
            else
            {
                strcat(current_dir, "/");
                strcat(current_dir, dirname);
                send(client_socket, "Diretorio do servidor modificado!\n", 35, 0);
            }
        }

        // Limpa buffers
        memset(receiveBuffer, '\0', 1024); 
        memset(sendBuffer, '\0', 1024);
    }

    close(client_socket);
}
