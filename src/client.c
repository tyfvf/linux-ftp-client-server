#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void download(int socket, const char* filename);
void upload(int socket, const char* filename);
void re_move_server(int socket, const char* filename);
void re_move_client(const char* filename);
void re_name_server(int socket, const char* oldName, const char* newName);
void re_name_client(const char* oldName, const char* newName);
void rm_dir_server(int socket, const char* dirname);
void rm_dir_client(const char* dirname);
void mk_dir_server(int socket, const char* dirname);
void mk_dir_client(const char* dirname);
void ls_server(int socket);
void ls_client();
void cd_server(int socket, const char* dirname);
void cd_client(const char* dirname);

char current_dir[1024] = ".";

int main(int argc, char *argv[])
{
    // Declaracao de socket e endereco do socket do servidor
    int sockfd;
    struct sockaddr_in serv_addr; 

    // Declaracao de buffers
    char sendBuffer[1024];

    // Criacao do socket e tratamento de erro
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("\nCLIENTE: Socket creation failed...\n");
        exit(0);
    }
    else
    {
        printf("\nCLIENTE: Socket successfully created..\n");
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuffer, '0', sizeof(sendBuffer)); 

    // Definicao do IP e Porta do socket do servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(2121); 

    // Conectando ao servidor e tratamento de erro
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        printf("\nCLIENTE: connection with the server failed...\n");
        exit(0);
    }
    else
    {
        printf("\nCLIENTE: connected to the server..\n");
    }

    // Loop para digitar os requests FTP
    while(1)
    {
        // Menu
        printf("----------------------------------------------------------\n");
        printf("Comandos disponiveis (UPPER = server | lower = client):\n");
        printf("DOWNLOAD <arquivo>\n");
        printf("UPLOAD <arquivo>\n");
        printf("REMOVE <arquivo>\n");
        printf("RENAME <nome velho> <nome novo>\n");
        printf("RMDIR <diretorio>\n");
        printf("MKDIR <diretorio>\n");
        printf("CD <diretorio>\n");
        printf("LS\n");
        printf("EXIT\n");
        printf("Seu comando: ");
        fgets(sendBuffer, 1024, stdin);

        if(strncmp(sendBuffer, "DOWNLOAD", 8) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com download
            char filename[256];
            sscanf(sendBuffer + 9, "%s", filename);
            download(sockfd, filename);
        }
        else if(strncmp(sendBuffer, "UPLOAD", 6) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com upload
            char filename[256];
            sscanf(sendBuffer + 7, "%s", filename);
            upload(sockfd, filename);
        }
        else if(strncmp(sendBuffer, "REMOVE", 6) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com remocao de arquivo
            char filename[256];
            sscanf(sendBuffer + 7, "%s", filename);
            re_move_server(sockfd, filename);
        }
        else if(strncmp(sendBuffer, "remove", 6) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com remocao de arquivo
            char filename[256];
            sscanf(sendBuffer + 7, "%s", filename);
            re_move_client(filename);
        }
        else if(strncmp(sendBuffer, "RENAME", 6) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com renomeacao
            char oldName[256];
            char newName[256];
            sscanf(sendBuffer + 7, "%s", oldName);
            sscanf(sendBuffer + 8 + strlen(oldName), "%s", newName);
            re_name_server(sockfd, oldName, newName);
        }
        else if(strncmp(sendBuffer, "rename", 6) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com renomeacao
            char oldName[256];
            char newName[256];
            sscanf(sendBuffer + 7, "%s", oldName);
            sscanf(sendBuffer + 8 + strlen(oldName), "%s", newName);
            re_name_client(oldName, newName);
        }
        else if(strncmp(sendBuffer, "RMDIR", 5) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com remocao de diretorio
            char dirname[256];
            sscanf(sendBuffer + 6, "%s", dirname);
            rm_dir_server(sockfd, dirname);
        }
        else if(strncmp(sendBuffer, "rmdir", 5) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com remocao de diretorio
            char dirname[256];
            sscanf(sendBuffer + 6, "%s", dirname);
            rm_dir_client(dirname);
        }
        else if(strncmp(sendBuffer, "MKDIR", 5) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com adicao de diretorio
            char dirname[256];
            sscanf(sendBuffer + 6, "%s", dirname);
            mk_dir_server(sockfd, dirname);
        }
        else if(strncmp(sendBuffer, "mkdir", 5) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com adicao de diretorio
            char dirname[256];
            sscanf(sendBuffer + 6, "%s", dirname);
            mk_dir_client(dirname);
        }
        else if(strncmp(sendBuffer, "CD", 2) == 0)
        {
            char dirname[256];
            sscanf(sendBuffer + 3, "%s", dirname);

            cd_server(sockfd, dirname);        
        }
        else if(strncmp(sendBuffer, "cd", 2) == 0)
        {
            char dirname[256];
            sscanf(sendBuffer + 3, "%s", dirname);

            cd_client(dirname);        
        }
        else if(strncmp(sendBuffer, "LS", 2) == 0)
        {
            // Pega o nome do arquivo e chama a funcao que lida com listagem
            ls_server(sockfd);
        }
        else if(strncmp(sendBuffer, "ls", 2) == 0)
        {    
            ls_client();
        }
        else if(strncmp(sendBuffer, "EXIT", 4) == 0)
        {
            break;
        }
        else
        {
            printf("Comando invalido.\n");
        }
    }

    // Fecha o socket e acaba o processo
    close(sockfd);
    return 0;
}

void upload(int socket, const char* filename)
{
    // Cria arquivo e abra o nome do arquivo em modo leitura
    char path[1024];
    strcpy(path, current_dir);
    strcat(path, "/");
    strcat(path, filename);

    FILE *file = fopen(path, "rb");
    // Tratamento de erro
    if(file == NULL)
    {
        perror("Erro ao abrir arquivo para upload");
        return;
    }

    // Criacao de um buffer
    char buffer[1024];
    // Enviando para o servidor qual o arquivo que sera enviado
    sprintf(buffer, "UPLOAD %s", filename);
    send(socket, buffer, strlen(buffer), 0);

    // Espera a resposta do servidor indicando que ele esta pronto
    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';
    if(strcmp(buffer, "READY") != 0)
    {
        printf("Erro: Servidor nao esta pronto para receber o upload.\n");
        fclose(file);
        return;
    }

    // Envia os dados do arquivo
    while ((n = fread(buffer, sizeof(char), 1024, file)) > 0)
    {
        send(socket, buffer, n, 0);
        recv(socket, buffer, 1024, 0);
    }

    // Envia que eh o final do arquivo
    send(socket, "EOF", 3, 0);
    // Fecha arquivo local
    fclose(file);

    // Recebe confirmação do servidor
    n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';

    printf("%s\n", buffer);

    memset(buffer, '\0', sizeof(buffer));
}

void download(int socket, const char* filename)
{
    // Declaracao buffer
    char buffer[1024];

    // Envia para o socket o arquivo a ser baixado
    sprintf(buffer, "DOWNLOAD %s", filename);
    send(socket, buffer, strlen(buffer), 0);

    // Espera uma confirmacao para comecar
    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';

    long file_size = atol(buffer);
    if(file_size <= 0)
    {
        printf("Erro: Tamanho do arquivo inválido ou arquivo não encontrado no servidor.\n");
        return;
    }

    send(socket, "READY", 5, 0);

    // Path do arquivo
    char path[1024];
    strcpy(path, current_dir);
    strcat(path, "/");
    strcat(path, filename);

    // Abre arquivo e trata em caso de erro
    FILE *file = fopen(path, "wb");
    if(file == NULL)
    {
        perror("Erro ao abrir o arquivo para download");
        return;
    }

    // Recebe dados do arquivo
    long bytes_received = 0;
    while(bytes_received < file_size)
    {
        n = recv(socket, buffer, 1024, 0);
        // Escreve se nao for o fim do arquivo
        if(n <= 0)
        {
            break;
        }
        fwrite(buffer, sizeof(char), n, file);
        bytes_received += n;
    }

    // Fecha arquivo e printa mensagem pro usuario
    fclose(file);
    printf("Download de %s concluido com sucesso! (%ld bytes recebidos)\n", filename, bytes_received);

    memset(buffer, '\0', sizeof(buffer));
}

void ls_server(int socket)
{
    // Envia LIST para o servidor e printa o que o servidor mandar
    char buffer[1024];

    send(socket, "LIST", 4, 0);
    
    recv(socket, buffer, 1024, 0);
    printf("%s\n", buffer);

    memset(buffer, '\0', sizeof(buffer));
}

void rm_dir_server(int socket, const char* dirname)
{
    // Envia RMDIR + diretorio e espera resposta
    char buffer[1024];

    sprintf(buffer, "RMDIR %s", dirname);
    send(socket, buffer, strlen(buffer), 0);

    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';
    printf("%s", buffer);

    memset(buffer, '\0', sizeof(buffer));
}

void rm_dir_client(const char* dirname)
{
    char path[1024];
    strcpy(path, current_dir);
    strcat(path, "/");
    strcat(path, dirname);

    if (rmdir(path) == 0) {
        printf("Pasta removida com sucesso.\n");
        strcpy(current_dir, ".");
    } else {
        printf("Erro ao remover pasta.\n");
    }

    memset(path, '\0', sizeof(path));
}

void mk_dir_server(int socket, const char* dirname)
{
    // Envia MKDIR + diretorio e espera resposta
    char buffer[1024];

    sprintf(buffer, "MKDIR %s", dirname);
    send(socket, buffer, strlen(buffer), 0);

    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';
    printf("%s", buffer);

    memset(buffer, '\0', sizeof(buffer));
}

void mk_dir_client(const char* dirname)
{
    char path[1024];
    strcpy(path, current_dir);
    strcat(path, "/");
    strcat(path, dirname);

    if (mkdir(path, 0755) == 0) {
        printf("Pasta criado com sucesso.\n");
    } else {
        printf("Erro ao criar pasta.\n");
    }

    memset(path, '\0', sizeof(path));
}

void re_move_server(int socket, const char* filename)
{
    // Envia REMOVE + arquivo e espera resposta
    char buffer[1024];

    sprintf(buffer, "REMOVE %s", filename);
    send(socket, buffer, strlen(buffer), 0);

    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';
    printf("%s", buffer);

    memset(buffer, '\0', sizeof(buffer));
}

void re_move_client(const char* filename)
{
    char path[1024];
    strcpy(path, current_dir);
    strcat(path, "/");
    strcat(path, filename);

    if (remove(path) == 0) {
        printf("Arquivo removido com sucesso.\n");
    } else {
        printf("Erro ao remover arquivo.\n");
    }

    memset(path, '\0', sizeof(path));
}

void re_name_server(int socket, const char* oldName, const char* newName)
{
    // Envia RMDIR + nomes e espera resposta
    char buffer[1024];

    sprintf(buffer, "RENAME %s %s", oldName, newName);
    send(socket, buffer, strlen(buffer), 0);

    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';
    printf("%s", buffer);

    memset(buffer, '\0', sizeof(buffer));
}

void re_name_client(const char* oldName, const char* newName)
{
    char pathOldName[1024];
    strcpy(pathOldName, current_dir);
    strcat(pathOldName, "/");
    strcat(pathOldName, oldName);

    char pathNewName[1024];
    strcpy(pathNewName, current_dir);
    strcat(pathNewName, "/");
    strcat(pathNewName, newName);

    if (rename(pathOldName, pathNewName) == 0) {
        printf("Renomeado com sucesso.\n");
    } else {
        printf("Erro ao renomear.\n");
    }

    memset(pathOldName, '\0', sizeof(pathOldName));
    memset(pathNewName, '\0', sizeof(pathNewName));
}

void ls_client()
{
    DIR *dir = opendir(current_dir);

    if(dir == NULL)
    {
        perror("Erro ao abrir o diretorio!");
    }
    else
    {
        // Cria uma struct entrada para imprimir o nome de cada arquivo e pasta
        struct dirent *entry;
        
        while((entry = readdir(dir)) != NULL)
        {
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                printf("%s ", entry->d_name);
            }
        }
        printf("\n");
        
        // Fecha arquivo
        closedir(dir);
    }
}

void cd_client(const char* dirname)
{
    char tmp_current_dir[1024];
    strcpy(tmp_current_dir, current_dir);
    
    strcat(tmp_current_dir, "/");
    strcat(tmp_current_dir, dirname);

    DIR * dir = opendir(tmp_current_dir);
    if(dir == NULL)
    {
        perror("Erro ao abrir o diretorio!");
    }
    else
    {
        strcat(current_dir, "/");
        strcat(current_dir, dirname);
    }
}

void cd_server(int socket, const char* dirname)
{
    char buffer[1024];

    sprintf(buffer, "CD %s", dirname);
    send(socket, buffer, strlen(buffer), 0);

    int n = recv(socket, buffer, 1024, 0);
    buffer[n] = '\0';
    printf("%s", buffer);

    memset(buffer, '\0', sizeof(buffer));
}