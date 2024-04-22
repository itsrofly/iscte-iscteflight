#!/bin/bash
SO_HIDE_DEBUG=1   ## Uncomment this line to hide all @DEBUG statements
SO_HIDE_COLOURS=1 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh   ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024, Enunciado Versão 3+
##
## Aluno: Nº: 124171      Nome: Rofly António
## Nome do Módulo: S3. Script: estado_voos.sh
## Descrição/Explicação do Módulo:
##
## This script receives no arguments and is responsible for reporting the status of flights belonging to the IscteFlight platform.
##
###############################################################################

# Files
voos_file="./voos.txt"
voos_html="./voos_disponiveis.html"


## S3.2.1. 
create_file() { # Create HTML file with the flights informations
    Date=$(date +%F)
    Time=$(date +%H:%M:%S)
    if >$voos_html; then
        echo '<html><head><meta charset="UTF-8"><title>IscteFlight: Lista de Voos Disponíveis</title></head>' >>"$voos_html"
        echo "<body><h1>Lista atualizada em $Date $Time</h1>" >>"$voos_html"
        while IFS= read -r voos; do
            nr_Voo=$(echo "$voos" | cut -d: -f1)
            origin=$(echo "$voos" | cut -d: -f2)
            destination=$(echo "$voos" | cut -d: -f3)
            departure_date=$(echo "$voos" | cut -d: -f4)
            departure_time=$(echo "$voos" | cut -d: -f5)
            lot=$(echo "$voos" | cut -d: -f7)
            avaible_seats=$(echo "$voos" | cut -d: -f8)
            taken_seats=$(($lot - $avaible_seats))
            if [ $avaible_seats -ge 1 ]; then
                echo "<h2>Voo: $nr_Voo, De: $origin Para: $destination, Partida em $departure_date $departure_time</h2>" >>"$voos_html"
                echo "<ul>" >>"$voos_html"
                echo "<li><b>Lotação:</b> $lot Lugares</li>" >>"$voos_html"
                echo "<li><b>Lugares Disponíveis:</b> $avaible_seats Lugares</li>" >>"$voos_html"
                echo "<li><b>Lugares Ocupados:</b> $taken_seats Lugares</li>" >>"$voos_html"
                echo "</ul>" >>"$voos_html"
            fi
        done <"$voos_file"
        echo "</body></html>" >>"$voos_html"
        so_success S3.2.1
    else
        so_error S3.2.1
        exit 1
    fi
}

## S3.1.1.
if [ -e $voos_file ]; then # Check if the voos_file exits
    so_success S3.1.1

    ## S3.1.2. 
    while IFS= read -r line; do # Check that the file structure is correct
        if [[ ! "$line" =~ ^[A-Z]{2}[0-9]{4}:(([A-Za-z]+)|([A-Za-z]+ [A-Za-z]+)):(([A-Za-z]+)|([A-Za-z]+ [A-Za-z]+)):[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1]):([0-1]?[0-9]|2[0-3])h[0-5][0-9]:[0-9]+:[0-9]+:[0-9]+$ ]]; then
            so_error S3.1.2 "$line"
            exit 1
        fi
    done <"$voos_file"
    so_success S3.1.2
    create_file
else
    so_error S3.1.1
    exit 1
fi
