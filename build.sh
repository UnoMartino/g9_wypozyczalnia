#!/usr/bin/env bash

COMPILER=${CXX:-clang++}

# Jeśli binarka './nob' nie istnieje, to robimy initial build
if [ ! -f ./nob ]; then
    echo "Build system (nob.c) is bootstrapping using $COMPILER"
    $COMPILER nob.c -o nob

    # Jak kompilator wypluje błąd, to ubijamy skrypt
    if [ $? -ne 0 ]; then
        echo "unable to build nob.c"
        exit 1
    fi
fi

# Odpalamy gotowego exeka i przekazujemy mu wszystkie argumenty z linii komend (np. run release)
./nob "$@"
