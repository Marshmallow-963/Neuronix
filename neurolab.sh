#!/bin/bash
# Pega o diretório onde este script está salvo
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Muda para a pasta bin e executa o programa
cd "$DIR/bin"
./neurolab "$@"
