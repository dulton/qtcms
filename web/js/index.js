var oPreView,oDiv,autoSearchDev,
	winState=[lang.Have_access_to_the_connection,lang.Connecting,lang.Disconnected,lang.Being_disconnected],
	currentWinStateChange = [lang.Connected,lang.Connecting,lang.Off,lang.Shutting_down],
	errorcolor = 'red';
 var viewDiv = [1,4,6,8,9,16,25,36,49];
 var view =['div1_1','div2_2','div6_1','div8_1','div3_3','div4_4','div5_5','div6_6','div7_7'];
 var recordStatus = 0;//录像状态
	$(function(){
		
		oPreView= $('#previewWindows')[0];
		
		oDiv = $('div.dev_list');

		var oAs = $('ul.dev_list_btn a');
	    
		autoSearchDev = $('#atuoSearchDevice')[0];
		autoSearchDev.startGetUserLoginStateChangeTime();
		//autoSearchDev.loginEx();
		//$(window).off();
         getUsername();
	    $('div.dev_list:eq(1)').hide();
		
		$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
		   var action = $(this).attr('class').split(' ')[0];
		   addMouseStyleByLimit($(this),action,1<<0);
	       })
		 //下拉菜单模拟
		/*// $('.select1').toSelect();  	
	    $('.select1').mousedown(function(){
			checkUserRightdiv(1<<0,0,'selectfn',$(this));
	    });
			*/	
		oAs.each(function(index){
			$(this).click(function(){
				$(window).off();
				oAs.removeClass('active');
				oDiv.hide();
				$(this).addClass('active');
				oDiv.eq(index).show();
			})
		})
 		//打开通道
		oDiv.on('dblclick','span.channel',function(){
			
			checkUserRightdiv(1<<0,0,"openclosechl",$(this));
			
		});

		//打开设备下的所有通道
		oDiv.on('dblclick','span.device',function(){ 
		    checkUserRightdiv(1<<0,0,"openclosedev",$(this));
			
		})
		
		//显示分屏的文字
		$('div.operat li.setViewNum').click(function(){
			$(this).siblings().removeClass('aaa');
			$(this).addClass('aaa');
			setViewNumNow();
		})

		//日志区域右键从菜单
		$('#actionLog').mouseup(function(event){
			if(event.which == 3){
				var maxT = $(this).offset().top+$(this).height() - 19;
				var l = event.pageX > $(this).width() - 84 ? $(this).width() - 84 : event.pageX;
				var t = event.pageY > maxT ? maxT: event.pageY;

				$('div.emptyAct').css({
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
				if($('#dev_'+target.data('data').dev_id).data('data').vendor == 'IPC'){
					return;
				}
				$('div.dev_list span.channel').removeClass('sel');
				target.addClass('sel');
				var maxT = $(this).offset().top+$(this).height() - 24;
				var l = event.pageX > $(window).width() - 80 ? $(window).width() - 80 : event.pageX;
				var t = event.pageY > maxT ? maxT :event.pageY;
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
		//云台控制速度
		$('#PTZ_control .ptz_speed span').each(function(index){
			$(this).click(function(){
				$('#PTZ_control .ptz_speed span').removeClass('act').slice(0,index+1).addClass('act');
				//writeActionLog(_T('PTZ_speed')+(index+1));
			})
		})

		// 云台方向控制;
		$('#Dire_Control div:lt(4)').on({
			mousedown:function(){
				//var currentWindow = oPreView.GetCurrentWnd();
				//var windowInfo = oPreView.GetWindowInfo(currentWindow);
				//console.log(windowInfo);
				checkUserRightBtn(1<<1,0,"PTZcontrol",$(this).attr('PTZ',1).index());
			},
			mouseup:function(){
				//writeActionLog(_T('stop_PTZ'));
				if($(this).attr('PTZ')){
				  oPreView.ClosePTZ($(this).removeAttr('PTZ').index());
				}
			},
			mouseleave:function(){
				if($(this).attr('PTZ')){
					//writeActionLog('鼠标移开停止PTZ');
					oPreView.ClosePTZ($(this).index());	
				}
			}
		})
		$('#Dire_Control div:last').on({
			mousedown:function(){
				/*alert($(this).attr('PTZ'));
				if($(this).attr('PTZ')){
				// writeActionLog('停止自动PTZ');
					$(this).removeAttr('PTZ');
					oPreView.ClosePTZ($(this).index());
				}else{
					//writeActionLog('开始自动PTZ');
					//$(this).attr('PTZ',1);
					PTZcontrol($(this).attr('PTZ',1).index());		
				}*/
				//var currentWindow = oPreView.GetCurrentWnd();
				//var windowInfo = oPreView.GetWindowInfo(currentWindow);
				//console.log(windowInfo);
				checkUserRightBtn(1<<1,0,"PTZcontrol",$(this).index());
				
			}
		})
			
		$('.para_Control li').each(function(index) {
            $(this).click(function(){
				var index = $(this).attr('data');
				//var currentWindow = oPreView.GetCurrentWnd();
				//var windowInfo = oPreView.GetWindowInfo(currentWindow);
				checkUserRightBtn(1<<1,0,"PTZcontrol",index);
			});
        });
		setViewMod(oCommonLibrary.getSplitScreenMode());
		//同步设置分屏UI
		var indexLi = $('li.setViewNum[onclick*='+oCommonLibrary.getSplitScreenMode()+']'),
			backPosition = indexLi.css('background-position').split(' ');
			indexLi.css('background-position','-30px '+backPosition[1]);
            indexLi.addClass('aaa');
		$('#setModel').css('background-position',indexLi.css('background-position'));

		setViewNumNow();

		//绑定控件事件
		oPreView.AddEventProc('CurrentWindows','WindCallback(ev)')

		oPreView.AddEventProc('CurrentStateChange','windChangeCallback(ev)');

		oPreView.AddEventProc('DivModeChange','setViewNumNow(ev)');

		oPreView.AddEventProc('ConnectRefuse','ConnectRefuse(ev)');
		
		oPreView.AddEventProc('Authority','Authority(ev)');
		
		oPreView.AddEventProc('wndStatus','ViewMax()');
		
		oPreView.AddEventProc('screenShotInfo','screenShotcallback(data)');
		
		oPreView.AddEventProc('Validation','PluginLimitcallback(data)');
		
         autoSearchDev.AddEventProc("reFreshDeviceList",'CurrentStateChange(ev)');
		 
		 autoSearchDev.AddEventProc("useStateChange",'useStateChange(ev)');
		 
		  AddActivityEvent('Validation','Validationcallback(data)');
		 
		var url =['index.html','play_back.html','backup.html','device.html','super.html','','log.html']
		/*for(i in url){
			if(i != 0){ 
				$('#winCon')[0].LoadNewPage('/skins/default/'+url[i]);
			}
		}*/
		$('div.top_nav li').each(function(index){
			$(this).click(function(){ 
				if(index == 0||index==5){
					return;
				}
				$('#winCon')[0].LoadNewPage('/skins/default/'+url[index]);
			})	
		})

		bFullScreen = oCommonLibrary.getAutoFullscreen();

		viewFullScreen();


		initOxcDevListStatus();
		
		var booll =$("#search_device .dev_list ul:gt(0) span").hasClass("device");

		!booll && $('#atuoSearchDevice')[0].startAutoSearchDevice(10,500,150);

		//设备是否自动连接功能
		DevAutoConnected();
		
            
		//window.status = '<pageaction SrcUrl="/skins/default/index.html" SrcAct="index" DstUrl="/skins/default/log.html" DstAct="reload"></pageaction>';
	})///
     function Validationcallback(data){ //id按钮权限验证
	 // console.log(data);
		if(data.ErrorCode=="1"){
			//autoSearchDev.showUserLoginUi(336,300);
		}else if(data.ErrorCode=="2"){
			writeActionLog(_T('no_limit'),errorcolor);
			var timer =setTimeout(function(){
				closeMenu();
				clearTimeout(timer);
			},1000);
		}
	}
	function PluginLimitcallback(data){
	   //console.log(data);
		if(data.ErrorCode=="1"){
			autoSearchDev.showUserLoginUi(336,300);
		}else if(data.ErrorCode=="2"){
			writeActionLog(_T('no_limit'),errorcolor);
			var timer =setTimeout(function(){
				closeMenu();
				clearTimeout(timer);
			},1000);
		}	
	}
	 
	function CurrentStateChange(ev){
		
	      if(ev.reFreash =='false')
		       return;
		  
			initOxcDevListStatus();
			  
			openCloseAll(1);
		
	}
	
	function screenShotcallback(data){
		
	  //console.log(data);	
	  writeActionLog(T('screenShot_success',data.fileDir));
	}
	function maxDiv(num){
		
		 var temp=-1;
		 for(var i=0;i<viewDiv.length;i++){
			  if(num<=viewDiv[i]){
			     temp = i;
				 break;
			  }   
		 }
		 if(temp==-1) temp=8;
		return  parseInt(temp,10); 
	}
	$(window).resize(viewFullScreen);
	
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
			height:oView.height()+144
		});

		oDiv.height(oLeft.height()-266);

		$('#operating').css({
			width:oView.width(),
			top:oView.height()+80
		});
		
		$('#actionLog').width(oView.width()-10);
		$('#foot').css('top',oView.height()+212);
	}

	function CloseWind(wind,channel_id){ 
       oPreView.CloseWndCamera(wind);
	}

	function openCloseAll(bool){  //打开关闭所有窗口
		var str = '';
		if(bool){
			if(!checkAlldevAllOpen()){
				
			 var devNum=0;
			
			 $("#search_device .dev_list span.channel").each(function(){
					  devNum++;
			 });
				 var Li = $('div.operat li.aaa'),
			       bp = Li.css('background-position').split(' ');
				 Li.css('background-position','0px '+bp[1]);
			  
			  setViewMod(view[maxDiv(devNum)]);
			 var indexLi = $('li.setViewNum[onclick*='+view[maxDiv(devNum)]+']'),
				 backPosition = indexLi.css('background-position').split(' ');
			  indexLi.css('background-position','-30px '+backPosition[1]);
		
			 $('#setModel').css('background-position',indexLi.css('background-position'));
		      setViewNumNow(); 
			 // $('div.operat li.setViewNum:eq('+maxDiv(devNum)+')').click(setViewNumNow);
			 // alert(maxDiv(devNum));
			     var wind = 0;
               
			  $('div.dev_list:visible span.channel').not('[wind]').each(function(){
				   wind = getWind(wind);
				   if(wind  == -1)return;
				   openWind(wind,getChlFullInfo($(this)));
				   wind++;
			     })

			  str = lang.All_channelsare_open_under_the_current_list;
			}/*else{
				str = lang.All_channel_already_open_under_the_current_list;
				}*/
		}else{
            
			/*if(checkAllchannelClose()){
				str = lang.The_current_list_of_all_channels_already_closed_under;
			}else{
					 oPreView.CloseAll();

			//$('div.dev_list:visible span.channel[wind]').each(function(){
			//	CloseWind($(this).attr('wind'),getChlFullInfo($(this)).dev_id);
			//})
			    str = lang.The_current_list_of_all_channels_are_closed_under;
			}*/
			if(!checkAllchannelClose()){ 
			   oPreView.CloseAll();
			   str = lang.The_current_list_of_all_channels_are_closed_under;
			}
			
		}
		writeActionLog(str);
	}
    function checkAlldevAllOpen(){
		var b = 1;
		$('div.dev_list:visible span.device').each(function(){
			if(!$(this).attr('bAllopen')){
				b = 0;
			}
		})
	  return b;
		
		}
	function checkAllchannelClose(){
	  var b = 1;
	  $('div.dev_list:visible span.channel').each(function(){
		  if($(this).attr('wind')){
			  b = 0;
		  }
	  })
	return b;
	  
	  }
	function checkAllchannelOpen(){
		var b = 1;
		$('div.dev_list:visible span.channel').each(function(){
			if(!$(this).attr('wind')){
				b = 0;
			}
		})
		/*var obj = $('#openAllchannel');
		if(b){
			obj.attr('toggle',1).css('background-position','0px'+' '+(-obj.height())+'px');
		}else{
			obj.removeAttr('toggle').css('background-position','0 0');
		}	*/
	  return b;
	}

	function openWind(wind,data){
			var windState = oPreView.GetWindowInfo(wind).usable;
				//console.log('当前窗口:'+wind+'的状态'+windState);
				if(!windState){ //该窗口不可用.
					var sWind = parseInt(wind)+1;
					var str = T('Open_failed_Error_The_current_window',data.name,data.channel_name,sWind)/*+'  '+winState[windState]*/;
					writeActionLog(str,errorcolor);
					wind = getWind(wind)
					//console.log('调整后的窗口:'+wind+'的状态'+oPreView.GetWindowConnectionStatus(wind));
				}		
		
				//console.log('最终确定窗口:'+wind+'的状态'+oPreView.GetWindowConnectionStatus(wind));
		
				if(wind  == -1) {return;}
				
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
		var obj = $('#channel_'+ev.ChannelId),
			c = '',
			chlData = getChlFullInfo(obj),
			str=T('device_in_window_action',chlData.name,chlData.channel_name,(parseInt(ev.WPageId)+1))+currentWinStateChange[ev.CurrentState];
		//console.log('befor------data---------state:'+ev.CurrentState+'-----------wind:'+ev.WPageId+'------channel_ID:'+ev.ChannelId);
		obj.attr({state:ev.CurrentState,wind:ev.WPageId});
		if(ev.CurrentState == 2){
			obj.removeAttr('state wind').removeClass('channel_1');
			checkDevAllOpen(obj.data('data').dev_id);
			//checkAllchannelOpen()
			writeActionLog(str);
		}else if(ev.CurrentState == 0){	
			checkDevAllOpen(obj.data('data').dev_id);
			obj.addClass('channel_1');
			//checkAllchannelOpen()
			writeActionLog(str);
			
		  if(recordStatus){
			var data = chlData;
			var wind = parseInt(ev.WPageId);
			var transKey,backStatus;
				if(oPreView.SetDevInfo(data.device_name,data.channel_number,wind)){
					transKey = 'channel_Manual_recording_data_binding_failed'
				}else{
					backStatus = oPreView.StartRecord(wind);
					//console.log('backStatus'+backStatus);
					if(backStatus){
						transKey = 'channel_Manual_recording_fail';
						if(backStatus == 2){
							transKey = 'channel_has_been_in_the_planning_Video_state';
						}
					}else{ 
						transKey = 'Start_manual_recording';
						c = '';
					}
				}
				writeActionLog(T(transKey,data.name,data.channel_name),c);
           }
			
		}else{
			str='';
			writeActionLog(str);
		}
		//console.log('after-----.channel----------state:'+obj.attr('state')+'-----------wind:'+obj.attr('wind'));
		
		

		/*if(checkOcxAllUsed() && ev.CurrentState == 0){
			writeActionLog('所以窗口正在使用!!',errorcolor);
		}*/
		if(ev.CurrentState == 0){
			var goal = 0;
			$('div.dev_list span.channel[state="0"]').each(function(){goal++;});
			if(goal>35){
				writeActionLog(_T('window_connected_tips'),errorcolor);
				}
		}
	}
	//获取当前窗口最经一个可用的窗口。
	function getWind(i){
		if(i>63){
			if(checkOcxAllUsed()){
				return -1;
			}else{	
				i = 0;
			}
		}
		if(!oPreView.GetWindowInfo(i).usable){
			i++;
			return getWind(i);
		}else{ 
			return i;
		}
	}
	function checkOcxAllUsed(){
		for(var i=0;i<63;i++){
			if(oPreView.GetWindowInfo(i).usable){
				return false;
			}
		}
		return true;
	}
	// 检查该设备是否全开.
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
	//用户登录状态回调函数
    function useStateChange(ev){
		//console.log(ev);
		if(ev.status==0){
		 $('.top_nav p span:eq(1)').html(ev.userName);	
		}else{
			var str = _T('not_Login');
		   $('.top_nav p span:eq(1)').html(str);	
		}
	}
	function ConnectRefuse(ev){
		var data = oPreView.GetWindowInfo(ev.WPageId),
			oDevData =  getChlFullInfo($('#channel_'+ev.ChannelId));
			chlData = $('#channel_'+data.chlId).data('data');
		writeActionLog(T('Resource_loaded',oDevData.device_name,chlData.channel_name,ev.WPageId),errorcolor);
	}
    
	function Authority(ev){
		var data = oPreView.GetWindowInfo(ev.WPageId),
			oDevData =  getChlFullInfo($('#channel_'+ev.ChannelId));  // 获取通道的所有信息包裹所属设备信息
			chlData = $('#channel_'+data.chlId).data('data');
		writeActionLog(T('User_logined',oDevData.device_name,chlData.channel_name,(parseInt(ev.WPageId)+1)),errorcolor);
		}
		
	//日志信息操作
	var countT=0;
	function writeActionLog(str,color){ 
		//var color = color ? color : '#4DBDEE';
		/*if(str){
			$('<p>[ '+getNowTime()+' ] :  <span style="color:'+color+'">'+str+'</span></p>').prependTo('#actionLog');
		}*/
		if(countT>=100){
		     countT=0;
		  	$('#actionLog p:gt(20)').remove();
			
		 }

		var color = color || '#4DBDEE'; 
		
		var parentNd = document.getElementById('actionLog');
		
		var childNd = document.createElement('p');
		childNd.innerHTML = ' [ '+getNowTime()+' ] :  <span style="color:'+color+'">'+str+'</span>';
		//childNd.innerHTML = '<span style="color:'+color+'">'+str+'</span>';
		 var node = parentNd.firstChild;
		// parentNd.insertBefore(childNd,node);
		
		 str && ((node && parentNd.insertBefore(childNd,node)) || parentNd.appendChild(childNd)); 
		 
		 countT++;
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

	function oPreviewIsAllUsed(){
		var b = false;
		for(var i=0;i<64;i++){
			if(!oPreView.GetWindowInfo(i).usable){
				b = true;
			}
		}
		return b;
	}
	
     //录像
	function Record(obj){ 
		var transKey = '',
		backStatus= 0,
		data = {},
		c = errorcolor;

		obj.mouseleave(function(){
			setTimeout(function(){
				if(!oPreviewIsAllUsed()){
					//writeActionLog(_T('没有接入的通道!'),c);
					obj.css('background-position','-120px -72px').removeAttr('toggle');
					//return;
				}
			},1)
		})

		setTimeout(function(){
			if(!oPreviewIsAllUsed()){
				writeActionLog(_T('no_Channle_Preview'),c);
				obj.css('background-position','-120px -72px').removeAttr('toggle');
				return;
			}
			if(obj.attr('toggle')){
				recordStatus = 1;
				$('div.dev_list span.channel[wind]').each(function(){

					data =getChlFullInfo($(this));
					/*console.log('------------1-------------');
					console.log(data);*/
					if(oPreView.SetDevInfo(data.device_name,data.channel_number,$(this).attr('wind'))){
						transKey = 'channel_Manual_recording_data_binding_failed'
					}else{
						backStatus = oPreView.StartRecord($(this).attr('wind'))
						console.log('backStatus'+backStatus);
						if(backStatus){
							transKey = 'channel_Manual_recording_fail';
							if(backStatus == 2){
								transKey = 'channel_has_been_in_the_planning_Video_state';
							}
						}else{ 
							transKey = 'Start_manual_recording';
							c = '';
						}
					}
					writeActionLog(T(transKey,data.name,data.channel_name),c);
				})
			}else{
				recordStatus = 0;
				$('div.dev_list span.channel[wind]').each(function(){
					data = getChlFullInfo($(this)),
					backStatus = oPreView.StopRecord($(this).attr('wind'));
					/*console.log('------------12-------------');
					console.log(data);*/
					if(backStatus){ 
						transKey = 'Close_the_manual_recording_failed'
						if(backStatus == 2){
							transKey = 'channel_has_been_in_the_planning_Video_state';
						}
					}else{ 
						transKey = 'Close_the_manual_recording';
						c = '';
					}
					writeActionLog(T(transKey,data.device_name,data.channel_name),c);
				})	
			}
		},500)

		/*obj.blur(function(){
			if(backStatus){
				obj.attr('toggle',1).css('background-position','-120px -108px');
			}else{
				obj.removeAttr('toggle').css('background-position','-120px -72px');
			}
		})*/
		
	}
	
     // 切换码流
	function SwithStream(){  
		var oChlData = $('#search_device span.channel.sel').data('data');
		if($('#dev_'+oChlData.dev_id).data('data').vendor == 'IPC'){
			return;
		}
		var	currWin = oPreView.GetCurrentWnd(),
			str = T('channel_switch_Stream',(currWin+1)),
			stream = oChlData.stream_id ? 0 : 1,
			c = errorcolor;
		if(oCommonLibrary.ModifyChannelStream(oChlData.channel_id,stream)){
			str += lang.Failed;
		}else{
			if(oPreView.SwithStream(currWin,oChlData.channel_id)){
				str += lang.Failed;
			}else{
				oChlData.stream_id = $('#search_device span.channel.sel').data('data').stream_id = stream;
				str += lang.Success;
				c = '';
			}
		}
		writeActionLog(str,c);
	}
    //ptz云台控制函数
	function PTZcontrol(code){
	 if(oPreView.GetWindowInfo(oPreView.GetCurrentWnd()).currentState == 0 ){  //当窗口接入预览时
		if(oPreView.OpenPTZ(code,$('#PTZ_control .act').length)){
			alert(lang.PTZ_operation_failed);
			}
	 }else{
			writeActionLog(_T('window_no_Preview'),errorcolor);	 
	 }

	}
	
	//  初始化控件,设备列表,以及之间的状态
	function initOxcDevListStatus(){  
		//区域列表;
		areaList2Ui();
		//分组列表;
		//groupList2Ui();

		$('span.channel').removeClass('channel_1');

		for(var i=0;i<64;i++){
			var oWinInfo = oPreView.GetWindowInfo(i);
			if(oWinInfo.chlId>0 && oWinInfo.currentState == 0 && $('#channel_'+oWinInfo.chlId)[0]){
				var chlData = $('#channel_'+oWinInfo.chlId).attr({
					wind:i,
					state:oWinInfo.currentState
				}).addClass('channel_1').data('data');
				checkDevAllOpen(chlData.dev_id);
			}
		}
	}
	//单击通道时打开、关闭通道
    function openclosechl(obj){
		var chlData = getChlFullInfo(obj);
		
			if(obj.attr('state')){
			   CloseWind(parseInt(obj.attr('wind'),10),parseInte(chlData.channel_id,10));
			}else{
				openWind(parseInt(oPreView.GetCurrentWnd(),10),chlData);
			}
		
	}
	//单击设备时打开、关闭设备下的所有通道
	function openclosedev(obj){
		
			    var oDevice = obj;
				var chlData;
				var wind = oPreView.GetCurrentWnd();
				if(oDevice.attr('bAllopen')){
					oDevice.next('ul').find('span.channel').each(function(){
						chlData = getChlFullInfo($(this));	 
						CloseWind($(this).attr('wind'),chlData.channel_id);
					})
				}else{
					oDevice.next('ul').find('span.channel').each(function(){
						chlData = getChlFullInfo($(this));	 
						if(!$(this).attr('wind')){
							oDevice.attr('bAllopen','1');
								win = getWind(wind);
								if(win  == -1) {return;}
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
		
	}
	
	//判断在页面加载完之后是否自动连接列表中所有设备
	function DevAutoConnected(){ 
	  var booll =$("#search_device .dev_list ul:gt(0) span").hasClass("device");
	  
	  if(booll && oCommonLibrary.getAutoConnect()) //如果设备列表不为空，且自动连接设备为true
	  {	   
		  openCloseAll(1); //打开列表中的全部设备
		 }
	 }
	 //全屏
   	function viewFullScreenEx(){
		$('#viewWarp').css({
			width:'100%',
			height:'100%',
			top:0,
			left:0,
			position: 'absolute'
		});
		getAudioObj().SetFullScreenFlag();
	}
	
	function viewFullScreen(){
		
		if(bFullScreen){
			viewFullScreenEx();
		}else{
			ViewMax();
		}
	}
  //画面是否按原始比例
  function allWindowStretch(obj){
	  var bl = obj.attr('toggle');
	  if(bl){
		obj.attr('title',_T('video_origin_screen')); 
		getAudioObj().AllWindowStretch(false); 
	   }else{
		 obj.attr('title',_T('video_Full_screen'));
		 getAudioObj().AllWindowStretch(true);
	   }
	  
   }
  //自动轮巡
  function AutoPolling(obj){

	 var bl = obj.attr('toggle'); 
	 var timer;
     if(bl){
	   getAudioObj().StartAutoPolling();
	   writeActionLog(_T('Automatically_start_polling'));
	 }else{
	   getAudioObj().StopAutoPolling();	
	   writeActionLog(_T('Stop_automatic_polling'));
	 }
	 
  }
  //截屏
  function ScreenShot(){
	  var user = autoSearchDev.getCurrentUser();
	  oPreView.screenShot(user,0);
  }

  //验证用户是否有权限
  function checkUserRight(uicode,uisubcode){
	  //console.log('uicode:'+uicode+' uisubcode:'+uisubcode);
	  var itema= autoSearchDev.checkUserLimit(uicode.toString(2),uisubcode);
	   //console.log("当前用户"+autoSearchDev.getCurrentUser()+" 登录状态："+itema);
		return itema;
 }
  function checkUserRightBtn(uicode,uisubcode,fn,num){
	  //console.log('uicode:'+uicode+' uisubcode:'+uisubcode);
	  var itema= autoSearchDev.checkUserLimit(uicode.toString(2),uisubcode);
	  //console.log("当前用户"+autoSearchDev.getCurrentUser()+" 登录状态："+itema);
		if(itema==0){
			window[fn](num);
		}else if(itema==1){
		}else{
		  writeActionLog(_T("no_limit"),errorcolor);
		}
 }
 function checkUserRightdiv(uicode,uisubcode,fn,num){
	 
	 var itema =  checkUserRight(uicode,uisubcode);
			if(itema==0){
				window[fn](num);
			}else if(itema==1){
			  var show = autoSearchDev.showUserLoginUi(336,300);
			  if(show==0){
				   checkUserRightBtn(uicode,uisubcode,fn,num);
			  } 
		    }else{
		        writeActionLog(_T("no_limit"),errorcolor);
		    }
	 
 }
 
function getUsername(){
	 
	 var username = autoSearchDev.getCurrentUser();
	username && $('.top_nav div p span:eq(1)').html(username);
	 
 }
 