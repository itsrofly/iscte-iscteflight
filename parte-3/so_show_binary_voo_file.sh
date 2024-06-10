#!/bin/bash
. /home/so/utils/bin/so_utils.sh

structName="Voo"
echo -e "${COLOUR_FAINT_MAGENTA}This program displays on STDOUT a binary file with one or more elements of type \"$structName\".${COLOUR_NONE}"

# Verify that one binary file was passed as argument
(($# != 1)) && so_error "" "SYNTAX: $0 <binary-file.dat>" && exit

# This script displays on STDOUT the contents of a binary file with one or more elements of type $structName
# This binary file MUST have ALL elements "rows" of the same type $structName.
# In this particular case, the type $structName is defined as:
#
# typedef struct {
#     char nrVoo[8];             // Número do voo escolhido
#     char origem[24];           // Origem do voo
#     char destino[24];          // Destino do voo
#     int  lugares[MAX_SEATS];   // Lista de lugares disponíveis e ocupados do voo
# } Voo;

# Print header row (just for better understanding of the table, this is not part of the binary file)
echo -e "${COLOUR_BACK_BOLD_YELLOW}| nrVoo  | origem  |   destino   | lugares[0]| lugares[1]| lugares[2]| lugares[3]| lugares[4]| lugares[5]| lugares[6]| lugares[7]| lugares[8]| lugares[9]|lugares[10]|${COLOUR_NONE}"

# This tool then shows in the STDOUT the information:
# |  num   |             nome              | nota  |  (field name)
#    int               char[30]              float    (field type in C)
#  4 Bytes             32 Bytes             4 Bytes   (field size in bytes (including any padding bytes))
#    1/4                 1/32                 1/4     (1/field size in bytes)
#    %6d                %-29s                %5.2f    (see printf-style formatting)
# 1/4 "%6d"          1/32 "%-29s"         1/4 "%5.2f" (full field formatting)
#hexdump -e '"|" 1/4 " %6d |" 1/32 " %-29s |" 1/4 " %5.2f |\n"' $1
hexdump -e '"|" 1/8 " %-6s |" 1/24 " %-7s |" 1/24 " %-11s |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |" 1/4 " %-9d |\n"' $1

# This means: Use the application hexdump to read the passed binary file and cycle:
# while (!EOF) {
#   Print "|"
#   Print 1 element that reads 4 Bytes of the file and format it as printf(" %6d |", bytes)
#   Print 1 element that reads 32 Bytes of the file and format it as printf(" %-29s |", bytes). %-29s means a string which has at most 29 chars (+'\0') and is left-justified
#   Print 1 element that reads 4 Bytes of the file and format it as printf(" %5.2f |\n", bytes)
# }