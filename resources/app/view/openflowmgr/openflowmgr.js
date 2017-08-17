
(function () {
    'use strict';
	var $log, $interval, $timeout, $scope, wss, ks,ps,fs,is;
	// constants
	var dataReq = 'OpenflowMgrDataRequest',
		dataResp = 'OpenflowMgrDataResponse';
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

	angular.module('ovOpenflowmgr', [])
		.controller('OvOpenflowmgrCtrl', 
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
				var protocolArray=["TCP","SSL"];
				var protocoltypeArray=new Array();
				for(var i=0;i<protocolArray.length;i++){
					if(data.openflowtype==protocolArray[i]){
						continue;
					}
					protocoltypeArray.push(protocolArray[i]);
				}
				$scope.default_value=data.openflowtype;
				$scope.protocoltype_array=protocoltypeArray;
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
			
			function CheckData(openflowip,openflowport) {
				var exp = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
				console.log("CheckData");
				if(openflowip=="")
				{
					alert("请输入openflow监听地址");
					return false;
				}
				var reg = openflowip.match(exp);
				if(reg == null) {
					alert("请输入正确的openflow监听地址");
					$("#openflowip").val('');
					return false;
				}
				
				if(openflowport=="")
				{
					alert("请输入openflow监听端口");
					return false;
				}
				if (!isMatch(openflowport))
				{
					alert("请输入openflow监听端口");
					$("#openflowport").val('');
					return false;
				}
			}
			$scope.CheckData = CheckData;	

			function ConfigData() {
				
				var result = CheckData($scope.data.openflowip, $scope.data.openflowport);
				if (result == false)
				{
					return;
				}
				wss.sendEvent(dataReq, {
					requestype:"setconfig",
                    openflowip: $scope.data.openflowip,
					openflowport:$scope.data.openflowport,
					openflowtype: $("#openflowtype").find("option:selected").val()
                });
			}
			$scope.ConfigData = ConfigData;
			
			var handlers = {};
			$scope.data = {};
			handlers[dataResp] = respDataCb;
			wss.bindHandlers(handlers);
			
			$scope.$on('$destroy', function () {
				wss.unbindHandlers(handlers);
				$log.log('OvOpenflowmgrCtrl has been destroyed');
			});

			
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