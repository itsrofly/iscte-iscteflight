/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº:       Nome:
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main()
{
    // C1
    checkExistsFifoServidor_C1(FILE_REQUESTS);
    // C2
    triggerSignals_C2();
    // C3 + C4
    CheckIn clientRequest = getDadosPedidoUtilizador_C3_C4();
    // C5
    writeRequest_C5(clientRequest, FILE_REQUESTS);
    // C6
    configureTimer_C6(MAX_ESPERA);
    // C7
    waitForEvents_C7();
    so_exit_on_error(-1, "ERRO: O cliente nunca devia chegar a este ponto");
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1       Ler a descrição da tarefa C1 no enunciado
 * @param nameFifo Nome do FIFO servidor (i.e., FILE_REQUESTS)
 */
void checkExistsFifoServidor_C1(char *nameFifo)
{
    so_debug("< [@param nameFifo:%s]", nameFifo);

    // Create a stat structure
    struct stat file_stat;

    // Get the file status
    if (stat(nameFifo, &file_stat) == -1)
    {
        so_error("C1", "");
        exit(1);
    }

    if (S_ISFIFO(file_stat.st_mode))
    {
        so_success("C1", "");
    }
    else
    {
        so_error("C1", "Não é fifo");
        exit(1);
    }

    so_debug(">");
}

/**
 * @brief C2   Ler a descrição da tarefa C2 no enunciado
 */
void triggerSignals_C2()
{
    so_debug("<");

    if (signal(SIGUSR1, trataSinalSIGUSR1_C8) == SIG_ERR ||
        signal(SIGHUP, trataSinalSIGHUP_C9) == SIG_ERR ||
        signal(SIGINT, trataSinalSIGINT_C10) == SIG_ERR ||
        signal(SIGALRM, trataSinalSIGALRM_C11) == SIG_ERR)
    {
        so_error("C2", "");
        exit(1);
    }
    so_success("C2", "");

    so_debug(">");
}

/**
 * @brief C3+C4    Ler a descrição das tarefas C3 e C4 no enunciado
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn getDadosPedidoUtilizador_C3_C4()
{
    CheckIn request;
    request.nif = -1; // Por omissão retorna erro
    so_debug("<");

    so_debug("\nIscteFlight: Check-in Online\n");
    so_debug("----------------------------\n");
    so_debug("\nIntroduza o NIF do passageiro: ");

    char senha[100];
    size_t nif;

    nif = so_geti();
    if (nif > 999999999)
    {
        so_error("C3", "NIF Invalido!");
        exit(1);
    }

    so_debug("\nIntroduza a Senha do passageiro: ");
    so_gets(senha, 100);
    if (strlen(senha) < 3)
    {
        so_error("C3", "Senha Invalida! Mínimo 3 caracteres");
        exit(1);
    }

    request.nif = nif;
    strcpy(request.senha, senha);
    request.pidCliente = getpid();
    so_success("C4", "%d %s %d", request.nif, request.senha, request.pidCliente);
    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}

/**
 * @brief C5       Ler a descrição da tarefa C5 no enunciado
 * @param request  Elemento com os dados a enviar
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void writeRequest_C5(CheckIn request, char *nameFifo)
{
    so_debug("< [@param request.nif:%d, request.senha:%s, request.pidCliente:%d, nameFifo:%s]",
             request.nif, request.senha, request.pidCliente, nameFifo);

    FILE *fptr;

    fptr = fopen(nameFifo, "w");

    if (fptr == NULL)
    {
        so_error("C5", "");
        exit(1);
    }

    int size = fprintf(fptr, "%d\n%s\n%d\n", request.nif, request.senha, request.pidCliente);
    if (size > 0)
    {
        so_success("C5", "");
    }
    else
    {
        fclose(fptr);
        so_error("C5", "");
        exit(1);
    }
    fclose(fptr);
    so_debug(">");
}

/**
 * @brief C6          Ler a descrição da tarefa C6 no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C6(int tempoEspera)
{
    so_debug("< [@param tempoEspera:%d]", tempoEspera);

    alarm(tempoEspera);
    so_success("C6", "Espera resposta em %d segundos", tempoEspera);
    so_debug(">");
}

/**
 * @brief C7 Ler a descrição da tarefa C7 no enunciado
 */
void waitForEvents_C7()
{
    so_debug("<");
    pause();
    so_debug(">");
}

/**
 * @brief C8            Ler a descrição da tarefa C8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_C8(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C8", "Check-in concluído com sucesso");
    alarm(0);
    exit(0);

    so_debug(">");
}

/**
 * @brief C9            Ler a descrição da tarefa C9 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C9(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C9", "Check-in concluído sem sucesso");
    alarm(0);

    so_debug(">");
	exit(1);
}

/**
 * @brief C10           Ler a descrição da tarefa C10 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C10(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    if (sinalRecebido == 2)
    {
        so_success("C10", "Cliente: Shutdown");
        exit(0);
    }

    so_debug(">");
}

/**
 * @brief C11           Ler a descrição da tarefa C11 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C11(int sinalRecebido)
{
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C11", "Cliente: Timeout");
    exit(1);
    so_debug(">");
}
