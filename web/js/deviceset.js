var oSearchOcx,autoSearchDev,onvifSetting,onvifuserInfo,
	searchedDev=[];//已经搜索到的设备;
	$(function(){
         document.getElementById('commonLibrary').getLanguage()== 'en_PR' ?  $('#Persian').show(): $('#Persian').hide();
		 oSearchOcx = document.getElementById('devSearch');
		 autoSearchDev = document.getElementById('atuoSearchDevice');
         onvifSetting = document.getElementById('onvifSetting');
		 onvifuserInfo = document.getElementById('onvifuserInfo');
		 
		 var username = autoSearchDev.getCurrentUser();
		  username && $('.top_nav div p span:eq(1)').html(username);
		  
		var oTreeWarp = $('div.dev_list').slice(2);

		oTreeWarp.hide();

		$('#iframe').hide();
		
			$('.hover').each(function(){  // 按钮元素添加鼠标事件对应样式
		   var action = $(this).attr('class').split(' ')[0];
		    addMouseStyleByLimit($(this),action,1<<5);
	       })
				
		$('.iput2.time_picker_start').each(function(i){
					
					var startDateTextBox =$(this);
					var endDateTextBox = $('.iput2.time_picker_end').eq(i);
		   
		            startDateTextBox.timepicker({
						 
					    timeFormat: 'HH:mm:ss',
						controlType:'select',
					    showTime: false,
						timeOnlyTitle: _T('Choose_time'),
						hourText: _T('Hour'),
						minuteText: _T('Minute'),
						secondText: _T('Second'),
						currentText: _T('current_time'),
						closeText: _T('confirm'),
					    onClose: function(dateText, inst) {
						  
						  if (endDateTextBox.val() != '') {
							
							var testStartDate = startDateTextBox.datetimepicker('getDate');
							var testEndDate = endDateTextBox.datetimepicker('getDate');
							var dataStart =startDateTextBox.attr('data');
							console.log(dataStart);
							if (testStartDate >= testEndDate){
								 
								Confirm(_T('Start_end_time_error'));
							//startDateTextBox.datetimepicker('setDate',dataStart);	
							startDateTextBox.val(dataStart);
							}
						  }else {
							endDateTextBox.val(dateText);
						  }
					
					    }
				    });	
			    });
				$('.iput2.time_picker_end').each(function(i){
					
					var startDateTextBox =$('.iput2.time_picker_start').eq(i);
					var endDateTextBox =$(this);
					 
				    endDateTextBox.timepicker({ 
				   
						timeFormat: 'HH:mm:ss',
	                    controlType:'select',
				        showTime: false,
					    //showButtonPanel: false,
					    timeOnlyTitle: _T('Choose_time'),
						hourText: _T('Hour'),
						minuteText: _T('Minute'),
						secondText: _T('Second'),
						currentText: _T('current_time'),
						closeText: _T('confirm'),
						
						onClose: function(dateText, inst) {
							if (startDateTextBox.val() != '') {
								var testStartDate = startDateTextBox.datetimepicker('getDate');
								var testEndDate = endDateTextBox.datetimepicker('getDate');
								var endData =endDateTextBox.attr('data');       
								if (testStartDate >= testEndDate){
									Confirm(_T('Start_end_time_error'));
									//endDateTextBox.datetimepicker('setDate', endData);
									endDateTextBox.val(endData);
								}
							}else{
								startDateTextBox.val(dateText);
							}
						}
					});
				});
				
		$('#searchtxt').on('keypress',function(event){
			if(event.keyCode == 13){ 
			  searchbtn();
			  $('#searchtxt').blur();
			}
			
			});
		$('div.menu .close').click(closeMenu);  //弹出操作框下部分元素添加关闭窗口事件
        $('#RemoveDevice_ok').click(function(){
			var timer = setTimeout(function(){
		    searchFlushReal();
		    clearTimeout(timer) 
		},100);});
		$('#device input:radio').each(function(index){ //添加设备弹出框下添加设备方式切换
			$(this).click(function(){
				$('#device input:radio').removeAttr('id');
				$(this).attr('id','method_ID');
				var oSwitch = $('#device tr').slice(2,6);
				oSwitch.removeClass('tr1').find('input').removeProp('disabled');
				if(index == 0){
					oSwitch.slice(3,4).addClass('tr1').find('input').prop('disabled',true);
				}else{
					oSwitch.slice(0,3).addClass('tr1').find('input').prop('disabled',true);
				}
			})
		})
		
				
			  $('#mainRight li :checkbox').click(function(){
				   var b = true;
				   $('#mainRight li :checkbox').each(function(){
						if(!$(this).prop('checked')){
							 b = false; 
						} 
				   })
				    $('#mainRight p').find('.selAll').prop('checked',b); 
			  });

				  $('.selAll').click(function(){
					 if($(this).prop('checked')){ 
						$(this).parent('p').next('ul').find('li input').prop('checked',true);
					 }else{
						$(this).parent('p').next('ul').find('li input').prop('checked',false);
					 }
				  });
				  
				  $('.cinterval').click(function(){
							$(this).next().next('input').prop('disabled',!$(this).prop('checked'));
					 });
				/*$('ul.filetree span.areaName').click(function(){
			$('ul.filetree span.areaName').css('background','0');
			$(this).css('background','#ccc');
		})*/

		// 添加设备下的 设备列表下的鼠标事件
		$('div.dev_list').each(function(index){
			var This = $(this);
			This.mouseup(function(event){ 
				event.stopPropagation();	
				var obj = $(event.target);
				/*if(obj[0].nodeName == 'SPAN'){
					alert(obj.attr('id'));
				}*/
				if(event.which == 1){
					if(obj[0].nodeName == 'SPAN'){
						obj.toggleClass('sel');
						This.find('span').not(obj).removeClass('sel');
						/*if(obj.hasClass('channel')){
							This.find('span').not('span.channel').removeClass('sel');
						}else{ 
							This.find('span').not(obj).removeClass('sel');
						}*/
					}
					//SetChannelIntoGroupData();
				}
				if(event.which == 3){ 
					//分组设置下的右键菜单调整
					$('#menu0 li').show()
					/*if(index == 1){
						$('#menu0').find('li:eq(1),li:eq(2)').hide();
					}else{
						$('#menu0 li:eq(0)').hide();
					}*/
					if(obj[0].nodeName == 'SPAN'){ 
						$('div.dev_list span').removeClass('sel');
						obj.addClass('sel');
					}else{ 
						if(!$(this).find('span.sel')[0]){
							$(this).find('span:eq(0)').addClass('sel');
						}
					}
					showContextMenu(event.clientY,event.clientX,obj);
				}else{ 
					$('#menu0').hide();
				}
			})
		})	
			
		//跳转其他页面之前 先关闭搜索。
		$('div.top_nav li').each(function(index){
			$(this).click(function(){
				if(index != 4){
					oSearchOcx.Stop();
					emptyDevSetMenu();
					
				    AJAX && AJAX.abort();
				  if(nowDev && nowDev._ID){
					  nowDev =null;
				   }
					
				}
				  
				
			  
			  
			})
		})
		
		//左侧二级菜单
		$('#set_content div.left li').each(function(index){
			
			$(this).click(function(){
				
				key = index;
				var warp = $('#set_content div.right div.right_content').hide().eq(index).show();
				
				oTreeWarp.show();
				
				set_contentMax();
				areaList2Ui('','closed');
				$('#ajaxHint').html('').stop(true,true).hide();
				 
				if(key == 0){
					searchFlush();
				}else{
				     
					 if(key == 1){
					AJAX && AJAX.abort();
					emptyDevSetMenu();
					if(nowDev && nowDev._ID){
						nowDev =null;
						}
					var itema = checkUserRight(1<<6,0);
					if(itema==1){
						autoSearchDev.showUserLoginUi(336,300);
					}else if(itema==2){
								  
					 closeMenu();
					 confirm_tip(_T('no_limit'));
					  var timer =setTimeout(function(){
						  closeMenu();
						  clearTimeout(timer);
					  },2000);
					}
					//reInitNowDev();  //重新加载设备信息
                    
					$('ul.filetree').not('[id]').eq(key).on('click','span.device',function(event){
                         
						 event.stopPropagation();
						 
						$('#ajaxHint').html('').stop(true,true).hide();

						$('ul.filetree:eq(2)').find('span.device').removeClass('sel');
						$(this).addClass('sel')

						var oDevData = $(this).data('data');
                   
						var _url = 'http://'+oDevData.address+':'+oDevData.port;

						$('ul.ipc_list0,ul.dvr_list0,div.dvr_list,div.ipc_list,ul.onvif_list0,div.onvif_list').hide();

						$('#dev_id_ID_Ex').val(oDevData.dev_id);

						AJAX && AJAX.abort();

						if(oDevData.vendor == 'IPC'){//如果选中设备为ipc
							$('ul.ipc_list0 li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct').parent('ul').show();
							//$('.ipc_list').eq($('.ipc_list0 li.ope_listAct').index()).show();
							$('.ipc_list').eq(0).show();
							//emptyDevSetMenu();
							//ipc(_url,oDevData.username,oDevData.password);
							//devinfo_load_content(true);	

							nowDev = new IPC(oDevData.username,oDevData.password,oDevData.address,oDevData.port,oDevData.dev_id,oDevData.vendor);
							
							console.log('------------new IPC()--------------');

							nowDev.ipcBasicInfo2UI();

							//oDev[$('ul.ipc_list0 li.ope_listAct').attr('action')]();
						}else if(oDevData.vendor == 'DVR' || oDevData.vendor == 'NVR'){//如果选中设备为DVR或NVR
							$('ul.dvr_list0 li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct').parent('ul').show();
							$('.dvr_list').eq(0).show();
							//emptyDevSetMenu();
                         
							//dvr(_url,oDevData.username,oDevData.password,oDevData.channel_count);
							//dvr_devinfo_load_content();
							console.log('------------new DVR()--------------');
							nowDev = new DVR(oDevData.username,oDevData.password,oDevData.address,oDevData.port,oDevData.dev_id,oDevData.vendor,oDevData.channel_count);
							
						

							nowDev.dvrBasicInfo2UI();
						}else if(oDevData.vendor=='ONVIF'){
							$('ul.onvif_list0 li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct').parent('ul').show();
							$('.onvif_list').eq(0).show();
							//console.log(oDevData);
							console.log('------------new ONVIF()--------------');
							
							nowDev = new ONVIF(oDevData.address,oDevData.port,oDevData.username,oDevData.password);
							
							nowDev.OnvifDeviceInfo2UI();
							//nowDev.onvifDeviceEncoderInfo2UI();
							
						}

			 	   });

				}else if(key == 2){  //单击“本地设置”
				   window['Fill'+warp.find('div.switch:visible').attr('id')+'Data']();
					var itema = checkUserRight(1<<7,0);
					if(itema==1){
						autoSearchDev.showUserLoginUi(336,300);
					}else if(itema==2){
								  
					 closeMenu();
					 confirm_tip(_T('no_limit'));
					  var timer =setTimeout(function(){
						  closeMenu();
						  clearTimeout(timer);
					  },2000);
					}
					
				}else if(key == 3){ 
				 
						$('.right_content:visible ul.ope_list li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct');
						$('.right_content:visible div.switch').hide();
						$('.right_content:visible div.switch').eq(0).show();
						cUserinfo2Ui();
		
				}					 
					oSearchOcx.Stop();
				}
				
				
				
			})
		})
        
			$('#set_content ul.dvr_list0 li').click(function(event){
				event.stopPropagation();
				//stop(true,true)解决不断用鼠标点击产生的积累
				$('#ajaxHint').html('').stop(true,true).hide();//当点击set_content right 的<li>标签时，要隐藏上个标签正在进行的ajaxhint 提示
				AJAX && AJAX.abort();//并停止正在进行的ajax请求
				
				if($(this).attr('action')){
					AJAX && AJAX.abort();
				nowDev && nowDev[$(this).attr('action')+'2UI']();

				}
			})

		  $('#set_content ul.ipc_list0 li').click(function(event){
			  event.stopPropagation();
			  $('#ajaxHint').html('').stop(true,true).hide();
			  AJAX && AJAX.abort();
			  if($(this).attr('action')){
				  AJAX && AJAX.abort();
				  nowDev && nowDev[$(this).attr('action')+'2UI']();
			  }
		  })

  
		  $('#set_content ul.onvif_list0 li').each(function(index){
			  $(this).click(function(){
			  event.stopPropagation();
			  $('#ajaxHint').html('').stop(true,true).hide();
			  AJAX && AJAX.abort();
			 // console.log('--to--before');
			  if($(this).attr('action')){
				  AJAX && AJAX.abort();
				 nowDev && nowDev[$(this).attr('action')+'2UI']();
				// console.log('--to--after');
			  }
			  });
		  })
		  
		   
		//搜索结果 设备列表tr委托部分事件;
		$('#SerachedDevList').on('click','tr',function(){
			var data = $(this).data('data');
			if(data.SearchVendor_ID == 'IPC'){
				for(i in data){
					$('#'+i).val(data[i]);
				}
			}else{
				$('#Allocation tbody input:text').val('');
			}
			$('#SerachedDevList').find('tr').removeClass('sel')
			$(this).addClass('sel');
		})

		//录像设置验证正确的时间段.
		/*$('#recordtime tbody tr:lt(4)').each(function(){
			$(this).on('focusout','input:text',function(){
				var oWarp = $(this).parent('div.timeInput').siblings('div.timeInput').addBack();
				if(oWarp.eq(0).gettime()>oWarp.eq(1).gettime()){
					Confirm(_T('time_range'));
					FillChannleRecordTime($('div.dev_list:eq(2) span.channel.sel'));
					//$(this).val($(this).attr('default'));
				}
			});
		});*/
		$('#Video_settings_SelectEveryday').click(function(){
		
				$('#weekday_choose input:enabled').prop('checked',$(this).prop('checked'));
			});
		//用户table下 tr委托部分事件
		$('#userList').on('click','tr',function(){  //添加用户 tr选中状态添加  数据整合到 hidden的input
			//整理选中的用户ID数组
			tableuserinit($(this));
		});
			
		/*$('ul.filetree').each(function(){ 
			var warp = $(this)
			$(this).on('click','span.device,span.group',function(){ 
				warp.find('span.device').removeClass('sel');
				$(this).addClass('sel');
			})
		})*/

		/*$('#group_0 span.channel').click(function(){
			var groupOutChlID =[];
			$('#group_0 span.channel.sel').each(function(){
				groupOutChlID.push($(this).data('data').r_chl_group_id);
			})
			$('#r_chl_group_id_ID').val('').val(groupOutChlID.join(','));
			//show($('#r_chl_group_id_ID').val());
		})*/
		/*
		控件触发事件调用的元素事件绑定.
		控件触发事件在JS事件绑定触发的事件后面... 有待考证.
		暂时只用了 RemoveChannelFromGroup_ok AddChannelInGroupDouble_ok 这2个ID做实验
		*/

		$('#RemoveChannelFromGroup_ok').click(function(){
			var groupOutChlID =[];
			$('#group_0 span.channel.sel').each(function(){
				groupOutChlID.push($(this).data('data').r_chl_group_id);
			})
			$('#r_chl_group_id_ID').val('').val(groupOutChlID.join(','));
		})

		$('#AddChannelInGroupDouble_ok').click(function(){ 
			selectEdparent('group');
			SetChannelIntoGroupData();
		});
		 /*client_setting*/
		$('div.dev_list:eq(2)').on('click','span.channel',function(){  //回访设置通道点击
			FillChannleRecordTime($(this));			
		})
        /*$('#SettingRecordDoubleTimeParm_ok').click(function(){
			setTimeout(FillChannleRecordTime($('div.dev_list:eq(2) span.channel.sel')),1000);
		   
			});*/
		//搜索设备全选和 TR 同步选中
		$('tbody.synCheckboxClick').each(function(){
			$(this).SynchekboxClick();
		})

		$('#searchtxt').focus(function(){
			$(this).val('');
		})
		
		//文件排序
		$('#SerachDevList').prev('table').find('thead td[sort]').mousedown(function(){
			$(this).css('background-position','0 -20px');
			}).mouseup(function(){
				$(this).css('background-position','0 0');
				var type = $(this).attr('sort');
				if(type && devFile){
					$('#SerachDevList tbody').html('');
					var sortType ='';
					var node =$(this);
					if($(this).attr('sortType')){
						sortType ='Des';
						node.removeAttr('sortType');
					}else{
						sortType = 'Asc';
						node.attr('sortType',1);
					}
					SortSearchFile_2UI(type+sortType);
				}
			}).mouseout(function(){
					$(this).css('background-position','0 0');
			});
		//选择或者修改设备的类型
		$('#vendor_chl_num li').each(function(index){
			$(this).click(function(){
				if(index==0||index==1){
					$('#channel_count_ID').val(4);
				}else{
					$('#channel_count_ID').val(1);
				}
				
		    });
            
        });
			
		//本地设置中的录像设置
       // $('#otherCopy').click(function(){ $('#SettingRecordDoubleTimeParm_ok').click();});
		
		/*$('#RecordTime div.timeInput').on('blur','input:text',initRecrodxml);
		$('#RecordTime').on('click','input:checkbox',initRecrodxml);*/
		/*控件触发事件调用的元素事件绑定.*/

		//设备操作相关的事件绑定
		var oActiveEvents = ['AddArea','ModifyArea','RemoveArea','AddGroup','RemoveGroup','ModifyGroup','ModifyChannel'/*,'AddDevice'*/,'ModifyDeviceFeedBack',/*'ModifyDevice','RemoveDevice','AddDeviceDouble',*/'AddChannelDoubleInGroup','SettingStorageParm','SettingCommonParm','SettingRecordDoubleTimeParm','RemoveChannelFromGroup','ModifyGroupChannelName'/*,'AddDeviceAll','RemoveDeviceAll'*/,'AddDeviceFeedBack','RemoveDeviceFeedBack',];  //事件名称集合
		for (i in oActiveEvents){
			AddActivityEvent(oActiveEvents[i]+'Success',oActiveEvents[i]+'Success(data)');
			AddActivityEvent(oActiveEvents[i]+'Fail','Fail(data)');
		}
        AddActivityEvent('Validation','Validationcallback(data)');
		var userEvents = ['AddUser','ModifyUser','DeleteUser','ModifyCurrentLoginUser']
		for (i in userEvents){
			AddActivityEvent(userEvents[i]+'Success',userEvents[i]+'Success(data)');
			AddActivityEvent(userEvents[i]+'Fail','userfail(data)');
		}
		
		//搜索设备;
		oSearchOcx.AddEventProc('SearchDeviceSuccess','callback(data);');
		oSearchOcx.AddEventProc('SettingStatus','autoSetIPcallBack(data);');
		
		autoSearchDev.AddEventProc("useStateChange",'useStateChange(ev)');
		autoSearchDev.startGetUserLoginStateChangeTime();
		
		onvifuserInfo.AddEventProc("importUserInfoCallback",'importUserInfoCallback(data)');
		onvifuserInfo.AddEventProc("outportUserInfoCallback",'outportUserInfoCallback(data)');
		//onvif 设置
		onvifSetting.AddEventProc('operationStart','operationStart(data)');
		onvifSetting.AddEventProc('operationReturnInfo','operationReturnInfo(data)');
		onvifSetting.AddEventProc('operationEnd','operationEnd(data)');
		initOxcDevListStatus();
	})///

	///$(window).resize(set_contentMax);

	var SplitScreenMode={'div1_1':'1','div2_2':'4','div6_1':'6','div8_1':'8','div3_3':'9','div4_4':'16','div5_5':'25','div6_6':'36','div7_7':'49'/*,'div8_8':'64'*/}
		for(i in SplitScreenMode){
			SplitScreenMode[i]=SplitScreenMode[i]+_T('Screen');
		}
	function FillCommonParmData(){ 
		var item = ['Language','AutoPollingTime','SplitScreenMode','AutoLogin','AutoSyncTime','AutoConnect','AutoFullscreen','BootFromStart'];
		for(var i in item){
			var str = oCommonLibrary['get'+item[i]]();
			//console.log(item[i]+'----------------'+str);
			var obj = $('#'+item[i]+'_ID')
			if(obj.is(':checkbox')){
				obj.dataIntoSelected(str);	
			}else{
				obj.dataIntoVal(str)
				$('#'+item[i]).dataIntoVal(str);
			}
		}
       $('#BootPersian_ID').dataIntoSelected(document.getElementById('commonLibrary').getIsPersian());
		//$('#SplitScreenMode').val(SplitScreenMode[$('#SplitScreenMode').val()]);
		$('#SplitScreenMode').val(oCommonLibrary.getSplitScreenMode());
		$('#CommonParm input:checkbox').each(function(){
			$(this).toCheck();
		})

		_t($('#CommonParm input:text'));
		/*$('#Language_ID').nextAll('li').click(function(){
		 	$('#Language_ID').val($(this).attr('value'));
		})*/

		/*$('#viewMod').on('click','a',function(){
		 	$('#SplitScreenMode_ID').val($(this).attr('value'));
		 	$('#viewMod').prev('div').find('span.SplitScreenMode').html($(this).html().match(/\d+|[\u4e00-\u9fa5]+/g).join(''));
		})*/

		//areaList2Ui();
	}
	//record setting  回放设置;
	var weeks = [_T('Mon'),_T('Tue'),_T('Wed'),_T('Thu'),_T('Fri'),_T('Sat'),_T('Sun')];
	function FillRecordTimeData(){
		/*SettingRecordDoubleTimeParm();
		FillChannleRecordTime($('div.dev_list:eq(2) span.channel:first'));*/
		areaList2Ui();
		/*SettingRecordDoubleTimeParm();*/
		_t($('#RecordTime input:text'));
		FillChannleRecordTime($('div.dev_list:eq(2) span.channel:first'));
	}

	function FillChannleRecordTime(obj){
		SettingRecordDoubleTimeParm();  //清空回放表单的数据


		//通道选中状态唯一
		$('div.dev_list:eq(2) span.channel').removeClass('sel');
		obj.addClass('sel');
		//填充完拷贝至其他通道的下拉菜单
		$('td.copyTo li:gt(0)').remove();
		
		var devData = obj.parent('li').parent('ul').siblings('span.device').data('data');
		
		//var allChlID = [];
		var allDevID=[];  //所有通道ID
		
		 $('div.dev_list:eq(2) span.channel').each(function(){
			var devData = $(this).data('data');
			allDevID.push(devData.channel_id);
			})
		//拷贝到所有通道选项的value写入;
		
		//$('<li  style="width:100px"><input class="all" data="" value="'+_T('select')+'" disabled="disabled" /></li>').find('input').attr('data',allChlID.join(',')).end().appendTo($('td.copyTo ul'));
        //拷贝到所有设备的所有通道上
		$('<li  style="width:200px"><input style="width:200px" class="all" data="" value="'+_T('Select')+'" disabled="disabled" /></li>').find('input').attr('data',allDevID.join(',')).end().appendTo($('td.copyTo ul'));
		//console.log(allChlID);
		
		obj.parent('li').parent('ul').children('li').each(function(){
			var chlData = $(this).find('.channel').data('data');
			 
			$('<li style="width:200px"><input style="width:200px" data="'+chlData.channel_id+'" value="'+devData.device_name+'_'+chlData.channel_name+'" disabled="disabled" /></li>').appendTo($('td.copyTo ul'));
			//allChlID.push(chlData.channel_id);
		})
 
		var chlData = obj.data('data');

		var sTimeID = oCommonLibrary.GetRecordTimeBydevId(chlData.channel_id); //获取该通道的时间段的ID列表
		/*console.log("---获取该通道的时间段的ID列表---"+sTimeID);*/
		for(var i in sTimeID){
			var sTimeIDdata = oCommonLibrary.GetRecordTimeInfo(sTimeID[i]); //获取该时间段的详细信息
			//console.log("通道"+sTimeIDdata.chl_id+"//时间段id:"+sTimeIDdata.schedle_id+"//星期："+sTimeIDdata.weekday+"//开始时间 结束时间："+sTimeIDdata.starttime+' '+sTimeIDdata.endtime+"//类型："+sTimeIDdata.enable);
			//时间段数据和对应的星期关联
			for(var j in weeks){
				if(j == sTimeIDdata.weekday){
				   $('#RecordTime ul.week.option li:eq('+j+')').data('data_'+sTimeID[i],sTimeIDdata);
				 /* console.log( $('#RecordTime ul.week.option li:eq('+j+')').data('data_'+sTimeID[i]));*/
				}
			}
		}
		var week_date = new Date().getDay();
		    week_date = week_date==0 ? 6 : parseInt(week_date,10)-1;
		var dateobj = $('#RecordTime ul.week.option li').eq(week_date);
		$('#week').val(dateobj.find('input').val());
		$('#week').attr('data',dateobj.find('input').attr('data'));
		initChannlrecTime(dateobj); //填充具体时间到页面
		$('#RecordTime ul.week.option li').each(function(index){
			$(this).on('click',function(){
				initChannlrecTime($(this));
			})
		})
		
	}
	function initChannlrecTime(obj){ //初始化计划录像的XML信息
       // console.log(obj.data());
		var oTimes=$('#recordtime tbody tr:gt(0):lt(5)');
		var str = '<recordtime num="4">';

		for(var i in obj.data()){
			var data = obj.data()[i];
			var timeid = i.split('_')[1];
			var start = data.starttime.split(' ')[1];
			var end = data.endtime.split(' ')[1]
			var enable = data.enable;
			var n = data.schedle_id;
			
			oTimes.eq(n).find('input.timeid').val(timeid);
			/*oTimes.eq(n).find('div.timeInput:eq(0)').timeInput({'initTime':start});
			oTimes.eq(n).find('div.timeInput:eq(1)').timeInput({'initTime':end});*/
			oTimes.eq(n).find('input.time_picker:eq(0)').val(start).end().find('input.time_picker:eq(0)').attr("data",start);
			oTimes.eq(n).find('input.time_picker:eq(1)').val(end).end().find('input.time_picker:eq(1)').attr("data",end);
			for(var j = 0; j < 2; j++)
			{
				if(enable == 0)
				{
					oTimes.eq(n).find('input:checkbox').eq(j).prop("checked",false);
				}else{
					
				  oTimes.eq(n).find('input:checkbox').eq(j).prop("checked",Boolean((enable & (1 << j))));
				  //oTimes.eq(n).css('border','1px red solid');
				// console.log((n)+' '+(enable & (1 << j)));
				}
			}			
			str+='<num'+n+' recordtime_ID="'+timeid+'" starttime_ID="1970-01-01 '+start+'" endtime_ID="1970-01-01 '+end+'" enable_ID="'+enable+'" />';
		}
		str +='</recordtime>';
	    // console.log(str);
		$('#recordtimedouble_ID').val(str);
		
		var objDe = $('td.copyTo ul li:first input');
		$('.copyTo .select input').val(objDe.val()).attr('data',objDe.attr('data'));
	}
	//添加前获取要修改的时间ID的类容XML;
	function getRecrodxml(to){
		var copyTo = [$('#RecordTime span.channel.sel').data('data').channel_id], // 初始的通道ID
			copyID = $('#RecordTime td.copyTo input:first').attr('data').split(','), // 要拷贝到的通道ID
			nowWeek = $('#week').attr('data').split(','), //要保存的星期
			nowWeekTimeID = [];
		    //console.log(nowWeek+"---nowWeek----");
		var copy = to==1 ? copyTo.concat(copyID) : copyTo;  // 要修改的通道的

		// 返回符合当天星期的时间ID
		for(i in copy){ 
			var timeID = oCommonLibrary.GetRecordTimeBydevId(copy[i]);
			//console.log(timeID+"---timeID--");
			for(j in timeID){
				var timeinfo = oCommonLibrary.GetRecordTimeInfo(timeID[j])
				for(k in nowWeek){
					if(timeinfo.weekday == nowWeek[k]){
						nowWeekTimeID.push([timeID[j],timeinfo.schedle_id]);
					}
				}
			}
		}
       // console.log(nowWeekTimeID+"----nowWeekTimeID-------");
		var str = '<recordtime num="'+nowWeekTimeID.length+'">';
		for( i in nowWeekTimeID){
			//var warp = $('#RecordTime tr.schedle_id:eq('+nowWeekTimeID[i][1]+')');
			var warp = $('#recordtime tbody tr:gt(0):lt(5)').eq(nowWeekTimeID[i][1]);
			var timeid = nowWeekTimeID[i][0];
			/*var start = warp.find('div.timeInput:eq(0)').gettime();
			var end = warp.find('div.timeInput:eq(1)').gettime();*/
			var start = warp.find('td .iput2.time_picker:eq(0)').val();
			var end = warp.find('td .iput2.time_picker:eq(1)').val();
			var enable  = 0;
	        warp.find('td input:checkbox').each(function(index){
			
			     if($(this).prop('checked')==true){ 
				 
					  enable += Math.pow(2, index);
					//console.log("---enable--"+enable+"--index--"+index);
				  }
            });
			
			str+='<num'+nowWeekTimeID[i][0]+' recordtime_ID="'+timeid+'" starttime_ID="1970-01-01 '+start+'" endtime_ID="1970-01-01 '+end+'" enable_ID="'+enable+'" />';
			//var str1 = '<num'+nowWeekTimeID[i][0]+' recordtime_ID="'+timeid+'" starttime_ID="1970-01-01 '+start+'" endtime_ID="1970-01-01 '+end+'" enable_ID="'+enable.toString()+'" />';
			//console.log(str1);
		}
		str +='</recordtime>';
		//console.log(str);
		to==1?$('#recordtimeCopy_ID').val(str):$('#recordtimedouble_ID').val(str);
		
	}

	// 全选
	function addAlldevIntoArea(){
		selectEdparent('area');
		var devList = $('#SerachedDevList input:checkbox').prop('checked',true);
		initDevIntoAreaXml(devList,$('#adddeviceall_ID'));
	}
	//添加多台设备
	function adddoubdevIntoArea(){
		selectEdparent('area');
		var devList = $('#SerachedDevList input:checked');
		initDevIntoAreaXml($('#SerachedDevList input:checked'),$('#adddevicedouble_ID'));
	}
	/*//一键添加
	function (){

	}*/
	//自动补全没没选中的对象    //针对  区域和分组
	function selectEdparent(sClass){
		var obj = $('ul.filetree:eq('+(sClass == 'group' ? 1 : 0)+')');
		if(!obj.find('span.'+sClass+'.sel')[0]){
			obj.find('span').removeClass('sel');
			obj.find('span.'+sClass+':first').addClass('sel');
		}
	}
	//初始化要添加到区域的XML信息
	function initDevIntoAreaXml (objList,obj){
			var oArea=$('div.dev_list:eq(0)')
			var areaID = oArea.find('span.sel').data('data')['area_id'] || 0;
			var str = '<devListInfo cut = "'+objList.length+'" area_id="'+areaID+'">';
			objList.each(function(){ 
				var data = $(this).parent('td').parent('tr').data('data');
				var dataStr = '<dev username="admin" password="" ';
				for(i in data){ 
					dataStr+=i+'="'+data[i]+'" ';
				}
				dataStr+=' />';
				str+=dataStr;
			});
			str+=' </devListInfo>';
			obj.val(str);
		}
	function FillStorageParmData(){
		var diskcheckbox = $('#Storage_Settings input:checkbox')
		var disks = oCommonLibrary.getEnableDisks().split(':');
		for(i in disks){ 
			diskcheckbox.each(function(){ 
				if(disks[i] == $(this).val()){ 
					$(this).prop('disabled',false);
				};
			})
		}
		var item = ['FilePackageSize','LoopRecording','DiskSpaceReservedSize']
		var useDisks = oCommonLibrary.getUseDisks().split(':');
		var clickedAbles = diskcheckbox.filter(function(){ 
			return $(this).is(':enabled');
		})
		for( n in useDisks){ 
			var b = true;
			clickedAbles.each(function(){ 
				$(this).dataIntoSelected(useDisks[n]);
				if(!$(this).is(':checked')){
					b = false;	
				}
			})
			$('#Storage_Settings_SelectAll').prop('checked',b);
		}


		for( k in item){ 
			var str = oCommonLibrary['get'+item[k]]();
			var obj = $('#'+item[k]+'_ID');
			if(obj.is(':checkbox')){
				obj.dataIntoSelected(str);
			}else{ 
				obj.dataIntoVal(str).dataIntoHtml(str);
			}
		}

		/*clickedAbles.each(function(){
			$(this).click(function(){
				var checked = []; 
				clickedAbles.each(function(){ 
					$(this).is(':checked') == true && checked.push($(this).val());
					$('#UseDisks_ID').val(checked.join(':'));
				})
			})
		})*/
		$('#LoopRecording_ID').toCheck();
	}
	
	//组合勾选的硬盘版格式以用于保存。  // C:D:E
	function SettingStorageParmData(){
		var UseDisks = [];
		$('#Storage_Settings :checked').each(function(){
			UseDisks.push($(this).val());
		})	
		$('#UseDisks_ID').val(UseDisks.join(':'));
	}

	//分组区域添加到分组, 数据组织
	function SetChannelIntoGroupData(){
		var oChlList = $('div.dev_list:eq(0) span.sel.channel');
		var oArea = $('div.dev_list:eq(1) span.sel.group');
		var str = '<chlintogroup cut="'+oChlList.length+'" group_id="'+oArea.data('data')['group_id']+'">';
		oChlList.each(function(){ 
			var sChl = '<chlinfo ';
			var data = $(this).data('data')
			for(i in data){ 
				if(i == 'channel_name'){
					sChl+='channel_name_ID="'+$('#dev_'+data.dev_id).data('data').name+'_chl_'+(parseInt(data.channel_number)+1)+'" ';	
				}else{
					sChl+=i+'_ID="'+data[i]+'" ';
				}
			}
			sChl+='/>';
			str+=sChl
		})
		str+='</chlintogroup>';
		$('#addchannelingroupdouble_ID').val(str);
	}

	function emptyDevSetMenu(){
		//console.log('清空表单的数据');
		//$('#dev_'+nowDev._ID).addClass('sel').parent('li').siblings('li').find('span').removeClass('sel');

		//$('#set_content div.switch input[class]').val('').prop('checked',false);

		$('#set_content .right_content:eq(1) div.switch').find('input[data-UI]:text,input[data-UI]:password,input[data-UI][type="select"]').val('').attr('data','')
									  		  .end().find(':checkbox,:radio').prop('checked',false);
		$('#ajaxHint').html('').stop(true,true).hide();
		$('#set_content .right_content:visible div.switch').find('input').css('border',0).removeAttr('b');
		 showAJAXHint().hide();;
		//$('#ajaxHint').stop(true,true).css('top',targetMenu.height() + 46).html(_T('loading')).show();
	}
	
	function set_contentMax(){
		var W = $(window).width(),
			H = $(window).height();
			W = W<1000?1000:W;
			H = H<600?600:H;

		$('#set_content div.left').height(H -106);
			
		$('div.right_content:eq('+key+')').show();

		var warp = $('#set_content div.right').css({ 
			width:W - 250,
			height:H - 106
		}).find('div.right_content:visible');

		if(key == 0){ 
			var main = $('#SerachDevList').css({ 
				height:H-270,
				width:W-600
			})

			main.find('thead td').not(':first,:last').width((main.width()-130)/2)
			theadtbody(main.find('thead td'),main.prev('table').find('thead td'));

			$('#Allocation').css({
				top:main.height()+18,
				width:main.width()
			}).find('tbody .aa').width((main.width()-176)/2)

			warp.find('div.dev_list').height(warp.height()-6)

			warp.find('div.action:eq(0)').css('left',main.width()-30);

			$('#left_list').css('left',main.width()+118);

			//searchFlush();

		}else if(key == 1){
			/*var devLeft=0;
			warp.find('div.switch').each(function(){
				devLeft = $(this).width()>devLeft ? $(this).width(): devLeft + 2;
			})*/
			warp.find('div.dev_list').height(warp.height()-6);
		}
		
		$('#foot').css('top',$('#set_content div.right').height()+78);
	}
	//用户管理模块
	
	//清除用户信息
	function emptyUserinfo(){
		$('.userRight:visible input:checkbox').prop('disabled',false);
		$('.switch:visible input').not(':checkbox').val('');
		$('.switch:visible :checkbox').prop('checked',false);
		$('#cinterval').next().next().prop('disabled',true);
		$('#otherinterval').next().next().prop('disabled',true);
	}
	//用户管理信息填充
	function tableuserinit(objec){
			emptyUserinfo();
             var obj=$('#userList');
			objec.find('input').prop('checked',!objec.find('input').prop('checked'));
			var data =objec.data('data');
			//console.log(data);
			obj.find('tr').removeClass('sel');
			objec.addClass('sel');
			$('#userR').val(data.username);
			var types = parseInt(data.userlv,2);
          
			for(var j = 0; j < 12; j++){
				if(types == 0){
				  $('#mainRight ul input:checkbox').eq(j).prop("checked",false);
				  break;
				}else{			
				  $('#mainRight ul input:checkbox').eq(j).prop("checked",(types & (1 << j)));  
				}
			}
			if(types==1023){
			   $('#mainRight .selAll').prop('checked',true);
			}	
			$('#exitinterval').val(data.exittime);		
	        if(parseInt(data.exittime,10)>0){ 
			  $('#otherinterval').prop('checked',true);
			  $('#exitinterval').prop('disabled',false);
			 }
	}
	 //用户登录状态回调函数
    function useStateChange(ev){
		//console.log(ev);
		if(ev.status==0){
		 $('.top_nav p span:eq(1)').html(ev.userName);	
		}else{
		  $('.top_nav p span:eq(1)').html(_T("not_Login"));
		}
		var index = $('.right_content:visible').index();
		if(index==3){
			$('.right_content:visible ul.ope_list li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct');
						$('.right_content:visible div.switch').hide();
						$('.right_content:visible div.switch').eq(0).show();
			cUserinfo2Ui();
		}
	}

	//当前已登录用户信息填充
	function cUserinfo2Ui(){
		emptyUserinfo();
		var obj=$('.userRight1');
		
		var cusername = autoSearchDev.getCurrentUser();
		var cuserlimit = autoSearchDev.getUserLimit(cusername);
		var exittime = autoSearchDev.getLoginOutInterval(cusername);
		if(cusername){
			//console.log("name:"+cusername);
			//console.log("limit:");
			//console.log(cuserlimit);
			//console.log("exittime:"+exittime);
			$('#cuser').val(cusername);
			$('#exitTime').val(exittime);
			var types = parseInt(cuserlimit.mainLimit,2);
			for(var j = 0; j < 12; j++){
				if(types == 0){
				 obj.find('input:checkbox').eq(j).prop("checked",false);
				  break;
				}else{			
				  obj.find('input:checkbox').eq(j).prop("checked",(types & (1 << j)));  
				}
			}
			obj.find('input:checkbox').prop('disabled',true);
			if(exittime>0){
				$('#cinterval').prop('checked',true);
				$('#exitTime').prop('disabled',false);
				
			}
		}else{
			//$('#userHint').html("无用户登录").show().css('top',$('.switch:visible').height() + 26).fadeOut(2000);
			autoSearchDev.showUserLoginUi(336,300); 
		}
	}
	//修改已登录用户的信息xml
	function getModcUserXml(){
		var name = $('#cuser').val();
		var oldpwd =$('#coldpwd').val();
		var newpwd = $('#cnewpwd').val();
		var logout = $('#cinterval').prop('checked')? $('#exitTime').val() : 0;
		var str="<modifyCurrentUserInfo sNewUserName='"+name+"' sOldPassword='"+oldpwd+"' sNewPassword='"+newpwd+"' sLogOutInterval='"+logout+"'></modifyCurrentUserInfo>";
		//console.log(str);
		$('#modifyCurrentUserEx_ID').val(str);
	}
	//用户设置方法 User Manage
	function userList2Ui(){
		//$('#userR').val('');
		//$('#pwdR').val('');
		emptyUserinfo();
		
		$('#userList tr').remove();
		 
		//$('#mainRight input').each(function(){$(this).prop('checked',false)});
		var userList = autoSearchDev.getUserList();
		var cuser = autoSearchDev.getCurrentUser();
		if(userList.length>0){  //避免数组为空的时候. 自己写的JS数组扩展方法引起 BUG;
		  //var fragment = document.createDocumentFragment();
			for(var i in userList){
				var userlv = autoSearchDev.getUserLimit(userList[i]);
				var userid = autoSearchDev.getUserInDatabaseId(userList[i]);
	            var logoutinterval = autoSearchDev.getLoginOutInterval(userList[i]);
				var data= {'username':userList[i],'userid':userid,'userlv':userlv.mainLimit,'exittime':logoutinterval};
			   // console.log(data);
				 userList[i]!=cuser && $('<tr><td><input type="checkbox" />'+userList[i]+'</td><td></td></tr>').appendTo('#userList').data('data',data);
				/*if(userid!=-1){
					var newItem = $('<tr><td><input type="checkbox" />'+userid+'</td><td>'+userList[i]+'</td></tr>');
					newItem.data('data',data);
					fragment.appendChild(newItem);
				}*/
			}
			//$('#userList')[0].appendChild(fragment);
			
		}
		
		  var warp = $('#UserList');

        if($('#userList').height() >= warp.height() && !warp.attr('b')){
            theadtbody(warp.find('thead td'),warp.prev('table').find('td'));
            warp.attr('b',0);
        }
		tableuserinit($('#userList tr:first'));		
	}
	
	
	//添加用户前获取xml
	function getAddUserXml(){
		var name = $('#userR').val();
		var pwd = $('#pwdR').val();
		var logoutinterval = $('#otherinterval').prop('checked')? $('#exitinterval').val() : 0;
		var limit=0;
		var c=0;
		var str1='';
		for(var i=0;i<12;i++){
		   if($('#mainRight li input:checkbox').eq(i).prop('checked')){
				 c++;

			    str1+="<sub mainCode='"+(1<<i).toString(2)+"' subCode='0'/>";
				limit+=1<<i;
		   }	
		}
		//console.log("limit:"+limit);
		var str="<main sUserName='"+name+"' sPassword='"+pwd+"' limitCode='"+limit.toString(2)+"' subLimitNum='"+c+"' sLogOutInterval='"+logoutinterval+"'>";
			str+=str1;
			str+="</main>";
		$('#addUserEx_ID').val(str);
		//console.log("getAddUserXml:"+str);
	}
	//修改用户前获取xml
	function getModUserXml(){
		var oldname=$('#userList tr.sel').data('data').username;
		var newname=$('#userR').val();
		var pwd=$('#pwdR').val();
		var logoutinterval = $('#otherinterval').prop('checked')? $('#exitinterval').val() : 0;
		var limit=0,c=0,str1='';
		for(var i=0;i<12;i++){
		   if($('#mainRight li input:checkbox').eq(i).prop('checked')){
				 c++;
			     str1+="<sub mainCode='"+(1<<i).toString(2)+"' subLimit='0'/>";
			     limit+=1<<i;
		   }	
		}
		var str=" <modify>";
	        str+="<mainInfo sOlduserName='"+oldname+"' sNewUserName='"+newname+"' sNewPassword='"+pwd+"' uiNewLimit='"+limit.toString(2)+"' sLogOutInterval='"+logoutinterval+"' subLimitSize='"+c+"'>";
            str+=str1;
	        str+="</mainInfo>";
	        str+="</modify>";
           $('#modifyUserEx_ID').val(str);
		//  console.log("getModUserXml:"+str);
	
	}
	//删除用户前获取xml
	function getDelUserXml(){
		var name = [],str1='';
		$('#userList input:checked').each(function(){
			var userdata = $(this).parent('td').parent('tr').data('data');
			 name.push(userdata.username);
			 str1+="<sub username='"+userdata.username+"'/>";
			});
		var str=" <del size='"+name.length+"'>";
	        str+=str1;
	     	str+="</del>";
      $('#deleteUserEx_ID').val(str);
		//console.log("getDelUserXml:"+str);
		
	}
	//修改当前用户
	function ModifyCurrentLoginUserSuccess(data){
		
		cUserinfo2Ui();
		
	}
	//
	function ShowUserOperateMenu(obj){  //显示弹出层 调整定位。
		var node = $('#'+obj+'');
		node.find('input:text,:password').val('');
		if(obj == 'menu3' || obj == 'confirm'){
			var str = $('table.UserMan input:hidden').val().split(',');
			if(str == ''){ 
				Confirm(_T('please_select_user'));
				return;
			}
			if(obj == 'menu3'){
				node.find('input:first').val(str[0]);
			}
		}
		objShowCenter(node);
	}

	function pwdTest(obj){
		var str = obj.val();
		if(str == '' || !/[\d\w_]{4,}$/.test(str)){
			Confirm(_T('Please_use_a_combination_of_alphanumeric_characters_and_underscores_length_greater_than_6'));
		}else{
			obj.attr('enable','1');
			valueIsSame($('#add_again_passwd'),$('#add_passwd'));
			valueIsSame($('#modify_again_passwd'),$('#modify_newpasswd'));
		}
	}
	function valueIsSame(obj,obj2){ 
		if(obj.val() && obj2.val() && obj.val() != obj2.val()){ 
			obj.attr('enable','0');
			obj2.attr('enable','0');
			Confirm(_T('twice_the_password_is_not_the_same'));
			return;
		}else{ 
			obj.attr('enable','1')
			obj2.attr('enable','1')
		}
	}
	function showMenu(id,str){
		if(str){ 
			$('#'+id).find('p span').html(str);
		}
		$('#iframe').show();
		objShowCenter($('#'+id));
	}
	function initModifyMenu(){ 
		var oDevData = $('#SerachDevList tbody tr').filter(function(){ 
			return $(this).find('input:checkbox').is(':checked')
		}).eq(0).data('data');
	}
	function removeArry(obj,number){ 
		for(i in obj){ 
			if(obj[i] == number){ 
				obj.splice(i,1);
			}
		}
		return obj;
	}
//设备搜索. 设备设置. 分组设置. 区域设置.
function showContextMenu(y,x,obj){
	var menu = $('#menu0')
	menu.find('li').off();
	if(obj[0].nodeName == 'SPAN'){
		if(obj[0].id == 'area_0' || obj[0].id == 'group_0' ){
			$('#menu0 li:gt(1)').hide();
		}else if(obj.hasClass('group')){
			menu.find('li:lt(1)').hide();
		}else if(obj.hasClass('device')){ 
			menu.find('li:lt(2)').hide();
		}else if(obj.hasClass('channel')){ 
			menu.find('li').not(':eq(2)').hide();
			menu.find('li:eq(2)').show();
		}	
		menu.find('li:eq(2)').one('click',function(){
			showObjActBox('Modify',obj.attr('class').split(' ')[0]);
		})
		menu.find('li:last').one('click',function(){ 
			showObjActBox('Remove',obj.attr('class').split(' ')[0]);
		})
	}else{ 
		$('#menu0 li:gt(1)').hide();
	}
	menu.css({
		top:y,
		left:x,					
	}).show(10,function(){ 
		$(document).mouseup(function(event){
			event.stopPropagation();
			menu.hide();
			$(document).off();
		})
	});
}

//遮罩层和弹出框方法.
var trance = {'area':'area','device':'device','channel':'channel','group':'group','Add':'add','Remove':'delete','Modify':'modify','GroupChannelName':'Channel_under_the_device_name'};
function showObjActBox(action,objclass){  //右键弹出菜单UI调整
	var pObjClass = objclass == 'group' ? 'group':'area';
	var pObj = $('span.sel');
	if(action == 'Add'){ // 调整添加的父级对象
		if(!$('span.sel')[0] || !$('span.sel').hasClass(pObjClass)){
			pObj = $('span.'+pObjClass+':eq(0)');
		}
		if(objclass == 'group'){
			pObj = $('#group_0');
		}
	}else{ 
		if(!$('span.sel')[0]){
			$('span.'+objclass+':eq(0)').addClass('sel');
		}
	}
	if(action == 'Remove'){ 
		var obox = $('#confirm');
	}else{
		var obox = $('#'+objclass);
	}
	obox.find('p span').html(_T(trance[action]+'_'+trance[objclass]));
	initActionBox(action,pObj,obox,objclass);
}
function initActionBox(action,pObj,obox,objclass){  //右键菜单数据填充.
	
	if(action=='Add' && objclass =='device'){
		var str1 ='{"address":"","port":"80","http":"80","eseeid":"","username":"admin","password":"","device_name":"","channel_count":"4","parea_name":"'+ pObj.data('data').area_name+'","area_id":"'+ pObj.data('data').area_id+'","vendor":"DVR"}';
	     var file =eval('('+str1+')');
		 //console.log(file);
	}else{
		var file = pObj.data('data');
	}
	var data = file;
	//var data =pObj.data('data');
	/*console.log('填充数据');
	console.log('----------------');
	console.log(data);*/
	if(!data){ 
		Confirm(_T('Please_select_a_device'));
		return ;
	}
	var pObjType = firstUp(pObj.attr('class').split(' ')[0]);
	if(pObj.attr('id')=='g_channel_'+data.channel_id){
		pObjType = 'GroupChannelName';
		/*obox.find('input:text').attr('id','r_chl_group_name_ID');
	}else if(pObj.attr('id')=='channel_'+data.channel_id){ 
		obox.find('input:text').attr('id','channel_name_ID');*/
	}
	$('#'+action+pObjType+'_ok').show();
	/*if(pObj.parent('li').parent('ul').prev('span').hasClass('group')){ 
		$('#channel input:text').attr('id','r_chl_group_name_ID');
	}else{ 
		$('#channel input:text').attr('id','channel_name_ID');
	}*/
	for(i in data){
		console.log();
		if(i.match(objclass+'_name') && action == 'Remove'){
			$('#confirm h4').attr('id',i+'_ID').html(_T('delete')+data[i]);
		}
		var str = data[i];		

		if($('#'+i+'_ID')[0]){ 
			$('#'+i+'_ID').val(str);
			$('#device span[dataAction="'+i+'_ID"]').html(str).attr('value',str);
		}else{ 
			$('<input type="hidden" class="data" value="'+str+'" id="'+i+'_ID"/>').appendTo(obox);
		}
	}
	if(action == 'Add'){
		$('#'+action+firstUp(objclass)+'_ok').show();
		$('#'+objclass+'_name_ID').val('');
		$('#area #'+objclass+'_id_ID').remove();
		$('#group #'+objclass+'_id_ID').remove();
		//$('#pid_ID').val(data['area_id']);
		//obox.find('input.parent'+pObjType).val(data['area_name']);
	}
	objShowCenter(obox);
}
function cleanDev(){  //清空设备
	var arr = [];
	$('div.dev_list span.device').each(function(){
		arr.push($(this).data('data').dev_id);
	})
	$('#removedeviceall_ID').val(arr.join(','));
	searchFlushReal();
}
function setIP(){ //设置IP
	var oData = $('#SerachedDevList tr.sel').data('data')
	if(oSearchOcx.SetNetworkInfo(oData.SearchDeviceId_ID,$('#SearchIP_ID').val(),$('#SearchMask_ID').val(),$('#SearchGateway_ID').val(),oData.SearchMac_ID,$('#SearchHttpport_ID').val(),'admin','')){
		Confirm(_T('IP_setup_failed'));
	}else{
		setTimeout(searchFlush,2000);
	}
}
function autoSetIP(){  //批量分配IP
	var ipcList= $('#SerachDevList tr.IPC')
	var str = '<devnetworkInfo Num="'+ipcList.length+'">'
	ipcList.each(function(){
		var oIpcData = $(this).data('data');
		str+='<dev sDeviceID="'+oIpcData.SearchDeviceId_ID+'" sAddress="'+oIpcData.SearchIP_ID+'" sMask="'+oIpcData.SearchMask_ID+'" sGateway="'+oIpcData.SearchGateway_ID+'" sMac="'+oIpcData.SearchMac_ID+'" sPort="'+oIpcData.SearchHttpport_ID+'" sUsername="admin" sPassword=""/>'
	})
	str+='</devnetworkInfo>';
	$('#AutoSetNetworkInfoID').val(str);

	oSearchOcx.AutoSetNetworkInfo();
	setTimeout(searchFlush,2000);
	
}
function autoSetIPcallBack(data){

}

//// oCommonLibrary, 操作做数据库回调方法.
    
	function importUserInfoCallback(data){
		console.log(data);
		 var wrap = $('#set_content div.switch:visible'),t;
	   if(data.flags=='true'){
		 t=_T('importsuccess');
	    }else{
		  t=_T('importfail');	
		}
	  $('#ImOutportHint').html(t).stop(true,true).show();
	  $('#ImOutportHint').css('top',wrap.height()+46).fadeOut(2000);
    }

   function outportUserInfoCallback(data){
	   var wrap = $('#set_content div.switch:visible'),t;
	   if(data.flags=='true'){
		 t=_T('exportsuccess');
	    }else{
		  t=_T('exportfail');	
		}
	  $('#ImOutportHint').html(t).stop(true,true).show();
	  $('#ImOutportHint').css('top',wrap.height()+46).fadeOut(2000);
   }
	function Validationcallback(data){ //id按钮权限验证
	  //console.log(data);
		if(data.ErrorCode=="1"){
			//autoSearchDev.showUserLoginUi(336,300);
		}else if(data.ErrorCode=="2"){
			closeMenu();
			confirm_tip(_T('no_limit'));
			var timer =setTimeout(function(){
				closeMenu();
				clearTimeout(timer);
			},2000);
		}
	}
	
	
	function AddUserSuccess(data){
         userList2Ui();
	}
	function DeleteUserSuccess(data){
		userList2Ui();
	}

	function ModifyUserSuccess(data){
		userList2Ui();
	}
    function userfail(data){
		Confirm(_T('Operation_failed'));
	}
	function Fail(data){
		//console.log(data);
		/*var str='';
		if(data.name){
			str +=data.name+': ';
		}else if(data.channelname){ 
			str +=data.name+': ';
		}
		str += data.fail*/
		Confirm(_T('save_failed'));
	}

	//搜索设备控件方法.
	function searchFlushReal(){
		$('#SerachDevList tbody tr').remove();
		devFile =[];
		oSearchOcx.Flush();
	}
	
	function searchFlush(){
		
		oSearchOcx.Stop();
		$('#SerachDevList tbody tr').remove();	
		devFile =[]; 
		oSearchOcx.Start();
		/*setTimeout(function(){
			oSearchOcx.Stop();
		},5000)*/
	}
	//设备搜索回调函数
	var devFile =[];
	function callback(data){
		var bUsed = 1;
		/*$('#SerachDevList tbody tr').each(function(){ 
			if(parseInt($(this).find('td:eq(1)').html()) == data.SearchSeeId_ID || $(this).find('td:eq(2)').html() == data.SearchIP_ID){
				bUsed =0;
			}
		})*/
		var key = data.SearchSeeId_ID ?  data.SearchSeeId_ID : data.SearchIP_ID;
		/*$('div.dev_list:eq(0) span.device').each(function(){ 
			if($(this).data('data')['eseeid'] == data.SearchSeeId_ID){ 
				bUsed = 0;
			}
		})*/

		if(searchDevAvailable(key)){
			var id = data.SearchSeeId_ID > 1 ? data.SearchSeeId_ID : data.SearchIP_ID.replace('.','-');
			$('<tr id="esee_'+id+'" class="'+data.SearchVendor_ID+'"><td><input type="checkbox" />'+data.SearchVendor_ID+'</td><td>'+data.SearchSeeId_ID+'</td><td>'+data.SearchIP_ID+'</td><td>'+data.SearchChannelCount_ID+'</td></tr>').appendTo($('#SerachDevList tbody')).data('data',data);
			devFile.push(data);
			//console.log('devFile:'+data.SearchIP_ID);
			//initDevIntoAreaXml($('#SerachDevList tbody input:checkbox'),$('#adddevicedouble_ID'));
		}


		var warp = $('#SerachDevList');

		if($('#SerachedDevList').height() > warp.height() && !warp.attr('b')){
			theadtbody(warp.find('thead td'),warp.prev('table').find('td'));
			warp.attr('b',0);
		}
				
	}
	function AddAreaSuccess(data){
		var name = $('#area_name_ID').val();
		var pid = $('#pid_ID').val();
		var pidname = oCommonLibrary.GetAreaName(pid) || _T('Area');
		var id = data.areaid;
		var add = $('<li><span class="area" id="area_'+id+'">'+name+'</span><ul></ul></li>').appendTo($('div.dev_list:eq(0) #area_'+pid).next('ul'));
		add.find('span.area').data('data',{'area_name':name,'pareaname':pidname,'area_id':id,'pid':pid});
		$('ul.filetree:eq(0)').treeview({add:add});
		closeMenu();
	} 

	function RemoveAreaSuccess(){
		var id = $('#confirm input:hidden').val();
		var devList = [];
		if(id != 0){
			$('#area_'+id).next('ul').find('span.device').each(function(){
				devList.push($(this).data('data')['dev_id']);
			});
			$('ul.filetree:eq(0)').treeview({remove:$('#area_'+id).parent('li')});
		}
		for(i in devList){
			$('ul.filetree:eq(1) span.channel').each(function(){ 
				if($(this).data('data')['dev_id'] == devList[i]){
					$('ul.filetree:eq(1)').treeview({remove:$(this).parent('li')});
				}	
			})
		}
		closeMenu();
	}

	function ModifyAreaSuccess(){
		var oArea = $('#area_'+$('#area_id_ID').val());
		var sNewName = $('#area_name_ID').val()
		oArea.html(sNewName);
		oArea.data('data')['area_name']=sNewName;
		closeMenu();
	} 
	function AddGroupSuccess(data){
		var name = $('#group_name_ID').val();
		var id = data.groupid;
		var add = $('<li><span class="group" id="group_'+id+'">'+name+'</span><ul></ul></li>').appendTo('#group_0');
			add.find('span.group').data('data',{'group_id':id,'group_name':name});
			$('ul.filetree:eq(1)').treeview({add:add});
		closeMenu();
	}
	function RemoveGroupSuccess(data){ 
		var id=$('#group_id_ID').val();
		/*$('#group_'+id).next('ul').find('span.channel').each(function(){ 
			var devid = $(this).data('data')['dev_id'];
			var add = $(this).parent('li').appendTo($('div.dev_list:eq(0) #dev_'+devid).next('ul'));
			$('ul.filetree').treeview({add:add});
		})*/
		$('ul.filetree').treeview({remove:$('#group_'+id).parent('li')});
		closeMenu();
	}
	function ModifyGroupSuccess(data){
		var id=$('#group_id_ID').val();
		var name = $('#group_name_ID').val();
		$('#group_'+id).html(name);
		$('#group_'+id).data('data')['group_name'] = name;
		closeMenu();
	}

	function ModifyChannelSuccess() {
		var id= $('#channel_id_ID').val();
		var oChannel=$('#channel_'+id)
		var name =$('#channel_name_ID').val();
		oChannel.html(name);
		oChannel.data('data')['channel_name'] = name;
		closeMenu();
	}
	function AddDeviceFeedBackSuccess(data){
		// /areaList2Ui(0);
		closeMenu();
		$('#SerachDevList tbody tr').filter(function(){
			return $(this).find('input').is(':checked');
		}).remove();
		var succeedId = data.succeedId.split(';');
		for(i in succeedId){
			if(succeedId[i]){
				adddev(succeedId[i])
			}
		}
		searchEdDev();
		data.total == 0 && Confirm(_T('add_failed'));
	}
	function RemoveDeviceFeedBackSuccess(data){
		/*console.log('RemoveDeviceFeedBackSuccess');
		console.log(data);*/
		//areaList2Ui(0);
		closeMenu();
		var succeedId = data.succeedId.split(';');
		var b = succeedId.length > 10 ? 1 : 0;
		for(i in succeedId){
			if(succeedId[i]){
				$('ul.filetree').treeview({remove:$('#dev_'+succeedId[i]).parent('li')});
				$('ul.filetree span.channel').each(function(){ 
					if($(this).data('data')['dev_id'] == succeedId[i]){
						$('ul.filetree:eq(1)').treeview({remove:$(this).parent('li')});
					}
				})
			}
		}
		
		searchEdDev();

		//console.log(searchedDev)
		if(b)
			searchFlushReal();
	}

	function adddev(devID){
		//data.device_name = data.device_name.replace(/-/g,'.');  // 用设备名做ID 名字中的.号转换
		//Confirm(data.device_name+'AddSuccess!');
		var obj = $('ul.filetree:eq(0)');

		var chlList = oCommonLibrary.GetChannelList(devID);

		var data = oCommonLibrary.GetDeviceInfo(devID);
			data.device_name = data.name;
			data.area_id = obj.find('span.area.sel').data('data').area_id;
			data.dev_id=devID;
			data.channel_count=chlList.length;
			data.parea_name = obj.find('span.area.sel').data('data').area_name;

		var add = $('<li><span class="device" id="dev_'+data.dev_id+'" >'+data.device_name+'</span><ul></ul></li>').appendTo($('#area_'+data.area_id).next('ul'));
		obj.treeview({add:add});
		add.find('span.device').data('data',data);

		for(i in chlList){
			var chldata = oCommonLibrary.GetChannelInfo(chlList[i]);
			var chlNum = parseInt(chldata.number)+1;
			var name = 'chl_'+chlNum;
			var chldata={'channel_id':chlList[i],'dev_id':data.dev_id,'channel_number':chlNum,'channel_name':name,'stream_id':'0'};
			var addchl = $('<li><span class="channel" id="channel_'+chlList[i]+'">'+name+'</span></li>').appendTo(add.find('ul'));
			addchl.find('span.channel').data('data',chldata);
			obj.treeview({add:addchl});
		}
	}
	/*function AddDeviceSuccess(data){  //单个添加设备.. 菜单添加设备
		alert(data);
		var dataIndex={'area_id':'','address':'','port':'','http':'','eseeid':'','username':'','password':'','device_name':'','channel_count':'','connect_method':'','vendor':'','dev_id':data.deviceid,'parea_name':$('#parea_name_ID').val()}
		for(i in dataIndex){ 
			if(dataIndex[i] == ''){
				dataIndex[i] = $('#device #'+i+'_ID').val();
			}
		}
		adddev(dataIndex);	
		$('ul.filetree').treeview();
	}
	function AddDeviceAllSuccess(data){
		AddDeviceDoubleSuccess(data);
	}
	function AddDeviceDoubleSuccess(data){  //添加多个设备
		data.name = data.name.replace(/\./g,'-');  // 用设备名做ID 名字中的.号转换
		var area = $('div.dev_list:eq(0) span.sel:eq(0)').hasClass('area') ? $('div.dev_list:eq(0) span.sel:eq(0)') : $('div.dev_list:eq(0) span.area:eq(0)');
		var devData = $('#esee_'+data.name).data('data');
		var devData2={'area_id':area.data('data')['area_id'],'address':devData['SearchIP_ID'],'port':devData['SearchHttpport_ID'],'http':devData['SearchHttpport_ID'],'eseeid':data.name,'username':'admin','password':'','device_name':data.name,'channel_count':devData['SearchChannelCount_ID'],'connect_method':'0','vendor':devData['SearchVendor_ID'],'dev_id':data.deviceid,'parea_name':area.data('data')['area_name']};
		adddev(devData2);
		$('#esee_'+data.name).remove();
		$('#adddevicedouble_ID').val('');
		$('ul.filetree').treeview();
	}*/
	function ModifyGroupChannelNameSuccess(data){
		var id= $('#channel_id_ID').val();
		var oChannel=$('#g_channel_'+id)
		var name =$('#channel_name_ID').val();
		oChannel.html(name);
		oChannel.data('data')['channel_name'] = name;
		closeMenu();
	}
    function judgeIpId(type,obj){
		
		var str = obj.val(),hint='';
		switch(type){ 
		  case 0:
		      var pre = /(^((25[0-5])|(2[0-4]\d)|(1\d{2})|([1-9]\d)|[1-9])(\.((25[0-5])|(2[0-4]\d)|(1\d{2})|([1-9]?\d))){3}$)|(^([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,6}$)/;
				if(!pre.test(str)){
					hint=_T('correct')+_T('IP_domainName');
				}else{
					$('#device_name_ID').val(obj.val());
				}
			break;
		 case 1:
		       var b=true;
		       if(str == ''){
					hint=_T('correct')+_T('Esee_ID');
					b=false;
			   }

				if(/^\d+$/.test(str)){
					if(!/^[1-9][0-9]{7,}$/.test(str)){
						//console.log('数字');
						hint=_T('correct')+_T('Esee_ID');
						b=false;
					}
				}else{
					//console.log(/^\w+\d+$/.test(str)+'前缀11--------------'+str.length);
					if(!/^\w+\d+$/.test(str) || str.length > 11){
						//console.log('前缀');
						hint=_T('correct')+_T('Esee_ID');
						b=false;
					}
				} 
				if(b){
					$('#device_name_ID').val(obj.val());
					} 
		 break;
		}
		if(hint){
			type > 3 ? Confirm(hint) : Confirm(hint,true);
			obj.val('');	
		}
		}
	function VerifidevMenu(type,obj){
		var hint='',
			str = obj.value;
		//console.log('type:'+type+'---------------value:'+str);
		switch(type){   //  IP地址
			case 0:
				var pre = /^(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.)(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.){2}([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))$/;
				
				if(!pre.test(str)){
					hint=_T('correct')+_T('IP_format');	
				};
			break;
			case 1: //端口
				if(parseInt(str) > 65535 || parseInt(str) < 0 || !/^\d{1,5}$/.test(str)){
					hint=_T('correct')+_T('Port');		
				};
			break;
			case 2:  // eseeID
				if(str == '')
					hint=_T('correct')+_T('Esee_ID');

				if(/^\d+$/.test(str)){
					if(!/^[1-9]\d{8,}$/.test(str)){
						//console.log('数字');
						hint=_T('correct')+_T('Esee_ID');
					}
				}else{
					//console.log(/^\w+\d+$/.test(str)+'前缀11--------------'+str.length);
					if(!/^\w+\d+$/.test(str) || str.length > 11){
						//console.log('前缀');
						hint=_T('correct')+_T('Esee_ID');
					}
				}
			break;
			case 3:  //通道数。
				if(!(str == 1 || str == 4 || str == 8 || str == 16 ||  str == 24 ||  str == 32)){

					hint=_T('correct')+_T('Channels')+'(1,4,8,16,24,32)';
				}
			break;
			case 4:  //自动轮训时间
				if(!/^\d+$/.test(str) || parseInt(str) > 60 || parseInt(str)<1){
					hint=_T('correct')+_T('rotation_time');
				}
			break;
			case 5:  //文件大小
				if(!/^\d+$/.test(str) || parseInt(str) > 512 || parseInt(str)<10){
					hint=_T('correct')+_T('File_size_range');
				}
			break;
			case 6:  //磁盘预留空间
				if(!/^\d+$/.test(str) || parseInt(str) <= (32*128)){
					hint=_T('correct')+_T('Disk_space_reserved');
				}
			break;
		    case 7:
			  var nummin = parseInt(obj.attr('min'),10),
			      nummax = parseInt(obj.attr('max'),10),
				  num = parseInt(obj.val(),10);
			if(nummin>num || nummax<num){
				hint=T('correct_range',nummin,nummax);
		     }
			  break;
		}
		if(hint){
			if(type==7){
				
				if(hint){
					$(obj).css('border','1px dashed red').attr('b',1);
					showAJAXHint(hint);
				}else{
					$(obj).css('border','0').removeAttr('b');
					showAJAXHint().hide();
				}
				
			}else{
			  type > 3  ? Confirm(hint) : Confirm(hint,true);
			  obj.value = '';
			}
		}
		
		
	}

	function ModifyDeviceFeedBackSuccess(data){
		/*var dataIndex={'area_id':'','address':'','port':'','http':'','eseeid':'','username':'','password':'','device_name':'','channel_count':'','connect_method':'','vendor':'','dev_id':'','parea_name':$('#parea_name_ID').val()}
		for(i in dataIndex){
			dataIndex[i] = $('#'+i+'_ID').val();
		}
		$('#dev_'+dataIndex.dev_id).data('data',dataIndex).html(dataIndex.device_name);*/
		closeMenu();
		if(data.total!=0){
			areaList2Ui('','closed');
			reInitNowDev();	
		}else{
			Confirm(_T('Modified_device')+_T('Failed'))
		}
		
	}
	/*function ModifyDeviceSuccess(data){
		var dataIndex={'area_id':'','address':'','port':'','http':'','eseeid':'','username':'','password':'','device_name':'','channel_count':'','connect_method':'','vendor':'','dev_id':'','parea_name':$('#parea_name_ID').val()}
		debugData(dataIndex);
		for(i in dataIndex){ 
			dataIndex[i] = $('#'+i+'_ID').val();
		}
		$('#dev_'+dataIndex.dev_id).data('data',dataIndex).html(dataIndex.device_name);
		closeMenu();
	}*/
	/*function RemoveDeviceSuccess(){ 
		var id = $('#dev_id_ID').val();
		$('ul.filetree:eq(1) span.channel').each(function(){ 
			if($(this).data('data')['dev_id'] == id){
				$('ul.filetree:eq(1)').treeview({remove:$(this).parent('li')});
			}
		})
		$('ul.filetree:eq(0)').treeview({remove:$('#dev_'+id).parent('li')});
		closeMenu();
	}*/

	/*function RemoveDeviceAllSuccess(data){
		$('ul.filetree:eq(0) span.device').each(function(){
			$('ul.filetree:eq(0)').treeview({remove:$(this).parent('li')});
		})		
	}*/

	function AddChannelDoubleInGroupSuccess(data){
		data.channelname=data.channelname.replace(/-/g,'.');
		var group = $('div.dev_list:eq(1) span.sel:eq(0)').hasClass('group') ? $('div.dev_list:eq(1) span.sel:eq(0)') : $('div.dev_list:eq(1) span.group:eq(0)');
		if($('#g_channel_'+data.chlid)[0]){
			return;
		}
			var add = $('div.dev_list:eq(0) #channel_'+data.chlid).parent('li').clone(true)
			    .find('span.channel')
			    .attr('id','g_channel_'+data.chlid)
			    .html(data.channelname)
			    .end()
			    .appendTo(group.next('ul'));
			add.find('span.channel').data('data')['r_chl_group_id'] = data.chlgroupid;
			add.find('span.channel').data('data')['channel_name'] =data.channelname;
		$('ul.filetree').treeview({add:add}).find('span.channel').removeClass('sel');
	}
	function RemoveChannelFromGroupSuccess(data){ 
		$('div.dev_list:eq(1) span.channel.sel').each(function(){
			$(this).parent('li').remove();//appendTo($('#dev_'+$(this).data('data').dev_id).next('ul'));
		})
		$('ul.filetree').treeview();
	}
	function SettingCommonParmSuccess(data){
		//
	}
	function SettingRecordDoubleTimeParmSuccess(data){
		/*var week = $('#week').attr('data').split(',');
		for(i in week){
			var weekData = $('ul.week.option li:eq('+week[i]+')').data()
			var n = 0;
			for(j in weekData){
				var newTimeWarp = $('#recordtime tbody tr:eq('+n+')')
				weekData[j].starttime='1970-01-01 '+newTimeWarp.find('div.timeInput:eq(0)').gettime();
				weekData[j].endtime='1970-01-01 '+newTimeWarp.find('div.timeInput:eq(1)').gettime();
				weekData[j].starttime='1970-01-01 '+newTimeWarp.find('.iput2.time_picker:eq(0)').val();
				weekData[j].endtime='1970-01-01 '+newTimeWarp.find('.iput2.time_picker:eq(1)').val();
				weekData[j].enable=newTimeWarp.find(':checkbox').is(':checked') ? 1 : 0;
				n++;
			}
			$('#recordtime tbody tr:lt(4)').each(function(){
				var nowWeekData = $('ul.week.option li:eq('+i+')').data('data_'+$(this).find('input.timeid').val());
				nowWeekData.starttime='1970-01-01 '+$(this).find('div.timeInput:eq(0)').gettime();
				nowWeekData.endtime='1970-01-01 '+$(this).find('div.timeInput:eq(1)').gettime();
				nowWeekData.enable=$(this).find(':checkbox').is(':checked') ? 1 : 0;
			})
		}*/
		var week = $('#week').attr('data').split(',');
		FillChannleRecordTime($('div.dev_list:eq(2) span.channel.sel'));
		week.length == 1 && $('#RecordTime ul.week.option li').eq(week[0]).click();
		
	}
	function SettingRecordDoubleTimeParm(){ //清空回放时间表单的数据
		//$('#recordtime div.timeInput input').val('');
		$('#recordtime .schedle_id .time_picker').val('');
		$('#recordtime input:checkbox').prop('checked',false);
		$('ul.week.option li').removeData();
		$('#week').val(lang.Monday).attr('data','0');
		$('#recordtime td.copyTo').find('li').remove()
								  .end().find('input').val('').data('');
		//areaList2Ui(2);
	}
	function SettingStorageParmSuccess(data){
		//alert(data);
	}
	function searchEdDev(){   //填充已搜索设备初始值
		searchedDev=[];
		$('ul.filetree span.device').each(function(){
			var devData = $(this).data('data');
			var usedKey = $(this).data('data').eseeid;
				usedKey = usedKey ? usedKey : devData.address;			
			searchedDev.push(usedKey);
		})
	}
	function searchDevAvailable(key){
		if(searchedDev.length == 0){
			return true;
		}
		for( i in searchedDev ){
			if(searchedDev[i] == key){
				return false;
			}
		}
		return true;
	}
	function initOxcDevListStatus(){
		//分组列表;
		//groupList2Ui();

		areaList2Ui('','closed');

		// UI 调整
		set_contentMax();

		searchEdDev();

		if(key == 0){
			searchFlush();
		}else{
			oSearchOcx.Stop();
		}
	}
	
    //设备管理中的IP/ID模糊搜索
	function searchbtn(){
 
            var searchText=$("#searchtxt").val(),//获取搜索框的关键词
                count = 0;
			   //去除已存在行样式，避免重复
			$('#SerachedDevList tr').removeClass('sel');
			   
			$('#SerachedDevList tr input:checkbox').prop("checked",false);
			   
            if(searchText!=""){//用indexof()模糊匹配，从而找到相应的数据
                      
                $('#SerachedDevList tr').each(function(){
						
				    if($(this).children('td:eq(1)').text().indexOf(searchText)!=-1 || $(this).children('td:eq(2)').text().indexOf(searchText)!=-1){ 
						   //将匹配的结果置顶
					    $(this).find('td input:checkbox').prop("checked",true).parent('td').parent('tr').prependTo('#SerachedDevList');

						count++;
					}
			    });
					
				count == 0 &&	Confirm(_T('Not_Found'));

            }else{
				   
				Confirm(_T('Not_Null_Enter_Keywords')); 
		    } 
        } 
 //搜索取消按钮
	function quitbtn(){
		
		$('#SerachedDevList tr input:checkbox').prop("checked",false);
		
		$("#searchtxt").val('');
	}
//检验输入的时分秒格式是否正确
	function checkTime_Jtp_fmt(goal){
		   
          if(!chk_value_time(goal.val())){//判断输入的时间格式是否正确
	
			Confirm(_T('Time_format_error'));
			goal.val(goal.attr("data"));//若格式错误，恢复原值
					 
					}
	}
   function chk_value_time(str)
   {
	
	var a = str.match(/^(\d{1,2})(:)?(\d{1,2})\2(\d{1,2})$/);
	if (a == null) return false;
	if (a[1]>23 || a[3]>59 || a[4]>59) return false;
	return true;
    }	
//ip和通道的排序

function SortSearchFile_2UI(sortfn){
	
	/*for(var i in devFile){
	  console.log(devFile[i].SearchChannelCount_ID);
	}*/
	var fn = window[sortfn];
	var nowDevFile=[];
	nowDevFile = devFile.sort(fn);
	/*console.log('---after sort ---- ');
	for(var i in devFile){
	  console.log(devFile[i].SearchChannelCount_ID);
	}
	*/

    for(var i=0;i<nowDevFile.length;i++){
		
	  
	   var data = nowDevFile[i];
	   
	   var key = data.SearchSeeId_ID ?  data.SearchSeeId_ID : data.SearchIP_ID;
		

		if(searchDevAvailable(key)){
			var id = data.SearchSeeId_ID > 1 ? data.SearchSeeId_ID : data.SearchIP_ID.replace('.','-');
			$('<tr id="esee_'+id+'" class="'+data.SearchVendor_ID+'"><td><input type="checkbox" />'+data.SearchVendor_ID+'</td><td>'+data.SearchSeeId_ID+'</td><td>'+data.SearchIP_ID+'</td><td>'+data.SearchChannelCount_ID+'</td></tr>').appendTo($('#SerachDevList tbody')).data('data',data);
		}
	}	
   
       var warp = $('#SerachDevList');

		if($('#SerachedDevList').height() > warp.height() && !warp.attr('b')){
			theadtbody(warp.find('thead td'),warp.prev('table').find('td'));
			warp.attr('b',0);
		}
	
}
function VendorAsc(a,b){
	
	if(a.SearchVendor_ID>b.SearchVendor_ID) return 1;
	if(a.SearchVendor_ID<b.SearchVendor_ID) return -1;
	return 0;
	
}
function VendorDes(a,b){
   if(b.SearchVendor_ID>a.SearchVendor_ID) return 1;
	if(b.SearchVendor_ID<a.SearchVendor_ID) return -1;
	return 0;
}
function ChlAsc(a,b){
	return a.SearchChannelCount_ID - b.SearchChannelCount_ID;
}

function ChlDes(a,b){
	return b.SearchChannelCount_ID - a.SearchChannelCount_ID;
}
function transformIp(ip){
	var ipValue = ip.split('.');
	return parseInt(ipValue[0],10)*256*256*256+parseInt(ipValue[1],10)*256*256+parseInt(ipValue[2],10)*256+parseInt(ipValue[3],10);
	}
function IPAsc(a,b){
	
	return transformIp(a.SearchIP_ID) - transformIp(b.SearchIP_ID);
	
}
function IPDes(a,b){
	
	return transformIp(b.SearchIP_ID) - transformIp(a.SearchIP_ID);
	
}

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
		   closeMenu();
		   confirm_tip(_T('no_limit'));
			var timer =setTimeout(function(){
				closeMenu();
				clearTimeout(timer);
			},2000);
			
		}
 }
   function checkUserRightdiv(uicode,uisubcode,fn,num){
	  //console.log('uicode:'+uicode+' uisubcode:'+uisubcode);
	  var itema= autoSearchDev.checkUserLimit(uicode.toString(2),uisubcode);
	//console.log("当前用户"+autoSearchDev.getCurrentUser()+" 登录状态："+itema);
		if(itema==0){
			window[fn](num);
		}else if(itema==1){
			
             var show = autoSearchDev.showUserLoginUi(336,300);
			  if(show==0){
				  var timer = setTimeout(function(){
					   checkUserRightBtn(uicode,uisubcode,fn,num);
					   clearTimeout(timer);
					  },300);
			  }else{
				  var timer1 = setTimeout(function(){
				    if(fn=='userList2Ui'){
					        $('.right_content:visible ul.ope_list li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct');
							  $('.right_content:visible div.switch').hide();
							  $('.right_content:visible div.switch').eq(0).show();
					  }  
					   clearTimeout(timer1);
					  },300);
			  }
		}else{
		   closeMenu();
		   confirm_tip(_T('no_limit'));
			var timer =setTimeout(function(){
				closeMenu();
				if(fn=='userList2Ui'){
				$('.right_content:visible ul.ope_list li').eq(0).addClass('ope_listAct').siblings('li').removeClass('ope_listAct');
						$('.right_content:visible div.switch').hide();
						$('.right_content:visible div.switch').eq(0).show();
			     cUserinfo2Ui();
				}
				clearTimeout(timer);
			},300);
			
		}
 }
 function checklogoutinterval(obj){
	 var value = parseInt(obj.val(),10);
	 if(/^\d+$/.test(value)){
	   if(value>0 && value<20){
		  obj.val('20'); 
	   }
	 }else{
		Confirm(_T('error_logoutinterval')); 
		obj.val('');
	 }
	 
 }
 function checkuserpwd(obj){
    var value = obj.val();
	if(/[\u4e00-\u9fa5]/.test(value)){
	   	Confirm(_T('error_userpwd')); 
		obj.val('');
	}
	 
}

function userSetInfoImport(){
 	
  onvifuserInfo.importUserInfo();	
}

function userSetInfoExport(){
 	
   onvifuserInfo.outportUserInfo();	
}
