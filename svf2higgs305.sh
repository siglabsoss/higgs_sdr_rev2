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
latest=ameet_grav3/240-a02e8f09c18514db2bcf2e5e2aa57eb5b5b9b643/fpgas
latest=ameet_grav3/295-c980b5276536ae8e56bc22b42675685c03cb16d1/fpgas

# thanos=ameet_grav3/301-0adc60a06f53fea4a11e616a0d845edbe74b37c5/fpgas
thanos=ameet_grav3/299-301581d7521965e01133f5907390f433b57cd8bb/fpgas
thanos=ameet_grav3/305-46effafbfaf8c4058131b70833f2211391b19e8b/fpgas


for P in \
cscfg cs00 cs01 cs02

do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${latest}/grav/${P}/build/${P}_top.svf
    fi
done



for P in cs10

do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${thanos}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${thanos}/grav/${P}/build/${P}_top.svf
    fi
done



for P in \
cs11 cs12 cs20 cs22 cs30 cs31 cs32 cs21 cs03

do
    if [[ ${P:0:2} == "cs" || ${P:0:2} == "cc" ]] ; then
        ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/cs/${P}/build/${P}_top.svf
    else
        ./node_modules/.bin/svf2ftdi -n 0 -d 2 --freq 5E6 --url ${server}/${latest}/grav/${P}/build/${P}_top.svf
    fi
done

