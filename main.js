var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var binding = require(binding_path);

exports.runExec = function() {
    return binding.runExec();
}

class Download {
    constructor() {
        this.obj = new binding.Download();
    }
	
	addTask(url, save) {
		return this.obj.addTask(url, save);
	}
	
	start(nIndex, func) {
		return this.obj.start(nIndex, func);
	}
	
	pause(nIndex) {
		return this.obj.pause(nIndex);
	}
	
	resume(nIndex) {
		return this.obj.resume(nIndex);
	}
	
	cancel(nIndex) {
		return this.obj.cancel(nIndex);
	}
    
    remove(nIndex) {
        return this.obj.remove(nIndex);
    }

}
