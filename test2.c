reg?
huachen/server
huachen/client
reged?
huachen/server/equipmentId
huachen/client/equipmentId

reg=101 
unreg=102
      103=query                 104

report status =104

	  105=dev control            {ret=  desc=  operatorCode=}
      106=dev sub control   
report data=202

//return by register
{
	"ret":"1",
	"desc":"1",
	"operatorCode":"1",
	"equipmentId":"1",	
	"equipmentNo":"1"
}



//device report
{
	"operateCode":"104",
	"equipmentNo":"boatsfsdf",
	"equipmentName":"huachenboat1",
	"equipmentType":"2",
	"company":"huachenjinhua" ,
	"equipmentStatus":"1",
	"equipmentViewData":"",
	"userId":"",
	"childEquipmentData":[
	{"childEquipmentNo":"B1",
	 "childEquipmentOpValue":"0"
	},
	{"childEquipmentNo":"B1",
	 "childEquipmentOpValue":"0"
	}]
}

//control sub device
{
	"operateCode":"106",
	"equipmentNo":"boatsfsdf",
	"childEquipmentNo":"boatsfsdf",
	"childEquipmentOpValue":"0",
}

// data to platform
{
	"operateCode":"202",
	"equipmentNo":"boatsfsdf",
	"equipmentName":"BoatRobot",
	"temperature":"20",
	"weather":"0" ,
	"remarks":"1",
	"locationName":"X1",
	"longitude":"120.00",
	"latitude":"35.22",
	"monitorData":[
	{"quotaName":"pH",
	 "position":"0",
	 "quataValue":"7.0" 	
	},
	{"quotaName":"COD",
	 "position":"0",
	 "quataValue":"0.5" 	
	}]
}