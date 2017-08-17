(function () {
    'use strict';
	var $log, $interval, $timeout, $scope, wss, ks;
	// constants
	var dataReq = 'StarfaceClientDataRequest',
		dataResp = 'StarfaceClientDataResponse';
    // constants
    var refreshInterval = 2000;
	var refreshPromise=null;
	angular.module('ovStarfaceserverclient', [])
		.controller('OvStarfaceserverclientCtrl', 
		[ '$log', '$interval', '$timeout', '$scope', 'WebSocketService', 'KeyService',
		function (_$log_, _$interval_, _$timeout_, _$scope_, _wss_, _ks_) { 
			$log = _$log_;
			$scope = _$scope_;
			$interval = _$interval_;
			$timeout = _$timeout_;
			wss = _wss_;
			ks = _ks_;
			function respDataCb(data) {
				$scope.data = data;
				$scope.$apply();
			}

			function getData() {
				wss.sendEvent(dataReq, {
					msgtype: "getdata",
                    deviceid: $scope.data.deviceid,
                });
			}  
			$scope.getData = getData;

			var handlers = {};

			$scope.data = {};

			handlers[dataResp] = respDataCb;

			wss.bindHandlers(handlers);
			getData();
			function fetchDataIfNotWaiting() {
				getData();
			}
			
			function startRefresh() {
				refreshPromise = $interval(fetchDataIfNotWaiting, refreshInterval);
			}

			function stopRefresh() {
				if (refreshPromise) {
					$interval.cancel(refreshPromise);
					refreshPromise = null;
				}
			}
			startRefresh();
			// cleanup
			$scope.$on('$destroy', function () {
				wss.unbindHandlers(handlers);
				stopRefresh();
				$log.log('OvStarfaceserverclientCtrl has been destroyed');
			});

			
		}]);
}());