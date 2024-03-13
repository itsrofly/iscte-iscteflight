#!/bin/bash
SO_HIDE_DEBUG=1   ## Uncomment this line to hide all @DEBUG statements
SO_HIDE_COLOURS=1 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh   ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024, Enunciado Versão 3+
##
## Aluno: Nº: 124171      Nome: Rofly Antonio
## Nome do Módulo: S2. Script: compra_bilhete.sh
## Descrição/Explicação do Módulo:
##
## This script takes no arguments and allows the passenger to buy a ticket for a flight from the list of available flights. 
## To make the purchase, the passenger must log in to confirm their identity and available balance. 
## The available flights are listed in the voos.txt file.
##
###############################################################################


# Files
voos_file="./voos.txt"
pass_file="./passageiros.txt"
rel_file="./relatorio_reservas.txt"
tmp_file="./.tmp_bilhete.txt"


## S2.1.1. 
if [ -f $voos_file ] && [ -f $pass_file ]; then # Check if the files exist
    so_success S2.1.1
else
    so_error S2.1.1
    exit 1
fi

## S2.1.2. 
read -p "Insira a cidade de origem ou destino do voo: " city_input
for word in $city_input; do # Capitalize the City
    if [ "$city" = "" ]; then
        city+="${word^}"
    else
        city+=" ${word^}"
    fi
done

counter=1
found=0
while IFS= read -r voos; do # Read all lines of the file
    if ([ "$(cut -d: -f2 <<<"$voos")" = "$city" ] || [ "$(cut -d: -f3 <<<"$voos")" = "$city" ]) && [ "$(cut -d: -f8 <<<"$voos")" -ge 1 ]; then # Check if the city is in the file

        if [ $found -eq 0 ]; then
            so_success S2.1.2 $city
            found=1
        fi

        # S2.1.3. 
        # List all citys
        echo "$counter.$(cut -d: -f2 <<<"$voos") para $(cut -d: -f3 <<<"$voos"), $(cut -d: -f4 <<<"$voos"), Partida:$(cut -d: -f5 <<<"$voos"), Preço: $(cut -d: -f6 <<<"$voos"), Disponíveis:$(cut -d: -f8 <<<"$voos") lugares"
        citys+=" ${voos// /-}"
        set -- $citys
        ((counter++))
    fi
done <"$voos_file"

if [ $found -eq 0 ]; then
    so_error S2.1.2
    exit 1
else
    # S2.1.3. 
    # Asks the user which city want
    echo -e "0.Sair\n"
    read -p "Insira o voo que pretende reservar: " options
    options=$((options))
    if [ $options -ge 1 ] && [ $options -le $counter ]; then
        chosen_flight="$(eval echo "\$$options")"
        chosen_flight="$(echo "$chosen_flight" | cut -d: -f1):$(echo "$chosen_flight" | cut -d: -f2-3 | tr '-' ' '):$(echo "$chosen_flight" | cut -d: -f4-8)"
        so_success S2.1.3 $options
    else
        so_error S2.1.3
        exit 1
    fi
fi

## S2.1.4. 
# Ask user login information
read -p "Insira o ID do seu utilizador: " user_id
found=0
while IFS= read -r line; do
    if [ "$user_id" = "$(echo $line | cut -d: -f1)" ]; then
        so_success S2.1.4 $user_id
        user_info=$line
        found=1
        break
    fi
done <"$pass_file"
if [ $found -eq 0 ]; then
    so_error S2.1.4
    exit 1
fi

## S2.1.5. 
read -p "Insira a senha do seu utilizador: " password
if [ "$password" = "$(echo $user_info | cut -d: -f5)" ]; then
    so_success S2.1.5
else
    so_error S2.1.5
    exit 1
fi

## S2.2.1. 
# Calculates the user's new balance
if [ "$(echo $user_info | cut -d: -f6)" -ge "$(echo $chosen_flight | cut -d: -f6)" ]; then 
    so_success S2.2.1 "$(echo $chosen_flight | cut -d: -f6)" "$(echo $user_info | cut -d: -f6)"
    newBalance=$(("$(echo $user_info | cut -d: -f6)" - "$(echo $chosen_flight | cut -d: -f6)"))
else
    so_error S2.2.1 "$(echo $chosen_flight | cut -d: -f6)" "$(echo $user_info | cut -d: -f6)"
    exit 1
fi

## S2.2.2.
# Update the pass_file with the new balance
while IFS= read -r line; do 
    if [ "$line" = "$user_info" ]; then
        newline=$(echo "$user_info" | cut -d: -f1-5)
        echo "$newline:$newBalance" >>"$tmp_file"
        if [ "$newline:$newBalance" != "$(tail -n 1 $tmp_file)" ]; then
            so_error S2.2.2
            exit 1
        fi
    else
        echo "$line" >>"$tmp_file"
        if [ "$line" != "$(tail -n 1 $tmp_file)" ]; then
            so_error S2.2.2
            exit 1
        fi
    fi
done <"$pass_file"

if mv "$tmp_file" "$pass_file"; then
    so_success S2.2.2 $newBalance
else
    so_error S2.2.2
    exit 1
fi

## S2.2.3. Update the voos_file with the new number of seats available
while IFS= read -r line; do
    if [ "$line" = "$chosen_flight" ]; then
        newline="$(echo $chosen_flight | cut -d: -f1-7)"
        newplace=$(("$(echo $chosen_flight | cut -d: -f8)" - 1))

        if ! echo "$newline:$newplace" >>"$tmp_file"; then
            so_error S2.2.3
            exit 1
        fi
    else

        if ! echo "$line" >>"$tmp_file"; then
            so_error S2.2.3
            exit 1
        fi
    fi
done <"$voos_file"

if mv "$tmp_file" "$voos_file"; then
    so_success S2.2.3
else
    so_error S2.2.3
    exit 1
fi

## S2.2.4.
# Create a booking report with information
date_booking=$(date +%F)
time_booking=$(date +%Hh%M)
no_voo=$(echo $chosen_flight | cut -d: -f1)
origin=$(echo $chosen_flight | cut -d: -f2)
destination=$(echo $chosen_flight | cut -d: -f3)
cost=$(echo $chosen_flight | cut -d: -f6)

if [ -f $rel_file ]; then
    id_booking=$(($(echo "$(tail -n 1 $rel_file)" | cut -d: -f1) + 1))
    line="$id_booking:$no_voo:$origin:$destination:$cost:$user_id:$date_booking:$time_booking"
    echo "$line" >>"$rel_file"
else
    if >$rel_file; then
        id_booking=1
        line="$id_booking:$no_voo:$origin:$destination:$cost:$user_id:$date_booking:$time_booking"
        echo "$line" >>"$rel_file"
    else
        so_error S2.2.4
        exit 1
    fi
fi

if [ "$(echo $line)" = "$(tail -n 1 $rel_file)" ]; then
    so_success S2.2.4
else
    so_error S2.2.4
    exit 1
fi
