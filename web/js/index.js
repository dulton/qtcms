var oPreView,oDiv;
var winState=[lang.Have_access_to_the_connection,lang.Connecting,lang.Disconnected,lang.Being_disconnected];
var currentWinStateChange = [lang.Connected,lang.Connecting,lang.Off,lang.Shutting_down];
	$(function(){
		
		oPreView= $('#previewWindows')[0];
		
		oDiv = $('div.dev_list');

		var oAs = $('ul.dev_list_btn a');
	    
		$(window).off();

	    $('ul.filetree').treeview().not(':eq(0)').parent('div.dev_list').hide();
		
		$('.hover').each(function(){
			var action = $(this).attr('class').split(' ')[0];
			addMouseStyle($(this),action);
		})
		
		oAs.each(function(index){
			$(this).click(function(){
				$(window).off();
				oAs.removeClass('active');
				oDiv.hide();
				$(this).addClass('active');
				oDiv.eq(index).show();
			})
		})
		//控件最大化
		ViewMax();
		
		$(window).resize(ViewMax)
 		//打开通道
		oDiv.on('dblclick','span.channel',function(){ 
			/*debugData($(this).data('data'));*/
			var chlData = getChlFullInfo($(this));
			if($(this).attr('state')){
				CloseWind($(this).attr('wind'),chlData.dev_id);
			}else{
				openWind(oPreView.GetCurrentWnd(),chlData);
			}
		})

		//打开设备下的说所有通道
		oDiv.on('dblclick','span.device',function(){ 
			var oDevice = $(this)
			var chlData;
			var wind = oPreView.GetCurrentWnd();
			if(oDevice.attr('bAllopen')){
				oDevice.next('ul').find('span.channel').each(function(){
					chlData = getChlFullInfo($(this));	 
					CloseWind($(this).attr('wind'),chlData.dev_id);
				})
			}else{
				oDevice.next('ul').find('span.channel').each(function(){
					chlData = getChlFullInfo($(this));	 
					if(!$(this).attr('wind')){
						oDevice.attr('bAllopen','1');
						var windState = oPreView.GetWindowConnectionStatus(wind);
						var win = wind;
						if(windState != 2){
							win = getWind(wind);
						}
						openWind(win,chlData);
					}	
				})
			}
			
			var str = '';
			if(oDevice.attr('bAllopen')){ 
				str = T('open_device',(parseInt(wind)+1),chlData.name);
			}else{ 
				str = lang.Shutting_down_device+chlData.name;
			}
			writeActionLog(str);
		})

		//显示分屏的文字
		$('div.operat li.setViewNum').click(function(){ 
			setViewNumNow();
		})

		//日志区域右键从菜单
		$('#actionLog').mouseup(function(event){
			if(event.which == 3){
				var l = event.pageX > $(this).width() - 84 ? $(this).width() - 84 : event.pageX;
				var t = event.pageY - $(this).offset().top 
					t = t > $(this).height() - 19 ? $(this).height() - 19: t;
				$(this).find('div.emptyAct').css({ 
					left:l,
					top:t
				}).show();
				$(document).click(function(){ 
					closeMenu();
					$(document).off();
				})
			}
		})
		//切换码流右键菜单
		$('div.dev_list').mouseup(function(event){
			var target = $(event.target),
				oMenu = $('div.menu0');
			if(event.which == 3 && target.hasClass('channel')){
				$('div.dev_list span.channel').removeClass('sel');
				target.addClass('sel');
				var l = event.pageX > $(window).width() - 80 ? $(window).width() - 80 : event.pageX;
				var t = event.pageY > $(this).offset().top+$(this).height() - 24 ? $(this).offset().top+$(this).height() - 24 :event.pageY;
				oMenu.css({ 
					left:l,
					top:t
				}).show();
				$(document).click(function(){ 
					$('div.dev_list span.channel').removeClass('sel');
					closeMenu();
					$(document).off();
				})
			}

		})

		/*//云台控制速度
		$('#PTZ_control .ptz_speed span').each(function(index){
			$(this).click(function(){
				$('#PTZ_control .ptz_speed span').removeClass('act').slice(0,index+1).addClass('act');
			})
		})

		// 云台方向控制;
		$('#Dire_Control div:lt(4)').on({
			mousedown:function(){
				//writeActionLog('开始PTZ');
				PTZcontrol($(this).attr('PTZ',1).index());
			},
			mouseup:function(){
				//writeActionLog('停止PTZ');
				oPreView.ClosePTZ($(this).removeAttr('PTZ').index());
			},
			mouseleave:function(){
				if($(this).attr('PTZ')){
					//writeActionLog('鼠标移开停止PTZ');
					oPreView.ClosePTZ($(this).index());	
				}
			}
		})
		$('#Dire_Control div:last').on({
			mouseup:function(){
				if($(this).attr('PTZ')){
					//writeActionLog('停止自动PTZ');
					$(this).removeAttr('PTZ');
					oPreView.ClosePTZ($(this).index());
				}else{
					//writeActionLog('开始自动PTZ');
					$(this).attr('PTZ',1);
					PTZcontrol($(this).attr('PTZ',1).index());		
				}
			}
		})*/
		setViewMod(oCommonLibrary.getSplitScreenMode());
		//同步设置分屏UI
		var indexLi = $('li.setViewNum[onclick*='+oCommonLibrary.getSplitScreenMode()+']'),
			backPosition = indexLi.css('background-position').split(' ');
			indexLi.css('background-position','-30px '+backPosition[1]);

		$('#setModel').css('background-position',indexLi.css('background-position'));

		setViewNumNow();
		//绑定控件事件
		oPreView.AddEventProc('CurrentWindows','WindCallback(ev)')

		oPreView.AddEventProc('CurrentStateChange','windChangeCallback(ev)');
		var url =['index.html','play_back.html','backup.html','device.html','log.html']
		/*for(i in url){
			if(i != 0){ 
				$('#winCon')[0].LoadNewPage('/skins/default/'+url[i]);
			}
		}*/
		$('div.top_nav li').each(function(index){
			$(this).click(function(){ 
				if(index == 0){
					return false;
				}
				$('#winCon')[0].LoadNewPage('/skins/default/'+url[index]);
			})	
		})

		//window.status = '<pageaction SrcUrl="/skins/default/index.html" SrcAct="index" DstUrl="/skins/default/log.html" DstAct="reload"></pageaction>';
	})///
	function ViewMax(){
		var W = $(window).width();
		var H = $(window).height();
			W = W <= 1000 ? 1000: W;
			H = H <= 600 ? 600: H;

		var oView = $('#viewWarp').css({
			width:W-236,
			height:H-240,
			top:78
		});
		var oLeft= $('#search_device').css({
			left:oView.width(),
			height:oView.height()+123
		});

		oDiv.height(oLeft.height()-240);

		$('#operating').css({
			width:oView.width(),
			top:oView.height()+80
		});
		
		$('#actionLog').width(oView.width()-10);
		$('#foot').css('top',oView.height()+212)
	}

	function CloseWind(wind,dev_id){ 
		oPreView.CloseWndCamera(wind);
	}

	function openCloseAll(bool){  //打开关闭所有窗口
		if(bool){
			var wind = 0;

			$('div.dev_list:visible span.channel').not('[wind]').each(function(){
				wind = getWind(wind);
				openWind(wind,getChlFullInfo($(this)));
				wind++;
			})

			writeActionLog(lang.All_channelsare_open_under_the_current_list);
		}else{

			$('div.dev_list:visible span.channel[wind]').each(function(){
				CloseWind($(this).attr('wind'),getChlFullInfo($(this)).dev_id);
			})

			writeActionLog(lang.The_current_list_of_all_channels_are_closed_under);
		}
	}

	function checkAllchannelOpen(){
		var b = 1;
		$('div.dev_list:visible span.channel').each(function(){
			if(!$(this).attr('wind')){
				b = 0;
			}
		})
		var obj = $('#openAllchannel');
		if(b){
			obj.attr('toggle',1).css('background-position','0px'+' '+(-obj.height())+'px');
		}else{
			obj.removeAttr('toggle').css('background-position','0 0');
		}	
	}

	function openWind(wind,data){
		var windState = oPreView.GetWindowConnectionStatus(wind);
		if(windState != 2 ){ //该窗口不可用.
			var sWind = parseInt(wind)+1;
			var str = T('Open_failed_Error_The_current_window',data.name,data.channel_name,sWind)+'  '+winState[windState];
			writeActionLog(str);
		}
		wind = getWind(wind);
		$('#channel_'+data.channel_id+',#g_channel_'+data.channel_id).attr('wind',wind);

		oPreView.SetDevChannelInfo(wind,data.channel_id);

		oPreView.OpenCameraInWnd(wind,data.address,data.port,data.eseeid,data.channel_number,data.stream_id,data.username,data.password,data.channel_name,data.vendor);
	}

	function WindCallback(ev){ 
		var obj = $('div.dev_list span.channel').filter(function(){ 
			return $(this).attr('wind') == ev.Wid;
		})
		$('div.dev_list span.channel').removeClass('sel');
		obj.addClass('sel');
	}

	function windChangeCallback(ev){ //CurrentState 0 STATUS_CONNECTED,1 STATUS_CONNECTING,2 STATUS_DISCONNECTED,3 STATUS_DISCONNECTING;
		var obj = $('#channel_'+ev.ChannelId);
		/*$('div.dev_list span.channel').filter(function(){ 
			return $(this).attr('wind') == ev.WPageId;
		})*/
		var chlData = getChlFullInfo(obj);
		
		var str=T('device_in_window_action',chlData.name,chlData.channel_name,(parseInt(ev.WPageId)+1))+currentWinStateChange[ev.CurrentState];

		if(ev.CurrentState == 2){
			obj.removeAttr('state wind').removeClass('channel_1');
			checkDevAllOpen(obj.data('data').dev_id);
			//checkAllchannelOpen()
		}else if(ev.CurrentState == 0){	
			checkDevAllOpen(obj.data('data').dev_id);
			obj.addClass('channel_1');
			//checkAllchannelOpen()
		}else{
			str=''
			obj.attr({state:ev.CurrentState,wind:ev.WPageId});
		}
		writeActionLog(str);
	}
	//获取当前窗口最经一个可用的窗口。
	function getWind(i){
		if(oPreView.GetWindowConnectionStatus(i)!=2){
			i++;
			if(i>64){
				i=0;
			}
			return getWind(i);
		}else{ 
			return i;
		}
	}
	// 检车该设备是否全开.
	function checkDevAllOpen(dev_id){ 
		var bAllopen = 1;
		var oDev =$('#dev_'+dev_id);
		oDev.next('ul').find('span.channel').each(function(){ 
			if(!$(this).attr('wind')){ 
				bAllopen = 0;
			};
		})
		if(bAllopen ==1){
			oDev.attr('bAllopen',bAllopen);
			oDev.addClass('device_1');
		}else{ 
			oDev.removeAttr('bAllopen');
			oDev.removeClass('device_1');
		}
	}

	function setViewMod(i){
		oPreView.setDivMode(i);
	}

	function setViewNumNow(){     //显示当前分屏模式和当前第级分屏
		var str = (oPreView.getCurrentPage()+1)+'/'+oPreView.getPages();
		$('#nowWinMod').html('').html(str);
	}

	function preNextPage(type){ 
		if(type){ 
			oPreView.prePage();
		}else{ 
			oPreView.nextPage();
		}
		setViewNumNow();
	}

	//日志信息操作
	function writeActionLog(str){ 
		if(str){
			$('<p>'+getNowTime()+'  '+str+'</p>').prependTo('#actionLog');
		}
	}

	function showEmptyAction(){ 
		$('#actionLog a.emptyAct').show();
	}

	function emptyLog(){
		$('#actionLog p').remove();	
		$('#actionLog a.emptyAct').hide();
	}

	function getChlFullInfo(oChl){  // 获取通道的所有信息包裹所属设备信息
		var dev_id = oChl.data('data').dev_id;
		var devData = $('#dev_'+dev_id).data('data');
		var chlData = oChl.data('data');
		for(i in devData){ 
			chlData[i]=devData[i];
		}
		return chlData;
	}
	function getNowTime(){  // 获取当前时间
		var now = new Date();
		var H = now.getHours();
			H = H<10 ? '0'+H:H;
		var M = now.getMinutes();
			M = M<10 ? '0'+M:M;
		var S = now.getSeconds();
			S = S<10 ? '0'+S:S;
		return H+':'+M+':'+S;	
	}

	function Record(obj){ //录像
		var transKey = '',
		backStatus= 0,
		data = {};
		if(obj.attr('toggle')){
			$('div.dev_list span.channel[wind]').each(function(){
				data = $(this).data('data');
				if(oPreView.SetDevInfo(data.name,data.channel_number,$(this).attr('wind'))){
					transKey = 'channel_Manual_recording_data_binding_failed'
				}else{
					backStatus = oPreView.StartRecord($(this).attr('wind'))
					if(backStatus){
						transKey = 'channel_Manual_recording_fail';
						if(backStatus == 2){
							transKey = 'channel_has_been_in_the_planning_Video_state';
						}
					}else{ 
						transKey = 'Start_manual_recording';
					}
				}
			})
		}else{
			$('div.dev_list span.channel[wind]').each(function(){
				data = $(this).data('data'),
				backStatus = oPreView.StopRecord($(this).attr('wind'));
				if(backStatus){ 
					transKey = 'Close_the_manual_recording_failed'
					if(backStatus == 2){
						transKey = 'channel_has_been_in_the_planning_Video_state';
					}
				}else{ 
					transKey = 'Close_the_manual_recording';
				}
			})	
		}
		writeActionLog(T(transKey,data.name,data.channel_name));
		/*obj.blur(function(){
			if(backStatus){
				obj.attr('toggle',1).css('background-position','-120px -108px');
			}else{
				obj.removeAttr('toggle').css('background-position','-120px -72px');
			}
		})*/
	}

	function SwithStream(){  // 切换码流
		var oChlData = $('#search_device span.channel.sel').data('data'),
			currWin = oPreView.GetCurrentWnd(),
			str = T('channel_switch_Stream',(currWin+1));
			stream = oChlData.stream_id ? 0 : 1;
		if(oCommonLibrary.ModifyChannelStream(oChlData.channel_id,stream)){
			str += lang.Failed;
		}else{
			oChlData.stream_id = $('#search_device span.channel.sel').data('data').stream_id = stream;
			if(oPreView.SwithStream(currWin,oChlData.channel_id)){
				str += lang.Failed;
			}else{
				str += lang.Success;
			}
		}
		writeActionLog(str);
	}

	function PTZcontrol(code){
		if(oPreView.OpenPTZ(code,$('#PTZ_control .act').length)){
			alert(lang.PTZ_operation_failed);
		};
	}

