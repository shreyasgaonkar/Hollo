#ifndef DEFINITIONS_H
#define DEFINITIONS_H

enum m_type {
	TEXT_M = 1,
	LOG_M,
	CREAT_ACC, //Creating account. Send this you will get back this on success
	STATUS, //send this to change status. If you receive this a friend changed status
	FRIEND, //5 .Friend requests and such
	CALL, //TODO
	VID, //TODO
	ERR,   //8  .Error message will be in body
	M_TYPE_MAX};

enum m_code {
	TEXT_C = 1,
	L_IN, //send to server to log in
	L_OUT, //send to server to log out
	L_SUCCESS, //receives from server on successfull log in
	C_ACC_SUCCESS, //5 .Sent tou you opn creating account success
	S_CHANGE_MIN, //DO NOT USE THIS ONE!!!!
	S_CHANGE_A, //Available status
	S_CHANGE_B, //busy
	S_CHANGE_O, //offline
	S_CHANGE_AW,  //10 .Away
	F_REQUEST, //If you receive someone is requesting to be your friend. Else you can use to request friend
	F_REMOVE, //remove a freind from friends list. Send to server or receive (someone could have renoved you)
	F_ACCEPT, //You send to server when you accept a freidn request
	F_DENY, //send to server when you want to deny a friend request
	F_GET_PENDING, //15  .Get list of pending friend requests
	/*FUTURE CODES PREDEFINED NOW*/
	//these are TODO
	INIT_CALL,
	INIT_VID,
	END_CALL,
	END_VID, //19
	//GET statuses of your friends
	S_GET_STATUSES, //20

	//MAX
	M_CODE_MAX};

#endif
