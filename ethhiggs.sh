#!/usr/bin/env bash

# nvm i 10
# npm i jtag.ftdi

./node_modules/.bin/detach-ftdi

login=FIXME
passwd=FIXME

server=http://${login}:${passwd}@FIXME:8080/jenkins
latest=higgs_sdr_master/196-6da5aea97f5f3c81ac68909baad9f81dd2083429/fpgas

for P in \
eth
do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${latest}/grav/${P}/build/${P}_top.svf
    fi
done
