#!/bin/bash
. ./so_utils.sh   ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024, Enunciado Versão 3+
##
## Aluno: Nº: 124171     Nome: Rofly Antonio
## Nome do Módulo: S1. Script: regista_passageiro.sh
## Descrição/Explicação do Módulo:
##
## This script is invoked when a new passenger registers on the IscteFlight platform.
## This script receives all the data by argument in the command line call.
## Passengers are registered in the passengers.txt file.
## It should receive the passenger's information as arguments in the following order:
## <User(passwd) Full Name:string> <Password (Random):string> <Newb balance to add:number> <NIF:number>.
##
###############################################################################

# User arguments
name=$1
password=$2
balance=$3
nif=$4

# Global variables
username=
email=
file_line=

# Files
file="passageiros.txt"
sorted_file="passageiros-saldos-ordenados.txt"

## S1.1.1.
if [ $# -ge 3 ] && [ $# -le 4 ]; then # Check that the number of arguments is ideal
    so_success S1.1.1
else
    so_error S1.1.1
    exit 1
fi

## S1.1.2.
# Check that the name exists on the tiger server, in this case the users in the _etc_passwd file.
found=0
if ! [[ "$name" =~ ^([a-zA-Z]+)|([a-zA-Z]+ [a-zA-Z]+)+$ ]]; then
    so_error S1.1.2
    exit 1
fi

while IFS=: read -r username _ _ _ user_info _ _; do
    if [ "${user_info%%,*}" = "$name" ]; then
        so_success S1.1.2
        match="$username:${user_info%%,*}"
        found=1
        break
    fi
done < <(getent passwd | sort -u)
if [ $found -eq 0 ]; then
    so_error S1.1.2
    exit 1
fi

## S1.1.3.
# Check if new balance argument is a number
if [[ "$balance" =~ ^[0-9]+$ ]]; then
    so_success S1.1.3
else
    so_error S1.1.3
    exit 1
fi

## S1.1.4.
# Check if the nif is valid
if [ $# -ge 4 ]; then

    if [[ "$nif" =~ ^[0-9]{9}$ ]]; then
        so_success S1.1.4
    else
        so_error S1.1.4
        exit 1
    fi

fi

## S1.2.2.
create_file() { # Create file
    if >$file; then
        so_success S1.2.2
    else
        so_error S1.2.2
        exit 1
    fi
}

# S1.2.4 - S1.2.7
create_line() {
    ## S1.2.4.
    # Checks that the nif was created to register the user
    if [ -n "$nif" ]; then
        so_success S1.2.4
    else
        so_error S1.2.4
        exit 1
    fi

    ## S1.2.5.
    # Get user by corresponding name on server
    username=$(cut -d: -f1 <<<"$match")
    if [ "$username" = "" ]; then
        so_error S1.2.5
        exit 1
    else
        so_success S1.2.5 $username
    fi

    ## S1.2.6.
    # Create email with first and last name
    name_array=($name)
    email=${name_array[0],,}.${name_array[-1],,}@iscteflight.pt

    if [ "$email" = "" ] || [ ${#name_array[@]} -le 1 ]; then
        so_error S1.2.6
        exit 1
    else
        so_success S1.2.6 $email
    fi

    ## S1.2.7. Add user info to the file
    file_line="$username:$nif:$name:$email:$password:0"
    if echo $file_line >>$file; then
        so_success S1.2.7 "$file_line"
    else
        so_error S1.2.7
        exit 1
    fi
}

add_balance() {
    ## S1.3.1. Check if the password is right
    if [ "$(cut -d: -f5 <<<"$file_line")" = "$password" ]; then
        so_success S1.3.1
        ## S1.3.2. Add money to the user
        broken_line="$(cut -d: -f1-5 <<<"$file_line")"
        old_balance=$(cut -d: -f6 <<<"$file_line")
        new_balance=$(($balance + $old_balance))

        if sed -i "s/$file_line/$broken_line:$new_balance/g" $file; then
            so_success S1.3.2 $new_balance
        else
            so_error S1.3.2
            exit 1
        fi
    else
        so_error S1.3.1
    fi
}

## S1.2.3.
# Load user data
load_file() {
    if grep -q "$name" "$file"; then
        file_line=$(grep "$name" "$file" | head -n 1)
        so_success S1.2.3
        add_balance
    else
        so_error S1.2.3
        create_line
        add_balance
    fi
}

## S1.2.1.
# Check if file exist
if [ -f "$file" ]; then
    so_success S1.2.1
    load_file
else
    so_error S1.2.1
    create_file
    load_file
fi

## S1.4.1.
# Creates a file with user information but sorted by balance
if sort -t: -k6 -nr "$file" >"$sorted_file"; then
    so_success S1.4.1
else
    so_error S1.4.1
    exit 1
fi
