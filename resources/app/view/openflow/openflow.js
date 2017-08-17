(function () {
    'use strict';
	var $log, $interval, $timeout, $scope, wss, ks;
	// constants
	var dataReq = 'OpenflowDataRequest',
		dataResp = 'OpenflowDataResponse';
    // constants
    var refreshInterval = 2000;
	var refreshPromise=null;
	angular.module('ovOpenflow', [])
		.controller('OvOpenflowCtrl', 
		[ '$log', '$interval', '$timeout', '$scope', 'WebSocketService', 'KeyService',
		function (_$log_, _$interval_, _$timeout_, _$scope_, _wss_, _ks_) { 
			$log = _$log_;
			$scope = _$scope_;
			$interval = _$interval_;
			$timeout = _$timeout_;
			wss = _wss_;
			ks = _ks_;
			
			function respDataCb(data) {
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
			
			var handlers = {};
			$scope.openflows = {};
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
				$log.log('OvOpenflowCtrl has been destroyed');
			});

		}]);
}());