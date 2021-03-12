#!/bin/sh

sv_stop() {
    for s in $(ls -d /service/*)
    do
        sv stop $s
    done
}

trap "sv_stop; exit" SIGTERM
runsvdir /service &
wait
