(function () {
    'use strict';
	var $log, $interval, $timeout, $scope, wss, ks;
	// constants
	var dataReq = 'ControllerDataRequest',
		dataResp = 'ControllerDataResponse';
    // constants
    var refreshInterval = 2000;
	var refreshPromise=null;
	angular.module('ovControllermgr', [])
		.controller('OvControllermgrCtrl', 
		[ '$log', '$interval', '$timeout', '$scope', 'WebSocketService', 'KeyService',
		function (_$log_, _$interval_, _$timeout_, _$scope_, _wss_, _ks_) { 
			$log = _$log_;
			$scope = _$scope_;
			$interval = _$interval_;
			$timeout = _$timeout_;
			wss = _wss_;
			ks = _ks_;
			
			function respDataCb(data) {
				for(var i=0;i<data.controlmgs.length;i++)
				{
					data.controlmgs[i]["controllerurl"]="http://"+data.controlmgs[i]["controllerip"]+":"+data.controlmgs[i]["controllerport"]+"/onos/ui/login.html";
				}
				$log.log(data);
				$scope.data = data;
				$scope.$apply();
			}
			
			function getData() {
				wss.sendEvent(dataReq, {
					requestype:"getInfo"
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
			
			function CheckData() {
				var exp = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
				
				if(($scope.data.controllerip=="") || ($scope.data.controllerip == null))
				{
					alert("请输入控制器地址");
					return false;
				}
				var reg = $scope.data.controllerip.match(exp);
				if (reg == null)
				{
					alert("请输入正确的控制器地址");
					$("#controllerip").val('');
					return false;
				}
				
				if(($scope.data.controllerport=="") || ($scope.data.controllerport == null))
				{
					alert("请输入控制器端口");
					return false;
				}
				if (!isMatch($scope.data.controllerport))
				{
					alert("请输入正确的控制器端口");
					$("#controllerport").val('');
					return false;
				}
				
				if(($scope.data.controllerusername=="") || ($scope.data.controllerusername == null))
				{
					alert("请输入控制器用户名");
					return false;
				}
				
				if(($scope.data.controllerpassword=="") || ($scope.data.controllerpassword == null))
				{
					alert("请输入控制器用户名");
					return false;
				}
				return true;
			}
			$scope.CheckData = CheckData;
			
			function DeleteData() {
				var id = $('input[name="checking"]:checked').val();
				$log.log("id="+id);
				if (id < 0)
				{
					alert("请选择要删除的信息");
					return;
				}
				id=""+id;
				wss.sendEvent(dataReq, {
					requestype:"deleteInfo",
					controllerid:id
                });
				//getData();
			}
			$scope.DeleteData=DeleteData;
			
			function PostData() {
				var result = CheckData();
				if (result == false)
				{
					return;
				}
				wss.sendEvent(dataReq, {
					requestype:"addInfo",
					controllerip: $scope.data.controllerip,
					controllerport:$scope.data.controllerport,
					controllerusername:$scope.data.controllerusername,
					controllerpassword:$scope.data.controllerpassword
                });
				//getData();
			}
			$scope.PostData=PostData;
			
			var handlers = {};
			$scope.data = {};
			handlers[dataResp] = respDataCb;
			wss.bindHandlers(handlers);
			
			function fetchDataIfNotWaiting() {
				getData();
			}
			
			function startRefresh() {
				refreshPromise = $interval(fetchDataIfNotWaiting, refreshInterval);
			}
			//startRefresh();

			function stopRefresh() {
				if (refreshPromise) {
					$interval.cancel(refreshPromise);
					refreshPromise = null;
				}
			}

			// cleanup
			$scope.$on('$destroy', function () {
				wss.unbindHandlers(handlers);
				stopRefresh();
				$log.log('OvControllermgrCtrl has been destroyed');
			});

		}]);
}());