var ws = new WebSocket('ws://localhost:9000/');

var app = new Vue({
  el: '#app',
  data: {
      message: 'Hello Vue3!',
      seen: true,
      columns: [["print","+",3],[null,null,4,null],[7,8,9,1]],
      focus_cell:{x: 0, y: 0}
      
      
  },
    methods:{
	toggleSeen: function(){
	    this.seen = !this.seen;
	},
	getMessage: function(){
	    if(this.seen) return "run"
	    return "stop"
	},
	getMessage2: function(){
	    return this.message;
	},
	takeFocus: function(value){
	    this.focus_cell.x = value.index;
	    this.focus_cell.y = value.col.index;
	},
	getFocusedNode: function(evt){

	    var td = evt.currentTarget.childNodes[this.focus_cell.y].childNodes[this.focus_cell.x]
	    return td;
	},
	checkFocus: function(evt){
	    var control = this.getFocusedNode(evt).firstChild.firstChild
	    if(control != document.activeElement)
		control.focus()

	    //	    evt.preventDefault();
	},
	moveFocusRight: function(thing){
	    if(!thing.ctrlKey) return;
	    var dx = 0;
	    var dy = 0;
	    if(thing.key == "ArrowDown")
		dy = dy + 1;
	    if(thing.key == "ArrowUp")
		dy = dy - 1;
	    if(thing.key == "ArrowLeft")
		dx = dx - 1;
	    if(thing.key == "ArrowRight")
		dx = dx + 1;
	    
	    this.focus_cell.x = this.focus_cell.x + dx;
	    this.focus_cell.y = this.focus_cell.y + dy;
	    var td = this.getFocusedNode(thing);
	    //td.firstChild.firstChild.focus()
	    
	    thing.currentTarget.firstChild.focus()
	    thing.preventDefault()
	},
	load: function (matrix){
	    var new_columns =[];
	    for(var i = 0; i < matrix.length; i++){
		var d = matrix[i];
		var col = {
		    app: this,
		}
		col.index = i;
		col.values = []
		for(var j = 0; j < d.length; j++){
		    col.values[j] = {
			index: j,
			value: d[j],
			col: col,
			selected: function(){
			    //console.log(this.app)
			    return this.col.app.focus_cell.y == this.col.index && this.col.app.focus_cell.x == this.index;
			}

		    }
		}
		new_columns[i] =col;
	    }
	    console.log(new_columns)
	    this.columns = new_columns;
	},
	loadBigTalk: function(){
	    ws.send("get_meta\n");
	},
	updateTable
	: function(){
	    var mat = app_update_scope();
	    this.load(mat)
	},
	getSubs: function(subs){
	    ws.send("get_sub:" + subs.join(","));
	},
	
	process: function(){
	    console.log("hej")
	    ws.send("hej");
	}
	    
    }
})

app.load([[1,2,3],[4,5,6]]);
var names = {};
var scope = {};
var meta = {};
var matrix = [];
function arrange_calls(row, column, id){
    while(matrix.length <= row)
	matrix.push([]);
    while(matrix[row].length <= column)
	matrix[row].push("");
    if(scope[id].type == meta.name_type.id){
	console.log("Found name type!");
    }
    console.log(id);
    console.log("NAME(ID): " + scope[id].value + " " + id + "?" + scope[id].type + " " + meta.name_type.id);


    if(scope[id].type == meta.cons_type.id && scope[id].value != "0"){
	row = arrange_calls(row, column + 1, scope[id].value);
	while(matrix.length <= row)
	    matrix.push([]);
	while(matrix[row].length <= column)
	    matrix[row].push("");
    }
    matrix[row][column] = scope[id].value;
    
    while(matrix.length <= row)
	matrix.push([]);
    while(matrix[row].length <= column)
	matrix[row].push("");
    
    matrix[row][column] = scope[id].value;
    var next = scope[id].next;
    
    column += 1;
    while(next != "0"){
	while(matrix.length <= row)
	    matrix.push([]);
	while(matrix[row].length <= column)
	    matrix[row].push("");
	matrix[row][column] = scope[next].value;


	if(scope[next].type == meta.cons_type.id){
	    row = arrange_calls(row, column, scope[next].value);
	}

	
	row += 1
	next = scope[next].next;
    }
    
    return row; 
}

function app_update_scope(){
    console.log(meta);
    console.log(scope);
    matrix = []
    arrange_calls(0,0,meta.root.id);
    return matrix;
}


function handle_message(obj){
    console.log(obj)    
    if(obj.call == "get_meta"){
	meta[obj.name] = obj;
	console.log("Got meta..\n");
	if(obj.name == "root"){
	    ws.send("get_sub " + obj.id);
	}
    }
    if(obj.call == "get"){
	delete obj.call;
	scope[obj.id] = obj;
	if(obj.type == meta.cons_type.id){
	    ws.send("get_sub " + obj.value);	    
	}
	if(obj.next != "0"){
	    ws.send("get_sub " + obj.next);
	}
	//app_update_scope();
    }
    
}

ws.onmessage = function(event) {
    
    console.log('response:\'' + event.data +'\'');
    
    if(event.data.indexOf("{") == 0){
	var json = 0;
	try{
	    json = JSON.parse(event.data);
	 
	}catch(err){
	    console.log('error' + err);
	}
	   handle_message(json);
    }
};

