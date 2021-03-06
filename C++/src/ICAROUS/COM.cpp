/**
 * Communication
 * 
 * This class contains functions to constantly collect data from the pixhawk and ground station.
 *
 * Contact: Swee Balachandran (swee.balachandran@nianet.org)
 * 
 * 
 * Copyright (c) 2011-2016 United States Government as represented by
 * the National Aeronautics and Space Administration.  No copyright
 * is claimed in the United States under Title 17, U.S.Code. All Other
 * Rights Reserved.
 *
 * Notices:
 *  Copyright 2016 United States Government as represented by the Administrator of the National Aeronautics and Space Administration. 
 *  All rights reserved.
 *     
 * Disclaimers:
 *  No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER EXPRESSED, 
 *  IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY
 *  IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, 
 *  ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, 
 *  WILL CONFORM TO THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN ENDORSEMENT BY GOVERNMENT 
 *  AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS, RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS 
 *  RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY DISCLAIMS ALL WARRANTIES AND 
 *  LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  
 *   RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE UNITED STATES GOVERNMENT, 
 *   ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE 
 *   RESULTS IN ANY LIABILITIES, DEMANDS, DAMAGES,
 *   EXPENSES OR LOSSES ARISING FROM SUCH USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, 
 *   RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED STATES GOVERNMENT, 
 *   ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  
 *   RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
 */

#include "COM.h"

COM_t::COM_t(Interface_t* px4int, Interface_t* gsint,AircraftData_t *fdata):log("COM"){
     px4Intf      = px4int;
     gsIntf       = gsint;
     FlightData   = fdata;
     RcvdMessages = fdata->RcvdMessages;
     WPcount      = 0;                     // total waypoint count
     WPloaded     = 0;                     // waypoints loaded
 }

// Get data from ground station
 void COM_t::GetGSData(){

     
     while(true){
         gsIntf->GetMAVLinkMsg();
         
         
         // Handle mission count
         MissionCountHandler();

         // Mission item handler
         MissionItemHandler();

         // Handle mission list request
         MissionRequestListHandler();

         //  Handle mission request
         MissionRequestHandler();

         // Handle parameter request list
         ParamRequestListHandler();

         // Handle parameter request read
         ParamRequestReadHandler();

         // Handle parameter set
         ParamSetHandler();

         // Handle set mode
         SetModeHandler();

         // Handle long commands
         CommandLongHandler();

         // Handle kinematic bands
         KinematicBandsHandler();

     }
     

 }

 
 void COM_t::MissionCountHandler(){
     mavlink_mission_count_t msg;
     bool have_msg = RcvdMessages->GetMissionCount(msg);
     if(have_msg && msg.target_system == 1){
    	 log.addWarning("Receiving waypoints");
         mavlink_message_t msg2send;
         mavlink_msg_mission_count_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
         WPcount = msg.count;
         FlightData->ClearMissionList();
         WPloaded = 0;
     }
 }

 void COM_t::MissionItemHandler(){
     mavlink_mission_item_t msg;
     bool have_msg = RcvdMessages->GetMissionItem(msg);
     if(have_msg && msg.target_system == 1){
         mavlink_message_t msg2send;
         mavlink_msg_mission_item_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
         if(msg.seq == WPloaded){
             //printf("Added wp %d to list\n",WPloaded);
             FlightData->AddMissionItem(msg);
             WPloaded++;
         }
     }
 }

 void COM_t::MissionRequestHandler(){
     mavlink_mission_request_t msg;
     bool have_msg = RcvdMessages->GetMissionRequest(msg);
     if(have_msg && msg.target_system == 1){
         mavlink_message_t msg2send;
         mavlink_msg_mission_request_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
     }

 }

 void COM_t::MissionRequestListHandler(){
     mavlink_mission_request_list_t msg;
     bool have_msg = RcvdMessages->GetMissionRequestList(msg);
     if(have_msg && msg.target_system == 1){
         mavlink_message_t msg2send;
         mavlink_msg_mission_request_list_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
     }
 }

 void COM_t::ParamRequestListHandler(){
     mavlink_param_request_list_t msg;
     bool have_msg = RcvdMessages->GetParamRequestList(msg);
     if(have_msg){
         mavlink_message_t msg2send;
         mavlink_msg_param_request_list_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
     }
 }

 void COM_t::ParamRequestReadHandler(){
     mavlink_param_request_read_t msg;
     bool have_msg = RcvdMessages->GetParamRequestRead(msg);
     if(have_msg && msg.target_system == 1){
         mavlink_message_t msg2send;
         mavlink_msg_param_request_read_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
     }
 }

 void COM_t::ParamSetHandler(){
	 mavlink_param_set_t msg;
	 bool have_msg = RcvdMessages->GetParamSet(msg);
	 if(have_msg && msg.target_system == 1){

		 bool icarous_parm;

		 if(!strcmp(msg.param_id,"ICHBEAT"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"HTHRESHOLD"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"VTHRESHOLD"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"HSTEPBACK"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"VSTEPBACK"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"STEPBACKTYPE"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"GRIDSIZE"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"BUFFER"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"LOOKAHEAD"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"PROXFACTOR"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"RES_SPEED"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"XTRK_GAIN"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"XTRK_DEV"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"TAKEOFF_ALT"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"CHEAP_DAA"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"CHEAP_SEARCH"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"MAX_CEILING"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"TRK_HEADING"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"TRK_DISTH"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"TRK_DISTV"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"CYL_RADIUS"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"CYL_HEIGHT"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"ALERT_TIME"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"EALERT_TIME"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"DAA_LOOKAHEAD"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"SYS_ID"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"COMP_ID"))
			 icarous_parm = true;
		 else if(!strcmp(msg.param_id,"AUTOPILOT_ID"))
			 icarous_parm = true;
		 else
			 icarous_parm = false;


		 if(icarous_parm){
			 FlightData->paramData->set(msg.param_id, msg.param_value, FlightData->paramData->getUnit(msg.param_id));
			 //printf("received %s\n",msg.param_id);
			 mavlink_message_t msg2send;
			 mavlink_param_value_t paramValue;
			 strcpy(paramValue.param_id,msg.param_id);
			 paramValue.param_value = msg.param_value;
			 mavlink_msg_param_value_encode(1,0,&msg2send,&paramValue);
			 gsIntf->SendMAVLinkMsg(msg2send);
		 }
		 else{

			 mavlink_message_t msg2send;
			 mavlink_msg_param_set_encode(255,0,&msg2send,&msg);
			 px4Intf->SendMAVLinkMsg(msg2send);
		 }

	 }
 }


  void COM_t::ParamValueHandler(){
     mavlink_param_value_t msg;
     bool have_msg = RcvdMessages->GetParamValue(msg);
     if(have_msg){
         mavlink_message_t msg2send;
         mavlink_msg_param_value_encode(255,0,&msg2send,&msg);
         
     }
 }

  void COM_t::SetModeHandler(){
     mavlink_set_mode_t msg;
     bool have_msg = RcvdMessages->GetSetMode(msg);
     if(have_msg && msg.target_system == 1){
         mavlink_message_t msg2send;
         mavlink_msg_set_mode_encode(255,0,&msg2send,&msg);
         px4Intf->SendMAVLinkMsg(msg2send);
     }
 }

 void COM_t::CommandLongHandler(){
     mavlink_command_long_t msg;
     bool have_msg = RcvdMessages->GetCommandLong(msg);
     if(have_msg && msg.command == MAV_CMD_MISSION_START){
    	 log.addWarning("Received mission start command");
         FlightData->SetStartMissionFlag((uint8_t)msg.param1);
     }
     else if(have_msg && msg.command == MAV_CMD_DO_FENCE_ENABLE){
    	 log.addWarning("Receiving geofence");
    	 FlightData->GetGeofence(gsIntf,msg);
     }
     else if(have_msg && msg.command == MAV_CMD_SPATIAL_USER_1){
    	 FlightData->AddTraffic((int)msg.param1,msg.param5,msg.param6,msg.param7,
    			 	 	 	 	msg.param2,msg.param3,msg.param4);
     }
     else if(have_msg && msg.command == MAV_CMD_USER_1){
    	 if(msg.param1 == 0){
    		 log.addWarning("Received reset command");
    		 FlightData->Reset();
    	 }
     }
     else if(have_msg){
    	 mavlink_message_t msg2send;
		 mavlink_msg_command_long_encode(255,0,&msg2send,&msg);
    	 px4Intf->SendMAVLinkMsg(msg2send);
     }
 }

 void COM_t::CommandIntHandler(){
     mavlink_command_int_t msg;
     bool have_msg = RcvdMessages->GetCommandInt(msg);
     if(have_msg && msg.command == MAV_CMD_SPATIAL_USER_2){
         
     }
 }

 void COM_t::KinematicBandsHandler(){
	 mavlink_kinematic_bands_t msg;
	 bool have_msg = RcvdMessages->GetKinematicBands(msg);
	 if(have_msg){
	     mavlink_message_t msg2send;
	     mavlink_msg_kinematic_bands_encode(1,0,&msg2send,&msg);
	     gsIntf->SendMAVLinkMsg(msg2send);
	 }
 }
