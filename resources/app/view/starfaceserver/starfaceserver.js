
(function () {
    'use strict';
	var $log, $interval, $timeout, $scope, wss, ks,ps,fs,is;
	// constants
	var dataReq = 'WebConfigDataRequest',
		dataResp = 'WebConfigDataResponse';
    // internal state
    var detailsPanel,
        pStartY,
        pHeight,
        top,
        topTable,
        bottom,
        iconDiv,
        nameDiv,
        wSize;


    // constants
    var topPdg = 28,
        ctnrPdg = 24,
        scrollSize = 17,
        portsTblPdg = 50,
        htPdg = 479,
        wtPdg = 532,

        pName = 'cluster-details-panel',
        detailsReq = 'clusterDetailsRequest',
        detailsResp = 'clusterDetailsResponse',

        propOrder = [
            'id', 'ip'
        ],
        friendlyProps = [
            'Node ID', 'IP Address'
        ],
        deviceCols = [
            'id', 'type', 'chassisid', 'mfr',
            'hw', 'sw', 'protocol', 'serial'
        ],
        friendlyDeviceCols = [
            'URI', 'Type', 'Chassis ID', 'Vendor',
            'H/W Version', 'S/W Version', 'Protocol',
            'Serial #'
        ];
    function closePanel() {
        if (detailsPanel.isVisible()) {
            $scope.selId = null;
            detailsPanel.hide();
            return true;
        }
        return false;
    }

    function addCloseBtn(div) {
        is.loadEmbeddedIcon(div, 'close', 20);
        div.on('click', closePanel);
    }

    function handleEscape() {
        return closePanel();
    }

    function setUpPanel() {
        var container, closeBtn, tblDiv;
        detailsPanel.empty();

        container = detailsPanel.append('div').classed('web-container', true);

        top = container.append('div').classed('top', true);
        closeBtn = top.append('div').classed('close-btn', true);
        addCloseBtn(closeBtn);
        iconDiv = top.append('div').classed('dev-icon', true);
        top.append('h2');
        topTable = top.append('div').classed('top-content', true)
            .append('table');
        top.append('hr');

        bottom = container.append('div').classed('bottom', true);
        bottom.append('h2').classed('devices-title', true).html('Devices');
        bottom.append('table');
        //ToDo add more details
    }

    function addProp(tbody, index, value) {
        var tr = tbody.append('tr');

        function addCell(cls, txt) {
            tr.append('td').attr('class', cls).html(txt);
        }
        addCell('label', friendlyProps[index] + ' :');
        addCell('value', value);
    }

    function populateTop(details) {
        is.loadEmbeddedIcon(iconDiv, 'node', 40);
        top.select('h2').html(details.id);

        var tbody = topTable.append('tbody');

        propOrder.forEach(function (prop, i) {
            addProp(tbody, i, details[prop]);
        });
    }

    function addDeviceRow(tbody, device) {
        var tr = tbody.append('tr');

        deviceCols.forEach(function (col) {
            tr.append('td').html(device[col]);
        });
    }

    function populateBottom(devices) {
        var table = bottom.select('table'),
            theader = table.append('thead').append('tr'),
            tbody = table.append('tbody'),
            tbWidth, tbHeight;

        friendlyDeviceCols.forEach(function (col) {
            theader.append('th').html(col);
        });
        devices.forEach(function (device) {
            addDeviceRow(tbody, device);
        });

        tbWidth = fs.noPxStyle(tbody, 'width') + scrollSize;
        tbHeight = pHeight
                    - (fs.noPxStyle(detailsPanel.el()
                                        .select('.top'), 'height')
                    + fs.noPxStyle(detailsPanel.el()
                                        .select('.devices-title'), 'height')
                    + portsTblPdg);

        table.style({
            height: tbHeight + 'px',
            width: tbWidth + 'px',
            overflow: 'auto',
            display: 'block'
        });

        detailsPanel.width(tbWidth + ctnrPdg);
    }

    function createDetailsPane() {
        detailsPanel = ps.createPanel(pName, {
            width: wSize.width,
            margin: 0,
            hideMargin: 0
        });
        detailsPanel.el().style({
            position: 'absolute',
            top: pStartY + 'px'
        });
        $scope.hidePanel = function () { detailsPanel.hide(); };
        detailsPanel.hide();
    }

    function populateDetails(details) {
        setUpPanel();

        //populateTop(details);
        //populateBottom(details.devices);

        //ToDo add more details
        detailsPanel.height(632);
        detailsPanel.width(465); //ToDO Use this when needed!
    }

    function respDetailsCb(data) {
        $scope.panelData = data.details;
        $scope.$apply();
    }
	function initPanel() {
		heightCalc();
		createDetailsPane();
	}

	angular.module('ovStarfaceserver', [])
		.controller('OvStarfaceserverCtrl', 
		[ '$log', '$interval', '$timeout', '$scope', 'IconService','WebSocketService', 'KeyService','PanelService',
		'FnService',
		function (_$log_, _$interval_, _$timeout_, _$scope_,_is_, _wss_, _ks_, _ps_,_fs_) { 
			$log = _$log_;
			$scope = _$scope_;
			$interval = _$interval_;
			$timeout = _$timeout_;
			is = _is_;
			wss = _wss_;
			ks = _ks_;
			ps = _ps_;
			fs = _fs_;
            $scope.panelData = {};
			
			function respDataCb(data) {
				$scope.data = data;
				$log.log(data);
				var controleriparray=[];
				var controleripallarray=data.controlerips;
				var controleripvalue;
				if(controleripallarray.length>0)
				{
					controleripvalue=controleripallarray[0];
					var num=0;
					for(var i=1;i<controleripallarray.length;i++)
					{
						if(controleripallarray[i]==controleripvalue)
						{
							num++;
							continue;
						}
					}
					
					if(num==0)
					{
						controleripvalue="auto";
						for(var i=1;i<controleripallarray.length;i++)
						{
							controleriparray.push(controleripallarray[i]);
						}
					}
					else
					{
						controleriparray.push("auto")
						for(var i=1;i<controleripallarray.length;i++)
						{
							if(controleripallarray[i]==controleripvalue)
							{
								continue;
							}
							controleriparray.push(controleripallarray[i]);
						}
					}
				}
				$scope.controlerip_value=controleripvalue;
				$scope.controlerip_array=controleriparray;
				//$scope.onosurl="http://"+controleripvalue+":"+controlerportvalue+"/onos/ui/login.html";
				$scope.starosurl="http://"+data.webip+":"+data.webhttpport+"/starface/ui/login.html";
				$scope.staroshttpsurl="https://"+data.webip+":"+data.webhttpsport+"/starface/ui/login.html";
				console.log($scope.staroshttpsurl);
				$scope.$apply();
				if ($scope.data.result=="set config success")
				{
					alert("set config success");
				}
				if ($scope.data.result=="set config failure")
				{
					alert("set config failure");
				}
			}
			
			function getData() {
				wss.sendEvent(dataReq, {
					requestype:"getconfig"
                });
			}
			$scope.getData = getData;
			getData();
			
			function isMatch(number)
			{
				if(isNaN(number))
				{
					$log.log("isNaN");
					return false;
				}
				if(number.slice(0,1)=="0")
				{
					$log.log("slice");
					return false;
				}
				if(parseInt(number)>65535)
				{
					$log.log("parseInt");
					return false;
				}
				return true;
			}
			
			function CheckData(webip,webhttpport,webhttpsport,controllerip) {
				var exp = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
				console.log("CheckData1");
				if(webip=="")
				{
					alert("请输入Web地址");
					return false;
				}
				var reg = webip.match(exp);
				if(reg == null) {
					alert("请输入正确的Web地址");
					$("#webip").val('');
					return false;
				}
				
				if(webhttpport=="")
				{
					alert("请输入Web服务器HTTP端口");
					return false;
				}
				if (!isMatch(webhttpport))
				{
					alert("请输入Web服务器HTTP端口");
					$("#webhttpport").val('');
					return false;
				}
				
				if(webhttpsport=="")
				{
					alert("请输入Web服务器HTTPS端口");
					return false;
				}
				if (!isMatch(webhttpsport))
				{
					alert("请输入Web服务器HTTPS端口");
					$("#webhttpsport").val('');
					return false;
				}
				
				if(controllerip=="")
				{
					alert("请输入控制器地址");
					return false;
				}
				return true;
			}
			$scope.CheckData = CheckData;	
			/*$scope.uploadAndSubmit =uploadAndSubmit;
			 function uploadAndSubmit() { 
				 //var form = document.forms["demoForm"]; 
					
				 if (document.getElementById("cerfile").files.length > 0&&document.getElementById("keyfile").files.length >0) { 
					 // 寻找表单域中的 <input type="file" ... /> 标签
					 var file = document.getElementById("cerfile").files[0]; 
					 var file2 =document.getElementById("keyfile").files[0]; 
					 // try sending 
					 var reader = new FileReader(); 
					 reader.onloadstart = function() { 
						 // 这个事件在读取开始时触发
						 console.log("onloadstart"); 
						 //document.getElementById("bytesTotal").textContent = file.size; 
					 } 
					 reader.onprogress = function(p) { 
						 // 这个事件在读取进行中定时触发
						 console.log("onprogress"); 
						 //document.getElementById("bytesRead").textContent = p.loaded; 
					 } 

					 reader.onload = function() { 
							// 这个事件在读取成功结束后触发
						 console.log("load complete"); 
					 } 
					
					 reader.onloadend = function() { 
							// 这个事件在读取结束后，无论成功或者失败都会触发
						 if (reader.error) { 
							console.log(reader.error); 
						 } else { 
							 //document.getElementById("bytesRead").textContent = file.size; 
							wss.sendEvent(dataReq, {
								requestype:"setconfig",
								webip: $scope.data.webip,
								webport:$scope.data.webport,
								controlerip: $("#controlerip").find("option:selected").val(),
								cerfile: reader.result,
								keyfile: reader.result,
							});
						 } 
					 } 
					  reader.readAsBinaryString(file,file2); 
				 } else { 
					alert ("Please choose a file."); 
				 } 
			 }*/
			
			 function uploadcerfile() { 
				 //var form = document.forms["demoForm"]; 
					
				 if (document.getElementById("cerfile").files.length > 0) { 
					 // 寻找表单域中的 <input type="file" ... /> 标签
					 var file = document.getElementById("cerfile").files[0]; 
					 // try sending 
					 var reader = new FileReader(); 
					 reader.onloadstart = function() { 
						 // 这个事件在读取开始时触发
						 console.log("onloadstart"); 
						 //document.getElementById("bytesTotal").textContent = file.size; 
					 } 
					 reader.onprogress = function(p) { 
						 // 这个事件在读取进行中定时触发
						 console.log("onprogress"); 
						 //document.getElementById("bytesRead").textContent = p.loaded; 
			 }
					 
					 reader.onload = function() { 
							// 这个事件在读取成功结束后触发
						 console.log("load complete"); 
					 } 
			
					 reader.onloadend = function() { 
							// 这个事件在读取结束后，无论成功或者失败都会触发
						 if (reader.error) { 
							console.log(reader.error); 
						 } else { 
							 //document.getElementById("bytesRead").textContent = file.size; 
							 uploadkeyfile(reader);
							 console.log("webip!!"+$scope.data.webip);
						 } 
					 } 
					  reader.readAsBinaryString(file); 
				 } else { 
					alert ("Please choose a file."); 
				 } 
			 }
			  $scope.uploadcerfile =uploadcerfile;
				function uploadkeyfile(reader) { 
				 //var form = document.forms["demoForm"]; 	
				 if (document.getElementById("keyfile").files.length > 0) { 
					 // 寻找表单域中的 <input type="file" ... /> 标签
					 var file2 =document.getElementById("keyfile").files[0]; 
					 // try sending 
					 var reader1 = new FileReader(); 
					 reader1.onloadstart = function() { 
						 // 这个事件在读取开始时触发
						 console.log("onloadstart"); 
						 //document.getElementById("bytesTotal").textContent = file.size; 
					 } 
					 reader1.onprogress = function(p) { 
						 // 这个事件在读取进行中定时触发
						 console.log("onprogress"); 
						 //document.getElementById("bytesRead").textContent = p.loaded; 
					 } 

					 reader1.onload = function() { 
							// 这个事件在读取成功结束后触发
						 console.log("load complete"); 
					 } 
					
					 reader1.onloadend = function() { 
							// 这个事件在读取结束后，无论成功或者失败都会触发
						 if (reader1.error) { 
							console.log(reader1.error); 
						 } else { 
							console.log("webip"+$scope.data.webip);
							console.log("webhttpport:"+$scope.data.webhttpport);
							console.log("webhttpsport:"+$scope.data.webhttpsport);
							 //document.getElementById("bytesRead").textContent = file.size; 
							wss.sendEvent(dataReq, {
								requestype:"setconfig",
								webip: $scope.data.webip,
								webhttpport:$scope.data.webhttpport,
								webhttpsport:$scope.data.webhttpsport,
								controlerip: $("#controlerip").find("option:selected").val(),
								cerfile: reader.result,
								keyfile: reader1.result,
							});
						 } 
					 } 
					  reader1.readAsBinaryString(file2); 
				 } else { 
					alert ("Please choose a file2."); 
				 } 
			 }
			 $scope.uploadkeyfile =uploadkeyfile;
			function ConfigData() {
				var controlerip_val=$("#controlerip").find("option:selected").val();
				var result = CheckData($scope.data.webip, $scope.data.webhttpport,$scope.data.webhttpsport, controlerip_val);
				if (result == false)
				{
					console.log("result:false");
					return;
				}
				if (document.getElementById("cerfile").files.length > 0&&document.getElementById("keyfile").files.length > 0)
				{
					console.log("uploadfile()");
					
					uploadcerfile();
				}
				else
				{
							wss.sendEvent(dataReq, {
								requestype:"setconfig",
								webip: $scope.data.webip,
								webhttpport:$scope.data.webhttpport,
								webhttpsport:$scope.data.webhttpsport,
								controlerip: $("#controlerip").find("option:selected").val(),
								cerfile: "",
								keyfile: "",
							});	
				}
			}
			$scope.ConfigData = ConfigData;
			
			var handlers = {};
			$scope.data = {};
			handlers[dataResp] = respDataCb;
			wss.bindHandlers(handlers);
			
			// cleanup
			$scope.$on('$destroy', function () {
				wss.unbindHandlers(handlers);
				$log.log('OvStarfaceserverCtrl has been destroyed');
			});

			
		}])
		.directive('triggerForm', function () {
			return {
            restrict: 'A',
            link: function (scope, elem) {
                elem.bind('click', function () {
                    document.getElementById('cerfile')
                        .dispatchEvent(new MouseEvent('click'));
                });
            }
        };
    })
		.directive('triggerForm', function () {
			return {
            restrict: 'A',
            link: function (scope, elem) {
                elem.bind('click', function () {
                    document.getElementById('keyfile')
                        .dispatchEvent(new MouseEvent('click'));
                });
            }
        };
    })

    // binds the model file to the scope in scope.appFile
    // sends upload request to the server
    .directive('fileModel', ['$parse',
        function ($parse) {
            return {
                restrict: 'A',
                link: function (scope, elem, attrs) {
                    var model = $parse(attrs.fileModel),
                        modelSetter = model.assign;

                    elem.bind('change', function () {
                        scope.$apply(function () {
                            modelSetter(scope, elem[0].files[0]);
                        });
                        scope.$emit('FileChanged');
                    });
                }
            };
        }])
    .directive('clusterDetailsPanel',
    ['$rootScope', '$window', '$timeout', 'KeyService',
    function ($rootScope, $window, $timeout, ks) {
        return function (scope) {
            var unbindWatch;

            function heightCalc() {
                pStartY = fs.noPxStyle(d3.select('.tab-content'), 'height')
                                        + 0 + topPdg;
                wSize = fs.windowSize(pStartY);
                pHeight = wSize.height;
            }

            function initPanel() {
                heightCalc();
                createDetailsPane();
            }

            // Safari has a bug where it renders the fixed-layout table wrong
            // if you ask for the window's size too early
            if (scope.onos.browser === 'safari') {
                $timeout(initPanel);
            } else {
                initPanel();
            }
			scope.panelData
			populateDetails(scope.panelData);
			detailsPanel.show();
            // create key bindings to handle panel
            ks.keyBindings({
                esc: [handleEscape, 'Close the details panel'],
                _helpFormat: ['esc']
            });
            ks.gestureNotes([
                ['click', 'Select a row to show cluster node details'],
                ['scroll down', 'See available cluster nodes']
            ]);
            // if the panelData changes
            scope.$watch('panelData', function () {
                if (!fs.isEmptyObject(scope.panelData)) {
                    populateDetails(scope.panelData);
                    detailsPanel.show();
                }
            });

            // if the window size changes
            unbindWatch = $rootScope.$watchCollection(
                function () {
                    return {
                        h: $window.innerHeight,
                        w: $window.innerWidth
                    };
                }, function () {
                    if (!fs.isEmptyObject(scope.panelData)) {
                        heightCalc();
                        populateDetails(scope.panelData);
                    }
                }
            );

            scope.$on('$destroy', function () {
                unbindWatch();
                ps.destroyPanel(pName);
            });
        };
    }]);
	
}());