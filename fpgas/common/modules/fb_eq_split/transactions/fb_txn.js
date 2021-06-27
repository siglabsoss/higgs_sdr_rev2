const _ = require('lodash');
let u = (din) => din >>> 0;
function getRandomInt(min, max) {
    return min + Math.floor(Math.random() * Math.floor(max-min+1));
}

function getRandomFromArray(arr) {
    return this[Math.floor(Math.random() * arr.length)];
}
exports.fb = (options) => {

    let txn = [];
    
    let {type=null, length=null, vectorType=null} = options;
    if(type == null) {
	type = getRandomInt(1,4);
    }
    if(length == null) {
	length = getRandomInt(0,65536);
    }
    if(vectorType == null) {
	vectorType = getRandomInt(0,255);
    }

    for(let i of _.range(length)) {
	if(i==0)
	    txn.push(type);
	else if(i==1)
	    txn.push(length);
	else if(i==4)
	    txn.push(vectorType);
	else
	    txn.push(u(getRandomInt(0,2^32-1)));
    }

    return txn;
}

