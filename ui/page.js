var ws = new WebSocket('ws://localhost:8888/');

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
	    var td = thing.currentTarget.childNodes[this.focus_cell.y].childNodes[this.focus_cell.x]
	    //td.firstChild.firstChild.focus()
	    thing.currentTarget.firstChild.focus()
	    thing.preventDefault()
	},
	load: function (){
	    var new_columns =[];
	    for(var i = 0; i < 20; i++){
		var col = {
		    app: this,
		}
		col.index = i;
		col.values = []
		for(var j = 0; j < 5; j++){
		    col.values[j] = {
			index: j,
			value: (j + 1) * (i + 1),
			col: col,
			selected: function(){
			    //console.log(this.app)
			    return this.col.app.focus_cell.y == this.col.index && this.col.app.focus_cell.x == this.index;
			}

		    }
		    if(j == 3 && i < 5)
			col.values[j].value = null;
		}
		new_columns[i] =col;
	    }
	    console.log(new_columns)
	    this.columns = new_columns;
	},
	loadBigTalk: function(){
	    ws.send("get_meta\n");
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

app.load();


ws.onmessage = function(event) {
    
    console.log('response:\'' + event.data +'\'');
    
    if(event.data.indexOf("{") == 0){
	try{
	console.log(JSON.parse(event.data))
    }catch(err){
	console.log('error' + err);
    }
    }
};

