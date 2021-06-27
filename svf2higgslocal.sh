#!/usr/bin/env bash

# nvm i 10
# npm i jtag.ftdi

./node_modules/.bin/detach-ftdi

login=FIXME
passwd=FIXME

server=http://${login}:${passwd}@FIXME:8080/jenkins

localpath=bitfiles



for P in cscfg cs00 cs01 cs02 cs10 cs11 cs12 cs20 cs22 cs30 cs31 cs32 cs21 cs03
do
    ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 -f ./${localpath}/${P}_top.svf
    echo ${P} finished
done
