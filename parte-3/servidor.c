/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 1+
 **
 ** Aluno: Nº: Rofly      Nome: António
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int shmId;                              // Variável que tem o ID da Shared Memory
int msgId;                              // Variável que tem o ID da Message Queue
int semId;                              // Variável que tem o ID do Grupo de Semáforos
MsgContent clientRequest;               // Variável que serve para as mensagens trocadas entre Cliente e Servidor
DadosServidor *database = NULL;         // Variável que vai ficar com UM POINTER PARA a memória partilhada
int indexClient = -1;                   // Índice do passageiro que fez o pedido ao servidor/servidor dedicado na BD
int indexFlight = -1;                   // Índice do voo do passageiro que fez o pedido ao servidor/servidor dedicado na BD
int nrServidoresDedicados = 0;          // Número de servidores dedicados (só faz sentido no processo Servidor)

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // S1
    shmId = initShm_S1();
    if (RETURN_ERROR == shmId) terminateServidor_S7();
    // S2
    msgId = initMsg_S2();
    if (RETURN_ERROR == msgId) terminateServidor_S7();
    // S3
    semId = initSem_S3();
    if (RETURN_ERROR == semId) terminateServidor_S7();
    // S4
    if (RETURN_ERROR == triggerSignals_S4()) terminateServidor_S7();

    // S5: CICLO1
    while (TRUE) {
        // S5
        int result = readRequest_S5();
        if (CICLO1_CONTINUE == result) // S5: "Se receber um sinal (...) retorna o valor CICLO1_CONTINUE"
            continue;                  // S5: "para que main() recomece automaticamente o CICLO1 no passo S5"
        if (RETURN_ERROR == result) terminateServidor_S7();
        // S6
        int pidServidorDedicado = createServidorDedicado_S6();
        if (pidServidorDedicado > 0)   // S6: "o processo Servidor (pai) (...) retorna um valor > 0"
            continue;                  // S6: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
        if (RETURN_ERROR == pidServidorDedicado) terminateServidor_S7();
        // S6: "o Servidor Dedicado (...) retorna 0 para que main() siga automaticamente para o passo SD10

        // SD10
        if (RETURN_ERROR == triggerSignals_SD10()) terminateServidorDedicado_SD18();
        // SD11
        indexClient = searchClientDB_SD11();
        int erroValidacoes = TRUE;
        if (RETURN_ERROR != indexClient) {
            // SD12: "Se o passo SD11 concluiu com sucesso: (...)"
            indexFlight = searchFlightDB_SD12();
            if (RETURN_ERROR != indexFlight) {
                // SD13: "Se os passos SD11 e SD12 tiveram sucesso, (...)"
                if (!updateClientDB_SD13())
                    erroValidacoes = FALSE; // erroValidacoes = "houve qualquer erro nas validações dos passos SD11, SD12, ou SD13"
            }
        }
        // SD14: CICLO5
        int escolheuLugarDisponivel = FALSE;
        while (!escolheuLugarDisponivel) {
            // SD14.1: erroValidacoes = "houve qualquer erro nas validações dos passos SD11, SD12, ou SD13"
            if (RETURN_ERROR == sendResponseClient_SD14(erroValidacoes)) terminateServidorDedicado_SD18();
            if (erroValidacoes)
                terminateServidorDedicado_SD18();

            // SD15: "Se os pontos anteriores tiveram sucesso, (...)"
            if (RETURN_ERROR == readResponseClient_SD15()) terminateServidorDedicado_SD18();
            // SD16
            if (RETURN_ERROR == updateFlightDB_SD16())  // SD16: "Se lugarEscolhido no pedido NÃO estiver disponível (...) retorna erro (-1)"
                continue;                               // SD16: "para que main() recomece o CICLO5 em SD14"
            else
                escolheuLugarDisponivel = TRUE;
        }
        sendConfirmationClient_SD17();
        terminateServidorDedicado_SD18();
    }
}

/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros.
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */



/**
 * @brief Check if Database exist and can be accessed
 * @return RETURN_ERROR (-1) in case of error
*/
int checkExistsDB_S1(char *nameDB)
{
    if (access(nameDB, R_OK) != RETURN_ERROR) {
        if (access(nameDB, W_OK) != RETURN_ERROR)        
           return RETURN_SUCCESS;
        return RETURN_ERROR;
    }
    return RETURN_ERROR;
}

/**
 * @brief Remove (SHM, SEM, MSG)
*/
void clear_S7()
{
    shmctl(shmId, IPC_RMID, NULL);
    semctl(semId, 3, IPC_RMID);
    msgctl(msgId, IPC_RMID, NULL);
    so_success("S7.5", "Servidor: End Shutdown");
}

/**
 * @brief Decrements the value of the semaphore variable by 1.
 * If the new value of the semaphore variable is negative, the process executing wait is blocked (i.e., added to the semaphore's queue). 
 * Otherwise, the process continues execution, having used a unit of the resource.
*/
void operation_P(int index) 
{
    struct sembuf operation = {
        index, // Semaphore Index
        -1, // Operation, Decrement the semaphore
        0
    };
    semop(semId, &operation, 1);
}


/**
 * @brief  Increments the value of the semaphore variable by 1. 
 * After the increment, if the pre-increment value was negative (meaning there are processes waiting for a resource), it transfers a blocked process from the semaphore's waiting queue to the ready queue.
*/
void operation_V(int index) 
{
    struct sembuf operation = {
        index, // Semaphore Index
        +1, // Operation, Increment the semaphore
        0 
    };
    semop(semId, &operation, 1);
}

/**
 * @brief S1: Ler a descrição da tarefa no enunciado
 * @return o valor de shmId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initShm_S1 () {
    shmId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    FILE *fp;
    int i;

    //  Validate that the files bd_passengers.dat and bd_voos.dat files exist in the local directory
    if (checkExistsDB_S1(FILE_DATABASE_PASSAGEIROS) == RETURN_ERROR ||
    checkExistsDB_S1(FILE_DATABASE_VOOS) ==  RETURN_ERROR) {
        so_error("S1.1", "");
        return RETURN_ERROR;
    }
    so_success("S1.1", "");

    // Try to open an existing shared memory segment with the specified key
    shmId = shmget(IPC_KEY, sizeof(DadosServidor), 0);

    // If the opening fails, print an error message
    if (shmId == RETURN_ERROR) {
        so_error("S1.2", "");
        if (errno == ENOENT) {
            // Check if the error was due to the SHM not existing yet (ENOENT)
            // If yes then create SHM
            so_success("S1.3", "");

            shmId = shmget(IPC_KEY, sizeof(DadosServidor), IPC_CREAT | IPC_EXCL);
            if (shmId == RETURN_ERROR) {
                so_error("S1.4", "");
                return RETURN_ERROR;
            }

            if ((database = (DadosServidor*)shmat(shmId, NULL, 0)) == (void *) -1) {
                so_error("S1.4", "");
                return RETURN_ERROR;
            }

            if (database == NULL) {
                so_error("S1.4", "");
                return RETURN_ERROR;
            }
            so_success("S1.4", "%d", shmId);

            // Init Database
            for (i = 0; i < MAX_PASSENGERS; i++) 
                database->listClients[i].nif = PASSENGER_NOT_FOUND;
            for (i = 0; i < MAX_FLIGHTS; i++)
                strcpy(database->listFlights[i].nrVoo, FLIGHT_NOT_FOUND);
            so_success("S1.5", "");


            // Read Database Files
            i = 0;
            fp = fopen(FILE_DATABASE_PASSAGEIROS, "r");
            if (fp == NULL) { 
                so_error("S1.6", "");   
                return RETURN_ERROR;
            }
            while(fread(&database->listClients[i], sizeof(CheckIn), 1, fp)) {
                database->listClients[i].pidCliente = PID_INVALID;
                database->listClients[i].pidServidorDedicado = PID_INVALID;
                i++;
            }
            fclose(fp);
            so_success("S1.6", "");

            // Read Database Files
            i = 0;
            fp = fopen(FILE_DATABASE_VOOS, "r");
            if (fp == NULL) { 
                so_error("S1.7", "");   
                return RETURN_ERROR;
            }

            while(fread(&database->listFlights[i], sizeof(Voo), 1, fp))
                i++;
            fclose(fp);
            so_success("S1.7", "");
            return shmId;
        }
        so_error("S1.3", "");
        return RETURN_ERROR;
    }
    so_success("S1.2", "");

    if ((database = (DadosServidor*)shmat(shmId, NULL, 0)) == (void *) -1) {
        so_error("S1.2.1", "");
        return RETURN_ERROR;
    }

    if (database == NULL) {
        so_error("S1.2.1", "");
        return RETURN_ERROR;
    }
    so_success("S1.2.1", "%d", shmId);

    so_debug("> [@return:%d]", shmId);
    return shmId;
}

/**
 * @brief S2: Ler a descrição da tarefa no enunciado
 * @return o valor de msgId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initMsg_S2 () {
    msgId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // S2.1 Remove the message queue if it already exists
    msgId = msgget(IPC_KEY, 0);
    if (msgId != RETURN_ERROR) {
        if (msgctl(msgId, IPC_RMID, NULL) == RETURN_ERROR) {
            so_error("S2.1", "");
            return RETURN_ERROR;
        } else {
            so_success("S2.1", "");
        }
    }

    // S2.2 Create the message queue
    msgId = msgget(IPC_KEY, IPC_CREAT | 0666);
    if (msgId == RETURN_ERROR) {
        so_error("S2.2", "");
        return RETURN_ERROR;
    }
    so_success("S2.2", "%d", msgId);

    so_debug("> [@return:%d]", msgId);
    return msgId;
}

/**
 * @brief S3: Ler a descrição da tarefa no enunciado
 * @return o valor de semId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initSem_S3 () {
    semId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Delete semaphore if it exists
    semId = semget(IPC_KEY, 3, 0);
    if (semId != RETURN_ERROR) {
        if (semctl(semId, 0, IPC_RMID, 0) == RETURN_ERROR) {
            so_error("S3.1", "");
            return RETURN_ERROR;
        } else {
            so_success("S3.1", "");
        }
    }


    // Create semaphore
    semId = semget(IPC_KEY, 3, IPC_CREAT | 0666);
    if (semId == RETURN_ERROR) {
        so_error("S3.2", "");
        return RETURN_ERROR;
    }
    so_success("S3.2", "%d", semId);

    // Set initial values of semaphores
    if ( semctl(semId, SEM_PASSAGEIROS, SETVAL, 1) == RETURN_ERROR ||
    semctl(semId, SEM_VOOS, SETVAL, 1) == RETURN_ERROR ||
    semctl(semId, SEM_NR_SRV_DEDICADOS, SETVAL, 0) == RETURN_ERROR ) {
        so_error("S3.3", "");
        return RETURN_ERROR;
    }
    so_success("S3.3", "");
    
    so_debug("> [@return:%d]", semId);
    return semId;
}

/**
 * @brief S4: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_S4 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Create signal handlers
    if (signal(SIGINT, trataSinalSIGINT_S8) == SIG_ERR) {
      so_error("S4", "");
      return result;
    }
    if (signal(SIGCHLD, trataSinalSIGCHLD_S9) == SIG_ERR) {
      so_error("S4", "");
      return result;
    }
    so_success("S4", "");
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief S5: O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readRequest_S5 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Read clientRequest Message Queue
    ssize_t i = msgrcv(msgId, &clientRequest, sizeof(clientRequest.msgData), MSGTYPE_LOGIN, 0);
    if (i == RETURN_ERROR) {
        if (errno == EINTR)
            return CICLO1_CONTINUE;
        so_error("S5", "");
        return result; 
    }
    // Log message
    so_success("S5", "%d %s %d", clientRequest.msgData.infoCheckIn.nif,
    clientRequest.msgData.infoCheckIn.senha, 
    clientRequest.msgData.infoCheckIn.pidCliente);
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief S6: Ler a descrição da tarefa no enunciado
 * @return PID do processo filho, se for o processo Servidor (pai),
 *         0 se for o processo Servidor Dedicado (filho),
 *         ou PID_INVALID (-1) em caso de erro
 */
int createServidorDedicado_S6 () {
    int pid_filho = PID_INVALID;    // Por omissão retorna erro
    so_debug("<");

    // Create child process to handle the next steps of the checkin
    pid_filho = fork();
    if (pid_filho < 0) {    
        so_error("S6", "");
        pid_filho = PID_INVALID;
    }
    else if (pid_filho != 0) {
        so_success("S6", "Servidor: Iniciei SD %d", pid_filho);
        nrServidoresDedicados++;
    }
    else
    {
        so_success("S6", "Servidor Dedicado: Nasci");
    }
        
    so_debug("> [@return:%d]", pid_filho);
    return pid_filho;
}

/**
 * @brief S7: Ler a descrição da tarefa no enunciado
 */
void terminateServidor_S7 () {
    FILE *fp;
    int i;
    so_debug("<");

    // Start Shutdown
    so_success("S7", "Servidor: Start Shutdown");
    if (shmId == RETURN_ERROR || database == NULL) {
        // If not shared memmory or database turn down the system
        so_error("S7.1", "");
        clear_S7();
        exit(0);
    }
    so_success("S7.1", "");

    // Save database informations to the file
    for (i = 0; i < MAX_PASSENGERS ; i++) {
        int pid;
        if ((pid = database->listClients[i].pidServidorDedicado) > 0) {
            kill(pid, SIGUSR2);
            so_success("S7.2", "Servidor: Shutdown SD %d", pid);
        }
    }

    // Create a barrier to wait child process end
    struct sembuf operation = {
        SEM_NR_SRV_DEDICADOS,
        -nrServidoresDedicados,
        0
    };
    semop(semId, &operation, 1);
    so_success("S7.3", "");

    // Save database informations, passageiros
    i = 0;
    fp = fopen(FILE_DATABASE_PASSAGEIROS, "w");
    if (fp != NULL)
        while (i < sizeof(database->listClients) / sizeof(CheckIn)) {
            fwrite(&database->listClients[i], sizeof(CheckIn), 1, fp);
            i++;
        }
    fclose(fp);

    // Save database informations, voos
    i = 0;
    fp = fopen(FILE_DATABASE_VOOS, "w");
    if (fp != NULL)
        while (i < sizeof(database->listFlights) / sizeof(Voo)) {
            fwrite(&database->listFlights[i], sizeof(Voo), 1, fp);
            i++;
        }
    fclose(fp);
    so_success("S7.4", "");
    clear_S7();

    so_debug(">");
    exit(0);
}

/**
 * @brief S8: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);
    // Terminate server
    so_success("S8", "");
    terminateServidor_S7();
    so_debug(">");
}

/**
 * @brief S9: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S9 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Get pid of dying process
    int pid = wait(NULL);
    // Start the barrier
    so_success("S9", "Servidor: Confirmo fim de SD %d", pid);
    operation_V(SEM_NR_SRV_DEDICADOS);

    so_debug(">");
}

/**
 * @brief SD10: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_SD10 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Handle child signals
    if (signal(SIGUSR1, trataSinalSIGUSR1_SD19) == SIG_ERR)
    {    
        so_error("SD10", "");
        return result;
    }
    if (signal(SIGUSR2, trataSinalSIGUSR2_SD20) == SIG_ERR)
    {    
        so_error("SD10", "");
        return result;
    }
    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
    {    
        so_error("SD10", "");
        return result;
    }
    so_success("SD10", "");
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD11: Ler a descrição da tarefa no enunciado
 * @return indexClient em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int searchClientDB_SD11 () {
    indexClient = -1;    // SD11: Inicia a variável indexClient a -1 (índice da lista de passageiros de database)
    so_debug("<");

    // Check if client nif nad password is valid
    for (int i = 0; i < sizeof(database->listClients) / sizeof(CheckIn); i++) {
        CheckIn client = database->listClients[i];

        if (client.nif == clientRequest.msgData.infoCheckIn.nif) {
            if (strcmp(client.senha, clientRequest.msgData.infoCheckIn.senha) == 0) {
                indexClient = i;
                so_success("SD11.3", "%d", indexClient);
                break;
            }
            else
                so_error("SD11.3", "Cliente %d: Senha errada", client.nif);
        }
    }

    if (indexClient == -1)
        so_error("SD11.1", "Cliente %d: não encontrado", clientRequest.msgData.infoCheckIn.nif);

    so_debug("> [@return:%d]", indexClient);
    return indexClient;
}

/**
 * @brief SD12: Ler a descrição da tarefa no enunciado
 * @return indexFlight em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int searchFlightDB_SD12 () {
    indexFlight = -1;    // SD12: Inicia a variável indexFlight a -1 (índice da lista de voos de database)
    so_debug("<");
    CheckIn client = database->listClients[indexClient];

    // Check if client flight number is valid
    for (int i = 0; i < sizeof(database->listFlights) / sizeof(Voo); i++) {
        Voo flight = database->listFlights[i];

        if (strcmp(flight.nrVoo, client.nrVoo) == 0) {
            indexFlight = i;
            so_success("SD12.2", "%d", indexFlight);
            break;
        }
    }

    if (indexFlight == -1)
        so_error("SD12.1", "Voo %s: não encontrado", client.nrVoo);

    so_debug("> [@return:%d]", indexFlight);
    return indexFlight;
}

/**
 * @brief SD13: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int updateClientDB_SD13 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro

    so_debug("<");

    so_success("SD13.1", "Start Check-in: %d %d", 
    clientRequest.msgData.infoCheckIn.nif, 
    clientRequest.msgData.infoCheckIn.pidCliente);

    operation_P(SEM_PASSAGEIROS);

    if (database->listClients[indexClient].lugarEscolhido != EMPTY_SEAT) {
        operation_V(SEM_PASSAGEIROS);
        so_error("SD13.2", "Cliente %d: Já fez check-in", 
        database->listClients[indexClient].nif);
        return result;
    }
    if (database->listClients[indexClient].pidCliente != PID_INVALID) {
        operation_V(SEM_PASSAGEIROS);
        so_error("SD13.2", "Cliente %d: Já fez check-in", 
        database->listClients[indexClient].nif);
        return result;
    }
    sleep(4);

    database->listClients[indexClient].pidCliente
     = clientRequest.msgData.infoCheckIn.pidCliente;
    database->listClients[indexClient].pidServidorDedicado = getpid();

    operation_V(SEM_PASSAGEIROS);

    so_success("SD13.5", "End Check-in: %d %d", 
    database->listClients[indexClient].nif, 
    database->listClients[indexClient].pidCliente);
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD14: Ler a descrição da tarefa no enunciado
 * @param erroValidacoes booleano que diz se houve algum erro nas validações de SD11, SD12 e SD13
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendResponseClient_SD14 (int erroValidacoes) {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("< [@param erroValidacoes:%d]", erroValidacoes);

 clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidCliente;

    if (erroValidacoes) {
        so_error("SD14.1", ""); // Error handling
        clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID;
    } else
    {
        Voo flight = database->listFlights[indexFlight];
        clientRequest.msgData.infoCheckIn.pidServidorDedicado = getpid();
        clientRequest.msgData.infoCheckIn.lugarEscolhido = EMPTY_SEAT;
        
        strcpy(clientRequest.msgData.infoVoo.destino, flight.destino);
        strcpy(clientRequest.msgData.infoVoo.origem, flight.origem);
        strcpy(clientRequest.msgData.infoVoo.nrVoo, flight.nrVoo);
        memcpy(clientRequest.msgData.infoVoo.lugares, flight.lugares, sizeof(flight.lugares));

        so_success("SD14.1", ""); // Success message
    }

    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) == RETURN_ERROR)
    {
        so_error("SD14.2", ""); // Error handling
        return result;
    }
    so_success("SD14.2", ""); // Success message
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD15: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readResponseClient_SD15 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    ssize_t i = msgrcv(msgId, &clientRequest, sizeof(clientRequest.msgData), getpid(), 0);
    if (i == RETURN_ERROR) {
        so_error("SD15", ""); // Error handling
        return result; 
    }
    so_success("SD15", "%d %d %d",
    clientRequest.msgData.infoCheckIn.nif,
    clientRequest.msgData.infoCheckIn.lugarEscolhido,
    clientRequest.msgData.infoCheckIn.pidCliente);
    result = RETURN_SUCCESS;
    
    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD16: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int updateFlightDB_SD16 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    CheckIn info = clientRequest.msgData.infoCheckIn;
    so_debug("<");

    so_success("SD16.1", "Start Reserva lugar: %s %d %d",
    info.nrVoo, info.nif, info.lugarEscolhido);

    operation_P(SEM_VOOS); // Wait for flights semaphore

    if (database->listFlights[indexFlight].lugares[info.lugarEscolhido] != EMPTY_SEAT) 
    {
        operation_V(SEM_VOOS); // Release semaphore
        so_error("SD16.2", "Cliente %d: Lugar já estava ocupado", 
        database->listClients[indexClient].nif);
        return result;
    }
    sleep(4); // Simulating work time

    database->listFlights[indexFlight].lugares[info.lugarEscolhido] = info.nif;
    database->listClients[indexClient].lugarEscolhido = info.lugarEscolhido;

    operation_V(SEM_VOOS); // Release semaphore
    
    so_success("SD16.6", "End Reserva lugar: %s %d %d",
    database->listFlights[indexFlight].nrVoo,
     database->listClients[indexClient].nif, 
     database->listClients[indexClient].lugarEscolhido);
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD17: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendConfirmationClient_SD17 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");
    
    clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidCliente;

    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) == RETURN_ERROR) {
        so_error("SD17", ""); // Error handling
        return result;
    }
    so_success("SD17", ""); // Success message
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD18: Ler a descrição da tarefa no enunciado
 */
void terminateServidorDedicado_SD18 () {
    so_debug("<");

    if (indexClient >= 0) {
        database->listClients[indexClient].pidCliente = PID_INVALID;
        database->listClients[indexClient].pidServidorDedicado = PID_INVALID;
    }
    so_success("SD18", "");  // Success message

    so_debug(">");
    exit(0);
}

/**
 * @brief SD19: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_SD19 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("SD19", "SD: Recebi pedido do Cliente para terminar");
    terminateServidorDedicado_SD18();
    so_debug(">");
}

/**
 * @brief SD20: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD20 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("SD20", "SD: Recebi pedido do Servidor para terminar");

    int pid;
    if ((pid = clientRequest.msgData.infoCheckIn.pidCliente) != PID_INVALID)
        kill(pid, SIGHUP);
    terminateServidorDedicado_SD18();
    so_debug(">");
}