#!/usr/bin/env bash

# nvm i 10
# npm i jtag.ftdi

./node_modules/.bin/detach-ftdi

login=FIXME
passwd=FIXME

server=http://${login}:${passwd}@FIXME:8080/jenkins


latest=higgs_sdr_ben_d_engine/13-1e7cbd4205481485cd930e25dbf8c3bf4aad06fe

# biteth=higgs_sdr_ben_grav3/241-e53e837e9ff0202937246dab5c1c0e23fdfd568d    # fix tx stall remove hack
# biteth=higgs_sdr_ben_grav3/290-a90a8148fb2983aab5910e1617085614defed1e7    # split join stall (mapmov last)
# biteth=higgs_sdr_ben_grav3/296-fd5a21853cbba13367d2627e6588d4442a56ca14    # join drops looped eq feedback bus - bugged
# biteth=higgs_sdr_ben_grav3/297-14ea8733985cb1d87d5481e406d032da0df6ef98    # shorter drop time - bugged
# biteth=higgs_sdr_ben_grav3/298-f0f78c9fb8a891db580b914fff709bf335603a10    # fix eth memory - bugged
# biteth=higgs_sdr_ben_grav3/300-141b5aeae75c7ac0afe976bfe3fde1379839d580    # buffer between split/join - bugged
biteth=higgs_sdr_ben_grav3/302-c6b53f646cf933c87b2def68882fae0faa788f5b    # second fix for 114
# biteth=higgs_sdr_ben_grav3/305-da374d01114f1f7d14a3cfa4439af612fb24e290    # 32k buffer

bitcs20=higgs_sdr_ben_grav3/294-ca02d7a06812dfd41041c0ad1677b3f93bc8bcec # cs20 2k buffer


for P in cscfg cs00 cs01 cs02
do
    ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/fpgas/cs/${P}/build/${P}_top.svf
    echo ${P} finished
done


for P in cs10
do
    ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${biteth}/fpgas/cs/${P}/build/${P}_top.svf
    echo ${P} finished
done

for P in cs11 cs12
do
    ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/fpgas/cs/${P}/build/${P}_top.svf
    echo ${P} finished
done


for P in cs20
do
    ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${bitcs20}/fpgas/cs/${P}/build/${P}_top.svf
    echo ${P} finished
done

for P in cs22 cs30 cs31 cs32 cs21 cs03
do
    ./node_modules/.bin/svf2ftdi -n 1 -d 2 --freq 5E6 --url ${server}/${latest}/fpgas/cs/${P}/build/${P}_top.svf
    echo ${P} finished
done

