#!/bin/bash
SO_HIDE_DEBUG=1   ## Uncomment this line to hide all @DEBUG statements
SO_HIDE_COLOURS=1 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh   ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024, Enunciado Versão 3+
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S5. Script: menu.sh
## Descrição/Explicação do Módulo:
##
## This script invokes the remaining scripts, receiving no arguments
##
###############################################################################

# Files
regista_passageiro="./regista_passageiro.sh"
compra_bilhete="./compra_bilhete.sh"
estado_voos="./estado_voos.sh"
stats="./stats.sh"

## S5.1.1.
while [ true ]; do # Send menu information
    echo "MENU:
1: Regista/Atualiza saldo do passageiro
2: Reserva/Compra de bilhetes
3: Atualiza Estado dos voos
4: Estatísticas - Passageiros
5: Estatísticas - Top Voos + Rentáveis
0: Sair
    "

    ## S5.2.1. 
    read -p "Opção: " opt # Check that the option is valid
    if [[ $opt =~ ^[0-5]$ ]]; then
        so_success S5.2.1 $opt
    else
        so_error S5.2.1 $opt
        continue
    fi

    ## S5.2.2.
    case $opt in # Executes the script that the user has chosen
    1) ## S5.2.2.1. 
        # Get user information
        echo "Regista passageiro / Atualiza saldo passageiro:"
        read -p "Indique o nome: " name
        read -p "Indique a senha: " key
        read -p "Para registar o passageiro, insira o NIF: " nif
        read -p "Indique o saldo a adicionar ao passageiro: " balance
        if [ "$nif" = "" ]; then # Check if have nif for registration
            $regista_passageiro "$name" "$key" "$balance"
        else
            $regista_passageiro "$name" "$key" "$balance" "$nif"
        fi
        so_success S5.2.2.1
        ;;

    2) ## S5.2.2.2.
        echo "Reserva/Compra de bilhetes:"
        $compra_bilhete
        so_success S5.2.2.1
        ;;

    3) ## S5.2.2.3. 
        echo "Atualiza Estado dos voos:"
        $estado_voos
        so_success S5.2.2.1
        ;;

    4) ## S5.2.2.4.
        echo "Estatísticas - Passageiros:"
        $stats "passageiros"
        so_success S5.2.2.1
        ;;

    5) ## S5.2.2.5.
        echo "Estatísticas - Top Voos + Rentáveis:"
        read -p "Indique o número de voos a listar: " nrvoos
        $stats "top" $nrvoos
        so_success S5.2.2.1
        ;;

    *)
        exit 0
        ;;
    esac
done
