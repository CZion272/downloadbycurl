var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var binding = require(binding_path);


'use strict'
class Download{
    constructor(){
        this.obj = new binding.Download();
    }
	
	addTask(url, save){
		return this.obj.addTask(url, save);
	}
	
	start(nIndex, func){

		this.obj.start(nIndex, func);
	}
	
	pause(nIndex){
		this.obj.pause(nIndex);
	}
	
	resume(nIndex){
		this.obj.resume(nIndex);
	}
	
	cancel(nIndex){
		this.obj.cancel(nIndex);
	}
	
}

const EventEmitter = require('events').EventEmitter

const emitter = new EventEmitter()

emitter.on('Progress', (index, total, now) => {
	console.log("Progress")
	console.log(index)
	console.log(total)
	console.log(now)
})

emitter.on('Start', (index,error) => {
	console.log("Start")
	console.log(error)
	console.log(index)
})

emitter.on('End', (index,error) => {
	console.log("End")
	console.log(error)
	console.log(index)
})

// obj.start(nIndex, emitter.emit.bind(emitter))
// obj.start(nIndex1, emitter.emit.bind(emitter))
// console.log(nIndex);

binding.runExec();
let obj = new Download();
let nIndex = obj.addTask("https://nodejs.org/dist/v10.15.1/node-v10.15.1-x64.msi", "D:\\test.msi");
let nIndex1 = obj.addTask("https://nodejs.org/dist/v11.10.0/node-v11.10.0-x64.msi", "D:\\test1.msi");

obj.start(nIndex, emitter.emit.bind(emitter))
obj.start(nIndex1, emitter.emit.bind(emitter))


