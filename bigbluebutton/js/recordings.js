        $.fn.dataTableExt.oApi.fnReloadAjax = function ( oSettings, sNewSource, fnCallback, bStandingRedraw ){
            if ( typeof sNewSource != 'undefined' && sNewSource != null ){
                oSettings.sAjaxSource = sNewSource;
            }
            this.oApi._fnProcessingDisplay( oSettings, true );
            var that = this;
            var iStart = oSettings._iDisplayStart;
	
            oSettings.fnServerData( oSettings.sAjaxSource, null, function(json) {
	    /* Clear the old information from the table */
	    that.oApi._fnClearTable( oSettings );
		
	    /* Got the data - add it to the table */
	    for ( var i=0 ; i<json.aaData.length ; i++ ){
                //that.oApi._fnAddData( oSettings, json.aaData[i] );
                that.oApi._fnAddData(oSettings, json[oSettings.sAjaxDataProp][i]);
            }
		
            oSettings.aiDisplay = oSettings.aiDisplayMaster.slice();
            that.fnDraw( that );
		
            if ( typeof bStandingRedraw != 'undefined' && bStandingRedraw === true ){
                oSettings._iDisplayStart = iStart;
			    that.fnDraw( false );
		    }
		
		    that.oApi._fnProcessingDisplay( oSettings, false );
		
		    /* Callback user function - for event handlers etc */
		    if ( typeof fnCallback == 'function' && fnCallback != null ){
			    fnCallback( oSettings );
		    }
                }, oSettings );
        }


        var oTable;
        
        $(document).ready(function(){
            if ( Drupal.settings.bigbluebutton_view == 'recordings' ){
                oTable = $('#example').dataTable( {
                    "aoColumns": [
                        { "sTitle": "Type", "sWidth": "50px" },
                        { "sTitle": "Meeting", "sWidth": "150px" },
                        { "sTitle": "Description", "sWidth": "150px" },
                        { "sTitle": "Date Recording", "sWidth": "100px"},
                        { "sTitle": "Actionbar", "sWidth": "50px", "sClass": "right" },
                        ],
		    
                    "oTableTools": {
                        "sRowSelect": "multi",
                        "aButtons": [ "select_all", "select_none" ]
                        },
                    
                    "sAjaxSource": Drupal.settings.bigbluebutton_jsonURL,
                    "bFilter": false,
                    "bPaginate": false,
                    "bInfo": false,
                    "fnInitComplete": function () {
                        oTable.fnReloadAjax();
                    }
                });
                    
                oTable.fnSetColumnVis( 4, Drupal.settings.bigbluebutton_ismoderator );			
                 
                setInterval(function() {
                    oTable.fnReloadAjax();
                }, 1000);

            }
        });
