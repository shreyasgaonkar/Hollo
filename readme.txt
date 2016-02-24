Hollo is a skype like application.
The Hollo application will eventually become a cross platform app where users can communicate with each other via several means.
Users will be able to send text based messages to each other and the text will be stored for future or until manually deleted. 
Users will also be able to make calls to one another in real time. Additionally, users will be able to video chat with each other.


1. SERVER
1.1 Data:
	The data is formated and sent to client(s) and server the following way:
	XXXXAAAABBBBCCCCDDDDtexttexttext....
	
	XXXX through DDDD is a header section.
	text is the data section. In conventional message it contains the actual
		message.

	XXXX - human readable number of bytes in text portion.
	AAAA - binary encoded number corresponding to "type"
	BBBB - binary encoded number corresponding to "code_number"
	CCCC - binary encoded number corresponding to "from"
	DDDD - binary encoded number corresponding to "to"
	text - any data, but usually human readable text

1.2 Data example:

	"|   5|0x0001|0x0001|0x0000|0x0000|hello|"
	  XXXX   AAAA   BBBB   CCCC   DDDD text
	first portion has spaces in front of the number.
	AAAA is not human readable but it should contain four bytes.
	BBBB, CCCC, DDDD are all similar to AAAA. It's just some data encoded

	Hexdump of the above would like this:
	|0x20 0x20 0x20 0x05| 0x00000001 | 0x00000001 | 0x00000000 | 0x00000000 |
0x68656c6c6f0000000000....00 |

1.3 Compiling server and testing:

	to compile the server you need boost framework. Checkout their website
		for the platform. On Arch linux do `#pacman -S boost` and it will
		install all necessary packages.
	to run the server enter `$./my_server [port]`
	to test the server you can use simple client with the same port and
		localhost as the ip. run client `$./my_client localhost [port]` where
		port is the same port as in the server.
	Both, the server and the client print right now some debug messages for
		testing purposes.
