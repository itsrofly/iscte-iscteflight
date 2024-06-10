/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº: 124171      Nome: Rofly António
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int msgId;                              // Variável que tem o ID da Message Queue
MsgContent clientRequest;               // Variável que serve para as mensagens trocadas entre Cliente e Servidor
int nrTentativasEscolhaLugar = 0;       // Variável que indica quantas tentativas houve até conseguir encontrar um lugar

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // C1
    msgId = initMsg_C1();
    so_exit_on_error(msgId, "initMsg_C1");
    // C2
    so_exit_on_error(triggerSignals_C2(), "triggerSignals_C2");
    // C3
    so_exit_on_error(getDadosPedidoUtilizador_C3(), "getDadosPedidoUtilizador_C3");
    // C4
    so_exit_on_error(sendRequest_C4(), "sendRequest_C4");
    // C5: CICLO6
    while (TRUE) {
        // C5
        configureTimer_C5(MAX_ESPERA);
        // C6
        so_exit_on_error(readResponseSD_C6(), "readResponseSD_C6");
        // C7
        int lugarEscolhido = trataResponseSD_C7();
        if (RETURN_ERROR == lugarEscolhido)
            terminateCliente_C9();
        // C8
        if (RETURN_ERROR == sendSeatChoice_C8(lugarEscolhido))
            terminateCliente_C9();
    }
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1: Ler a descrição da tarefa no enunciado
 * @return o valor de msgId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initMsg_C1 () {
    msgId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Attempt to get a message queue identifier using a specified IPC key
    msgId = msgget(IPC_KEY, 0);

    // Check if there was an error in getting the message queue identifier
    if (msgId == RETURN_ERROR)
        // Log an error message indicating failure with identifier "C1"
        so_error("C1", "");
    else
        so_success("C1", "%d", msgId);
    return msgId;

    so_debug("> [@return:%d]", msgId);
}

/**
 * @brief C2: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_C2 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Attempt to set up a signal handler for SIGHUP
    if (signal(SIGHUP, trataSinalSIGHUP_C10) == SIG_ERR)
    {    
        // Log an error message with identifier "C2" if setting the handler fails
        so_error("C2", "");
        return result;
    }

    // Attempt to set up a signal handler for SIGINT
    if (signal(SIGINT, trataSinalSIGINT_C11) == SIG_ERR)
    {    
        // Log an error message with identifier "C2" if setting the handler fails
        so_error("C2", "");
        return result;
    }

    // Attempt to set up a signal handler for SIGALRM
    if (signal(SIGALRM, trataSinalSIGALRM_C12) == SIG_ERR)
    {    
        // Log an error message with identifier "C2" if setting the handler fails
        so_error("C2", "");
        return result;
    }

    // Log a success message with identifier "C2" if all signal handlers are set successfully
    so_success("C2", "");
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C3: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int getDadosPedidoUtilizador_C3 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    char senha[100];
    size_t nif;

    so_debug("<");

    // Print the header for the online check-in
    printf("\nIscteFlight: Check-in Online\n");
    printf("----------------------------\n");
    printf("\nIntroduza o NIF do passageiro: ");

    // Get the passenger's NIF
    nif = so_geti();

    // Validate the NIF; it should be a 9-digit number
    if (nif > 999999999 || nif < 100000000 )
    {
        so_error("C3", "NIF Invalido!");
        return result;
    }

    // Get the user password
    printf("\nIntroduza a Senha do passageiro: ");
    so_gets(senha, 100);

    // Validate the password; it should be at least 3 characters long
    if (strlen(senha) < 3)
    {
        so_error("C3", "Senha Invalida! Mínimo 3 caracteres");
        return result;
    }

    // Store the valid NIF in the client request structure
    clientRequest.msgData.infoCheckIn.nif = nif;

    // Copy the valid password into the client request structure
    strcpy(clientRequest.msgData.infoCheckIn.senha, senha);

    so_success("C3", "");
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    
    return result;
}

/**
 * @brief C4: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendRequest_C4 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // Assign the process ID of the current process to the pidCliente field
    clientRequest.msgData.infoCheckIn.pidCliente = getpid();

    // Set the pidServidorDedicado field to an invalid PID, indicating no dedicated server yet
    clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID;

    // Set the message type to MSGTYPE_LOGIN, indicating a login request
    clientRequest.msgType = MSGTYPE_LOGIN;

    // Send the clientRequest message to the message queue identified by msgId
    // The size of the message sent is the size of clientRequest.msgData
    // The last parameter '0' indicates no special flags are used
    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) == RETURN_ERROR) {
        so_error("C4", "");
        return result;
    }

    // Log a success message
    so_success("C4", "%d %s %d",
        clientRequest.msgData.infoCheckIn.nif,
        clientRequest.msgData.infoCheckIn.senha,
        clientRequest.msgData.infoCheckIn.pidCliente);
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C5: Ler a descrição da tarefa no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C5 (int tempoEspera) {
    so_debug("< [@param tempoEspera:%d]", tempoEspera);

    // Create alarm using tempoEspera
    alarm(tempoEspera); 

    // Log a success message
    so_success("C5", "Espera resposta em %d segundos", MAX_ESPERA);

    so_debug(">");
}

/**
 * @brief C6: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readResponseSD_C6 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");


    // Read Message with clientRequest
    // The message type is the client pid
    // No flags used
    ssize_t i = msgrcv(msgId, &clientRequest, sizeof(clientRequest.msgData), getpid(), 0);    
    if (i == RETURN_ERROR) {
        so_error("C6", "");
        return result; 
    }
    // Log success message
    so_success("C6", "%d %d %d",
    clientRequest.msgData.infoCheckIn.nif,
    clientRequest.msgData.infoCheckIn.lugarEscolhido,
    clientRequest.msgData.infoCheckIn.pidCliente);
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C7: Ler a descrição da tarefa no enunciado
 * @return Nº do lugar escolhido (0..MAX_SEATS-1) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int trataResponseSD_C7 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    CheckIn client = clientRequest.msgData.infoCheckIn;
    Voo flight = clientRequest.msgData.infoVoo;
    so_debug("<");

    // Remove Alarm
    alarm(0);

    if (client.pidServidorDedicado == PID_INVALID) {
        // Means that the login operations failed
        so_error("C7.2", "Falha no login");
        exit(1);
    }

    if (client.lugarEscolhido != EMPTY_SEAT) {
        // Reservation is already done
        so_success("C7.3", "Reserva concluída: %s %s %d", 
        flight.origem, flight.destino, client.lugarEscolhido);
        exit(0);
    }

    // Manage the number of attemps
    if (nrTentativasEscolhaLugar++ > 0)
        so_error("C7.4.1", "Erro na reserva de lugar");
    else    
        so_success("C7.4.1", "");

    // Show Seats
    printf("IscteFlight: Voo %s\n", flight.nrVoo);
    printf("---------------------------\n");
    printf("Lugares disponíveis: ");
    for (int i = 0, f = 0; i < MAX_SEATS; i++) {
        if (flight.lugares[i] == EMPTY_SEAT) {
            if (f++ == 0)
                printf("%d", i);
            else
                printf(", %d", i);
        }
    }

    // Get user seat
    printf("\nIntroduza o lugar que deseja reservar: ");
    result = so_geti();

    // Valid seat
    if (result < 0 || result > MAX_SEATS - 1 
    || flight.lugares[result] != EMPTY_SEAT) {
        so_error("C7.4.3", "");
        return RETURN_ERROR; 
    }
    so_success("C7.4.3", "%d", result);

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C8: Ler a descrição da tarefa no enunciado
 * @param lugarEscolhido índice do array lugares que o utilizador escolheu, entre 0 e MAX_SEATS-1
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendSeatChoice_C8 (int lugarEscolhido) {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("< [@param lugarEscolhido:%d]", lugarEscolhido);

    // Change Message type to dedicated server pid
    clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidServidorDedicado;

    // Assign the process ID of the current process to the pidCliente field
    clientRequest.msgData.infoCheckIn.pidCliente = getpid();

    // Set the chosen seat
    clientRequest.msgData.infoCheckIn.lugarEscolhido = lugarEscolhido;

    // Send message to dedicated server
    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) == RETURN_ERROR) {
        so_error("C8", "");
        return result;
    }
    so_success("C8", "%d %d %d", 
    clientRequest.msgData.infoCheckIn.nif,
    clientRequest.msgData.infoCheckIn.lugarEscolhido,
    clientRequest.msgData.infoCheckIn.pidCliente);
    result = RETURN_SUCCESS;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C9: Ler a descrição da tarefa no enunciado
 */
void terminateCliente_C9 () {
    so_debug("<");

    if (clientRequest.msgData.infoCheckIn.pidServidorDedicado
    // Means that some process failed
      == PID_INVALID)
        so_error("C9", "");
    else 
    {
    // Send signal to the dedicated server, to turn down
        int status = kill(clientRequest.msgData.infoCheckIn.pidServidorDedicado,
        SIGUSR1);
        so_success("C9", "");
        if (status == RETURN_ERROR)
            so_error("C9", "");
    }

    so_debug(">");
    exit(0);
}

/**
 * @brief C10: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C10 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Log fail check-in
    so_success("C10", "Check-in concluído sem sucesso");
    exit(0);
    so_debug(">");
}

/**
 * @brief C11: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C11 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Log shutdown
    so_success("C11", "Cliente: Shutdown");
    terminateCliente_C9();

    so_debug(">");
}

/**
 * @brief C12: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C12 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // Log timout
    so_error("C12", "Cliente: Timeout");
    terminateCliente_C9();

    so_debug(">");
}