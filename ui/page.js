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
	    console.log("check"); 
	    if(this.seen) return "hide"
	    var new_columns =[];
	    for(var i = 0; i < 20; i++){
		var col = {}
		col.index = i;
		col.values = []
		for(var j = 0; j < 5; j++){
		    col.values[j] = {index: j, value: (j + 1) * (i + 1)}
		    if(j == 3 && i < 5)
			col.values[j].value = null;
		}
		new_columns[i] =col;
	    }
	    console.log(new_columns)
	    this.columns = new_columns;
	    return "show"
	},
	getMessage2: function(){
	    console.log("check2"); 
	    return this.message;
	},
	moveFocusRight: function(thing){
	    this.focus_cell.x = this.focus_cell.x + 1;
	}
	    
    }
})
