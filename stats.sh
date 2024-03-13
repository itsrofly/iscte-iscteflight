#!/bin/bash
SO_HIDE_DEBUG=1   ## Uncomment this line to hide all @DEBUG statements
SO_HIDE_COLOURS=1 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh   ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024, Enunciado Versão 3+
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S4. Script: stats.sh
## Descrição/Explicação do Módulo:
##
## This script obtains information about the system, displaying different results in STDOUT depending on the arguments passed at its invocation.
## The summarised syntax is: ./stats.sh <passengers>|<top <nr>>
##
###############################################################################

rela_file="./relatorio_reservas.txt"
voos_file="./voos.txt"
pass_file="./passageiros.txt"

tmp_file="./.tmp_stats.txt"
aux_file="./.aux_file.txt"

stats_file="./stats.txt"

my_name() {
    if [ ! -f $pass_file ]; then # Check if file exist
        return 1
    fi

    if grep -q "$1" "$pass_file"; then # Get user name
        file_line=$(grep "$1" "$pass_file" | head -n 1)
        echo "$file_line" | cut -d: -f3
        return 0
    else
        return 1
    fi
}

add_info() {
    target=$(echo "$1")
    value=$(echo $2)

    if ! >$aux_file; then # Create aux file
        return 1
    fi

    if grep -q "$target" "$tmp_file"; then # Check if target info is in file
        while IFS= read -r line; do # If yes then update the info stats with then new info
            if [ "$target" = "$(echo $line | cut -d: -f1)" ]; then
                i=$(($(echo $line | cut -d: -f2) + 1))
                k=$(($(echo $line | cut -d: -f3) + $value))
                echo "$target:$i:$k" >>$aux_file
            else
                echo "$line" >>$aux_file
            fi
        done <"$tmp_file"

        if sort -t: -k3,3 -nsr "$aux_file" >"$tmp_file"; then # Sort tmp file
            rm $aux_file
            return 0
        else
            return 1
        fi
    else # If not then add the new info
        echo "$target:1:$value" >>"$tmp_file"
        if sort -t: -k3,3 -nsr "$tmp_file" >"$aux_file"; then # Sort tmp file
            if mv "$aux_file" "$tmp_file"; then
                return 0
            else
                return 1
            fi
        else
            return 1
        fi
    fi
}


## S4.1.1. 
if [ $# -eq 1 ]; then # Checks the user arguments
    if [ "$1" = "passageiros" ]; then
        so_success S4.1.1

        ## S4.2.1. 
        if [ ! -f $rela_file ] || ! >$tmp_file; then # Check if the report file exists and create a tmp file
            so_error S4.2.1
            exit 1
        fi
        while IFS= read -r line; do # Get user information
            user="$(echo $line | cut -d: -f6)"
            cost="$(echo $line | cut -d: -f5)"
            name="$(my_name "$user")"
            if [ $? -eq 1 ]; then
                so_error S4.2.1
                exit 1
            fi
            add_info "$name" $cost # Add info to the tmp stats file
            if [ $? -eq 1 ]; then
                so_error S4.2.1
                exit 1
            fi
        done <"$rela_file"

        if ! >$stats_file; then # Create final stats file
            so_error S4.2.1
            exit 1
        fi

        while IFS= read -r line; do # Get the tmp stats file
            # Applies the information to the final file following the requested structure
            reserva="reserva"
            if [ $(echo $line | cut -d: -f2) -ge 2 ]; then
                reserva="reservas"
            fi
            echo "$(echo $line | cut -d: -f1): $(echo $line | cut -d: -f2) $reserva; $(echo $line | cut -d: -f3)€" >>"$stats_file"

        done <"$tmp_file"

        if rm $tmp_file; then # Remove tmp file
            so_success S4.2.1
        else
            so_error S4.2.1
        fi
    else
        so_error S4.1.1
        exit 1
    fi
elif [ $# -eq 2 ]; then
    if [ "$1" = "top" ] && [[ "$2" =~ ^[1-9]+[0-9]*$ ]]; then
        so_success S4.1.1

        ## S4.2.2. 
        if [ ! -f $rela_file ] || ! >$tmp_file; then # Check if the report file exists and create a tmp file
            so_error S4.2.2
            exit 1
        fi
        while IFS= read -r line; do # Get flight information
            nrvo="$(echo $line | cut -d: -f2)"
            cost="$(echo $line | cut -d: -f5)"
            add_info "$nrvo" $cost # Add info to the tmp stats file
            if [ $? -eq 1 ]; then
                so_error S4.2.2
                exit 1
            fi
        done <"$rela_file"

         # Create final stats file
        if ! >$stats_file; then
            so_error S4.2.2
            exit 1
        fi

        i=$(echo "$2") # Get the top number
        while IFS= read -r line; do # list $i the best flights
            if [ $i -ge 1 ]; then
                echo "$(echo $line | cut -d: -f1): $(echo $line | cut -d: -f3)€" >>"$stats_file"
                ((i--))
            fi

        done <"$tmp_file"

        if rm $tmp_file; then # Remove tmp file
            so_success S4.2.2
        else
            so_error S4.2.2
        fi
    else
        so_error S4.1.1
        exit 1
    fi
else
    so_error S4.1.1
    exit 1
fi
cat "$stats_file" #Show stats info
