p1="fpgas/cs/cs00/build"

#ls -R $p1/tmp/* | grep cs00_top_impl1.mrp


echo "there are"
find $p1 | wc -l
echo "files"


find $p1 | grep cs00_top_impl1.mrp
find $p1 | grep synlog/report
find $p1 | grep automake.log

