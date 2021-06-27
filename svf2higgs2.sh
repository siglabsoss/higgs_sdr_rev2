#!/usr/bin/env bash

# nvm i 10
# npm i jtag.ftdi

./node_modules/.bin/detach-ftdi

login=FIXME
passwd=FIXME

server=http://${login}:${passwd}@FIXME:8080/jenkins

#latest=ameet_grav3/157-ba892921057f56e1f782df6d1ac36bb3fe40b7f5/fpgas
#latest=ameet_grav3/214-316ce0d9cc07411c556d93734a6fc312b1e103e6/fpgas
#latest=ameet_grav3/221-008d563baf64ffc91cdc55a6194bb8f7f13a47de/fpgas
#latest=ameet_grav3/222-241f9f85f7179bfa6203316f207e5ac238f533ac/fpgas
#latest=ameet_grav3/224-63ff9031a40ca4b0fcaed95cadc84799819ea809/fpgas
#latest=ameet_grav3/203-2802c5fab874f49c38887173c1d99e4a2b3b25d7/fpgas
#latest=ameet_grav3/223-29d20906f50970da55176308d90bbcc001d6ddc5/fpgas
#latest=ameet_grav3/226-08fea61ef4bcffa0619a9a31c888b1c19aea2e91/fpgas
# parity=ameet_grav3/240-a02e8f09c18514db2bcf2e5e2aa57eb5b5b9b643/fpgas



# parity=ameet_grav3/288-97831905eee71d3cc5a5fac1655c1bad551251b2/fpgas
parity=ameet_grav3/287-1555c46a4ba6124851878f50bf621c9044cd664f/fpgas
latest=higgs_sdr_ben_grav3/10-8ee18897ab8e7bb0aba7e6ed4da94a2d83344426/fpgas

for P in cscfg

do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${latest}/grav/${P}/build/${P}_top.svf
    fi
done



for P in cs00

do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${parity}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${parity}/grav/${P}/build/${P}_top.svf
    fi
done


for P in cs01 cs02 cs10 cs11 cs12 cs20 cs22 cs30 cs31 cs32 cs21 cs03

do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${latest}/grav/${P}/build/${P}_top.svf
    fi
done
