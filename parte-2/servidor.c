/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº:       Nome:
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
CheckIn clientRequest; // Variável que tem o pedido enviado do Cliente para o Servidor

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main()
{
    // S1
    checkExistsDB_S1(FILE_DATABASE);
    // S2
    createFifo_S2(FILE_REQUESTS);
    // S3
    triggerSignals_S3(FILE_REQUESTS);

    int indexClient; // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD

    // S4: CICLO1
    while (TRUE)
    {
        // S4
        clientRequest = readRequest_S4(FILE_REQUESTS); // S4: "Se houver erro (...) clientRequest.nif == -1"
        if (clientRequest.nif < 0)                     // S4: "Se houver erro na abertura do FIFO ou na leitura do mesmo, (...)"
            continue;                                  // S4: "(...) e recomeça o Ciclo1 neste mesmo passo S4, lendo um novo pedido"

        // S5
        int pidServidorDedicado = createServidorDedicado_S5();
        if (pidServidorDedicado > 0) // S5: "o processo Servidor (pai) (...)"
            continue;                // S5: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
                                     // S5: "o Servidor Dedicado (que tem o PID pidServidorDedicado) segue para o passo SD9"

        // SD9
        triggerSignals_SD9();
        // SD10
        CheckIn itemBD;
        indexClient = searchClientDB_SD10(clientRequest, FILE_DATABASE, &itemBD);
        // SD11
        checkinClientDB_SD11(&clientRequest, FILE_DATABASE, indexClient, itemBD);
        // SD12
        sendAckCheckIn_SD12(clientRequest.pidCliente);
        // SD13
        closeSessionDB_SD13(clientRequest, FILE_DATABASE, indexClient);
        so_exit_on_error(-1, "ERRO: O servidor dedicado nunca devia chegar a este ponto");
    }
}

/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros.
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */

/**
 * @brief S1     Ler a descrição da tarefa S1 no enunciado
 * @param nameDB O nome da base de dados (i.e., FILE_DATABASE)
 */
void checkExistsDB_S1(char *nameDB)
{
    so_debug("< [@param nameDB:%s]", nameDB);

    // Criar uma string para armazenar o comando completo

    if (access(nameDB, R_OK) != -1)
    {
        if (access(nameDB, W_OK) != -1)        
		{
           so_success("S1", "");
		}
        else
        {
            so_error("S1", "");
            exit(1);
        }
    }
    else
    {
        so_error("S1", "");
        exit(1);
    }
    so_debug(">");
}

/**
 * @brief S2       Ler a descrição da tarefa S2 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void createFifo_S2(char *nameFifo)
{
    so_debug("< [@param nameFifo:%s]", nameFifo);

	unlink(nameFifo);

    // Create the FIFO with read and write permissions for the user
    if (mkfifo(nameFifo, 0666) == -1)
    {
        so_error("S2", "");
        exit(1);
    }

    so_success("S2", "");
    so_debug(">");
}

/**
 * @brief S3   Ler a descrição da tarefa S3 no enunciado
 */
void triggerSignals_S3()
{
    so_debug("<");

    // Arma e trata o sinal SIGINT
    if (signal(SIGINT, trataSinalSIGINT_S6) == SIG_ERR)
    {
        so_error("S3", "");
        // Vai para o passo S7 (encerramento do Servidor)
        deleteFifoAndExit_S7();
    }

    // Arma e trata o sinal SIGCHLD
    if (signal(SIGCHLD, trataSinalSIGCHLD_S8) == SIG_ERR)
    {
        so_error("S3", "");
        // Vai para o passo S7 (encerramento do Servidor)
        deleteFifoAndExit_S7();
    }

    so_success("S3", "");
    so_debug(">");
}

/**
 * @brief S4       O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa S4 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn readRequest_S4(char *nameFifo)
{
    CheckIn request;
    request.nif = -1; // Por omissão retorna erro
    so_debug("< [@param nameFifo:%s]", nameFifo);

    FILE *fptr;
    char line[100];
    int i = 0;

    so_debug("Aguardando novo pedido!\n");
    fptr = fopen(nameFifo, "r");
    if (fptr == NULL)
    {
        so_error("S4", "");
        deleteFifoAndExit_S7();
    }

    while (so_fgets(line, 100, fptr))
    {
        if (i == 0)
            request.nif = atoi(line);
        if (i == 1)
        {
            strcpy(request.senha, line);
            int s = strlen(request.senha);
            if (request.senha[s - 1] == '\n')
                request.senha[s - 1] = '\0';
        }
        if (i == 2)
            request.pidCliente = atoi(line);
        i++;
    }
    if (request.nif <= 0 || request.pidCliente <= 0 || strlen(request.senha) <= 0
		|| request.nif > 999999999)
    {
        fclose(fptr);
        so_error("S4.1", "");
        deleteFifoAndExit_S7();
    }
    fclose(fptr);
    so_success("S4.1", "%d %s %d", request.nif, request.senha, request.pidCliente);
    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}

/**
 * @brief S5   Ler a descrição da tarefa S5 no enunciado
 * @return int PID do processo filho, se for o processo Servidor (pai),
 *             0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int createServidorDedicado_S5()
{
    int pid_filho = -1; // Por omissão retorna erro
    so_debug("<");

    int pid = fork();
    if (pid < 0)
    {
        so_error("S5", "");
        deleteFifoAndExit_S7();
    }
    if (pid != 0)
    {
        so_success("S5", "Servidor: Iniciei SD %d", pid);
    }
    pid_filho = pid;
    so_debug("> [@return:%d]", pid_filho);
    return pid_filho;
}

/**
 * @brief S6            Ler a descrição das tarefas S6 e S7 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S6(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    if (sinalRecebido == 2)
    {
        so_success("S6", "Servidor: Start Shutdown");

        FILE *fptr;
        fptr = fopen(FILE_DATABASE, "rb");

        if (fptr == NULL)
        {
            so_error("S6.1", "");
            deleteFifoAndExit_S7();
        }
        so_success("S6.1", "");

        CheckIn data;
        while (fread(&data, sizeof(CheckIn), 1, fptr))
        {
            if (data.pidServidorDedicado > 0)
            {
                kill(data.pidServidorDedicado, SIGUSR2);
                so_success("S6.3", "Servidor: Shutdown SD %d", data.pidServidorDedicado);
            }
        }

        if (feof(fptr))
        {
            so_success("S6.2", "");
        }
        else if (ferror(fptr))
        {
            so_error("S6.2", "");
        }
        fclose(fptr);
        so_debug(">");
        deleteFifoAndExit_S7();
    }
}

/**
 * @brief S7 Ler a descrição da tarefa S7 no enunciado
 */
void deleteFifoAndExit_S7()
{
    so_debug("<");

    int status = unlink(FILE_REQUESTS);
    if (status == 0)
    {
        so_success("S7", "Servidor: End Shutdown");
    }
    else
    {
        so_error("S7", "");
        exit(1);
    }
    so_debug(">");
    exit(0);
}

/**
 * @brief S8            Ler a descrição da tarefa S8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S8(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    pid_t i = wait(NULL);
    if (i > 0)
        so_success("S8", "Servidor: Confirmo fim de SD %d", i);
    so_debug(">");
}

/**
 * @brief SD9  Ler a descrição da tarefa SD9 no enunciado
 */
void triggerSignals_SD9()
{
    so_debug("<");
    // Tratamento pra SIGUSR2, e ignorar SIGINT, SIGUSR1 (Por causado SD12)
    if (signal(SIGUSR2, trataSinalSIGUSR2_SD14) == SIG_ERR || signal(SIGINT, SIG_IGN) == SIG_ERR || signal(SIGUSR1, SIG_IGN) == SIG_ERR)
    {
        so_error("SD9", "");
        exit(1);
    }
    so_success("SD9", "");
    so_debug(">");
}

/**
 * @brief SD10    Ler a descrição da tarefa SD10 no enunciado
 * @param request O pedido do cliente
 * @param nameDB  O nome da base de dados
 * @param itemDB  O endereço de estrutura CheckIn a ser preenchida nesta função com o elemento da BD
 * @return int    Em caso de sucesso, retorna o índice de itemDB no ficheiro nameDB.
 */
int searchClientDB_SD10(CheckIn request, char *nameDB, CheckIn *itemDB)
{
    int indexClient = 0; // SD10:"inicia uma variável indexClient a 0"
    so_debug("< [@param request.nif:%d, request.senha:%s, nameDB:%s, itemDB:%p]", request.nif,
             request.senha, nameDB, itemDB);

    FILE *fptr;
    int found = 0;
    fptr = fopen(nameDB, "rb");

    if (fptr == NULL)
    {
        so_error("SD10", "");
        exit(1);
    }

    while (fread(itemDB, sizeof(CheckIn), 1, fptr))
    {
        if (request.nif == itemDB->nif)
        {
            if (strcmp(request.senha, itemDB->senha) == 0)
            {
                so_success("SD10.3", "%d", indexClient);
                found = 1;
                break;
            }
            else
            {
                fclose(fptr);
                so_error("SD10.3", "Cliente %d: Senha errada", request.nif);
                kill(request.pidCliente, SIGHUP);
                exit(1);
            }
        }

        indexClient++;
    }

    if (feof(fptr))
    {
        if (found == 0)
        {
            fclose(fptr);
            so_error("SD10.1", "Cliente %d: não encontrado", request.nif);
            kill(request.pidCliente, SIGHUP);
            exit(1);
        }
    }
    else if (ferror(fptr))
    {
        fclose(fptr);
        so_error("SD10.1", "");
        exit(1);
    }
    fclose(fptr);

    so_debug("> [@return:%d, nome:%s, nrVoo:%s]", indexClient, itemDB->nome, itemDB->nrVoo);
    return indexClient;
}

/**
 * @brief SD11        Ler a descrição da tarefa SD11 no enunciado
 * @param request     O endereço do pedido do cliente (endereço é necessário pois será alterado)
 * @param nameDB      O nome da base de dados
 * @param indexClient O índica na base de dados do elemento correspondente ao cliente
 * @param itemDB      O elemento da BD correspondente ao cliente
 */
void checkinClientDB_SD11(CheckIn *request, char *nameDB, int indexClient, CheckIn itemDB)
{
    so_debug("< [@param request:%p, nameDB:%s, indexClient:%d, itemDB.pidServidorDedicado:%d]",
             request, nameDB, indexClient, itemDB.pidServidorDedicado);

    strcpy(request->nome, itemDB.nome);
    strcpy(request->nrVoo, itemDB.nrVoo);
    request->pidServidorDedicado = getpid();
    so_success("SD11.1", "%s %s %d", request->nome, request->nrVoo, request->pidServidorDedicado);

    FILE *fptr;
    fptr = fopen(nameDB, "r+");

    if (fptr == NULL)
    {
        so_error("SD11.2", "");
        kill(request->pidCliente, SIGHUP);
        exit(1);
    }

    if (fseek(fptr, indexClient * sizeof(CheckIn), SEEK_SET) != 0)
    {
        fclose(fptr);
        so_error("SD11.3", "");
        kill(request->pidCliente, SIGHUP);
        exit(1);
    }

    fwrite(request, sizeof(CheckIn), 1, fptr);
    if (ferror(fptr))
    {
        fclose(fptr);
        so_error("SD11.4", "");
        kill(request->pidCliente, SIGHUP);
        exit(1);
    }
    fclose(fptr);
    so_success("SD11.4", "");

    so_debug("> [nome:%s, nrVoo:%s, pidServidorDedicado:%d]", request->nome,
             request->nrVoo, request->pidServidorDedicado);
}

/**
 * @brief SD12       Ler a descrição da tarefa SD12 no enunciado
 * @param pidCliente PID (Process ID) do processo Cliente
 */
void sendAckCheckIn_SD12(int pidCliente)
{
    so_debug("< [@param pidCliente:%d]", pidCliente);

    int random_time;

    // Gera um número aleatório entre 1 e MAX_ESPERA
    random_time = so_rand() % MAX_ESPERA + 1;
    so_success("SD12", "%d", random_time);
    sleep(random_time);
    kill(pidCliente, SIGUSR1);
    so_debug(">");
}

/**
 * @brief SD13          Ler a descrição da tarefa SD13 no enunciado
 * @param clientRequest O endereço do pedido do cliente
 * @param nameDB        O nome da base de dados
 * @param indexClient   O índica na base de dados do elemento correspondente ao cliente
 */
void closeSessionDB_SD13(CheckIn clientRequest, char *nameDB, int indexClient)
{
    so_debug("< [@param clientRequest:%p, nameDB:%s, indexClient:%d]", &clientRequest, nameDB,
             indexClient);

    clientRequest.pidCliente = -1;
    clientRequest.pidServidorDedicado = -1;

    FILE *fptr;
    fptr = fopen(nameDB, "r+");

    if (fptr == NULL)
    {
        so_error("SD13.1", "");
        exit(1);
    }
    so_success("SD13.1", "");

    if (fseek(fptr, indexClient * sizeof(CheckIn), SEEK_SET) != 0)
    {
        fclose(fptr);
        so_error("SD13.2", "");
        exit(1);
    }
    so_success("SD13.2", "");

    fwrite(&clientRequest, sizeof(CheckIn), 1, fptr);
    if (ferror(fptr))
    {
        fclose(fptr);
        so_error("SD13.3", "");
        exit(1);
    }
    fclose(fptr);
    so_success("SD13.3", "");

    so_debug("> [pidCliente:%d, pidServidorDedicado:%d]", clientRequest.pidCliente,
             clientRequest.pidServidorDedicado);
    exit(0);
}

/**
 * @brief SD14          Ler a descrição da tarefa SD14 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD14(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    pid_t i = getpid();
    if (i > 0)
    {
        so_success("SD14", "SD: Recebi pedido do Servidor para terminar");
        exit(0);
    }
    so_debug(">");
}
