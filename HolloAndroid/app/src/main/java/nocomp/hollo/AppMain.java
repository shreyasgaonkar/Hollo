package nocomp.hollo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.text.InputType;
import android.util.Log;
import android.widget.EditText;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;
import java.util.ArrayList;

public class AppMain
{
    private static ServerMessaging SM = null;
    private static Activity state = null;
    private static User user = null;
    private static Contact contact =  null;
    public static EditText chat = null;

    public static void start(String ip, int port)
    {
        logout();
        SM = new ServerMessaging(ip, port, state);
        SM.execute();
    }

    public static void decodeMessage(String in)
    {
        Log.d("decodeMessage", "before if");
        if(in.length() >= 20)
        {
            Log.d("decodeMessage", in);
            try {
                String text = "";
                String temp = in.substring(0, 3).trim();
                int length;
                if(temp.length() > 0)
                length = Integer.parseInt(temp);
                else length = 0;
                int type = ByteBuffer.wrap(new StringBuffer(in.substring(4, 8)).reverse().toString().getBytes()).getInt();
                int code = ByteBuffer.wrap(new StringBuffer(in.substring(8, 12)).reverse().toString().getBytes()).getInt();
                int from = ByteBuffer.wrap(new StringBuffer(in.substring(12, 16)).reverse().toString().getBytes()).getInt();
                int to = ByteBuffer.wrap(new StringBuffer(in.substring(16, 20)).reverse().toString().getBytes()).getInt();
                if (in.length() > 20) text = in.substring(20);
                Log.d("decodeMessage", "past formatting");
                Log.d("type", String.valueOf(type));
                Log.d("code", String.valueOf(code));
                Log.d("from", String.valueOf(from));
                Log.d("to", String.valueOf(to));
                Log.d("message", text);
                switch (type) {
                    case Type.TEXT_M:
                        if (to == user.getUserID())
                            receiveMessage(text);
                        Log.d("Decode", "case 0");
                        break;
                    case Type.LOG_M:
                        if (Code.L_SUCCESS == code)
                            login(to);
                        break;
                    case Type.CREAT_ACC:
                        if (Code.C_ACC_SUCCESS == code) {
                            warning("Account created");
                        }
                        break;
                    case Type.STATUS:
                        changeStatus(code, from, to, text);
                        break;
                    case Type.FRIEND:
                        contact(code, from, to, text);
                        break;
                    default:
                        Log.d("Decode", "Error");
                        warning(text);
                        break;
                }
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
        }
    }

    public static void encode(String s, int type, int code, int from, int to)
    {
        String x;
        switch(String.valueOf(s.length()).length())
        {
            case 1: x = "   " + String.valueOf(s.length()); break;
            case 2: x = "  " + String.valueOf(s.length()); break;
            case 3: x = " " + String.valueOf(s.length()); break;
            case 4: x = String.valueOf(s.length()); break;
            default: x = "   0"; break;
        }
        x += new String(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(type).array(), Charset.forName("UTF-8"));
        x += new String(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(code).array(), Charset.forName("UTF-8"));
        x += new String(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(from).array(), Charset.forName("UTF-8"));
        x += new String(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(to).array(), Charset.forName("UTF-8"));
        x += s;
        Log.d("Encode - x", x);
        SM.setMessage(x);
        Log.d("Encode", "Past setMessage");
    }

    public static void login(String name, String password)
    {
        //
        try
        {
            if(name.length() <= 16 && password.length() <= 16) {
                user = new User(-1, name, password);
                while(name.length() < 16)
                    name += String.valueOf('\0');
                while(password.length() < 16)
                    password += String.valueOf('\0');
                encode(name + password, Type.LOG_M, Code.L_IN, 0, 0);
            }
            else warning("Name and Password must be MAX 16 characters!");
        }
        catch(Exception e)
        {
            //
        }
    }

    private static void login(int to)
    {
        Log.d("login - to", String.valueOf(to));
        user.setUserID(to);
        user.setStatus(1);
        contact(Code.F_GET_PENDING, user.getUserID(), 0, "");
        state.startActivity(new Intent(state, Contacts.class));
    }

    public static void create(String name, String password)
    {
        //
        try
        {
            if(name.length() <= 16 && password.length() <= 16) {
                while (name.length() < 16)
                    name += String.valueOf('\0');
                while (password.length() < 16)
                    password += String.valueOf('\0');
                encode(name+password, Type.CREAT_ACC, 0, 0, 0);
            }
            else warning("Name and Password must be MAX 16 characters!");
        }
        catch(Exception e)
        {
            //
        }
    }

    public static void warning(String description)
    {
        AlertDialog alertDialog = new AlertDialog.Builder(state).create();
        alertDialog.setTitle("Alert");
        alertDialog.setMessage(description);
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });
        alertDialog.show();
    }

    public static void setChat()
    {
        if(contact != null)
        {
            if(contact.getType() == false)
            state.startActivity(new Intent(state, MainActivity.class));
            else warning("Friendship is pending");
        }
    }

    public static void sendMessage(String s)
    {
        try
        {
            encode(s, Type.TEXT_M, Code.TEXT_C, user.getUserID(), contact.getUserID());
            chat.append(user.getUserName() + ": " + s + "\n");
        }
        catch(Exception e)
        {

        }
    }

    public static void receiveMessage(String s)
    {
        Log.d("receiveMessage", state.getLocalClassName());
        if(state.getLocalClassName().equals("MainActivity"))
        {
            if(chat != null)
                chat.append(contact.getUserName() + ": " + s + "\n");
        }
    }

    public static void contact(int code, int from, int to, String name)
    {
        Log.d("userID", String.valueOf(user.getUserID()));
        if(contact != null)
        Log.d("contactID", String.valueOf(contact.getUserID()));
        switch(code)
        {
            case Code.F_REQUEST:
                if(from == user.getUserID())
                    encode("", Type.FRIEND, Code.F_REQUEST, from, to);
                else if(to == user.getUserID()) {
                    user.addContact(new Contact(from, name, 3,true));
                }
                break;
            case Code.F_ACCEPT:
                if(from == user.getUserID()) {
                    encode("", Type.FRIEND, Code.F_ACCEPT, from, to);
                    contact.setType(false);
                    user.changeContactType(contact.getUserID(), false);
                }
                else if(to == user.getUserID())
                    user.changeContactType(from, false);
                break;
            case Code.F_DENY:
                 if(from == user.getUserID())
                 {
                     encode("", Type.FRIEND, Code.F_DENY, from, to);
                     user.removeContact(to);
                 }
                else if(to == user.getUserID())
                     user.removeContact(from);
                break;
            case Code.F_REMOVE:
                if(from == user.getUserID())
                {
                    encode("", Type.FRIEND, Code.F_REMOVE, from, to);
                    user.removeContact(to);
                }
                else if(to == user.getUserID())
                    user.removeContact(from);
                break;
            case Code.F_GET_PENDING:
                encode("", Type.FRIEND, Code.F_GET_PENDING, user.getUserID(), 0);
                break;
            default: break;
        }
        Log.d("contact", state.getLocalClassName());
        if(state.getLocalClassName().equals("Contacts"))
            Contacts.update(state);
    }

    public static void acceptFriend()
    {
        if(contact != null) {
            Log.d("acceptFriend", contact.getUserName());
            int id = contact.getUserID();
            user.changeContactType(id, false);
            contact(Code.F_ACCEPT, user.getUserID(), id, "");
        }
    }

    public static void removeFriend()
    {
        if(contact != null) {
            Log.d("removeFriend", contact.getUserName());
            int id = contact.getUserID();
            user.removeContact(id);
            contact = null;
            contact(Code.F_REMOVE, user.getUserID(), id, "");
        }
    }

    public static void searchFriend(String name)
    {
        Log.d("searchFriend", name);
        user.addContact(new Contact(0, name, 3, true));
        contact(Code.F_REQUEST, user.getUserID(), 0, name);
    }

    public static void changeStatus(int code)
    {
        Log.d("change status", String.valueOf(code));
        switch(code)
        {
            case 1: user.setStatus(code); changeStatus(Code.S_CHANGE_A, user.getUserID(), 0, ""); break;
            case 2: user.setStatus(code); changeStatus(Code.S_CHANGE_B, user.getUserID(), 0, ""); break;
            case 3: user.setStatus(code); changeStatus(Code.S_CHANGE_O, user.getUserID(), 0, ""); break;
            case 4: user.setStatus(code); changeStatus(Code.S_CHANGE_AW, user.getUserID(), 0, ""); break;
            default: break;
        }
    }

    public static void changeStatus(int code, int from, int to, String text)
    {
        switch(code)
        {
            case Code.S_CHANGE_A:
                if(from == user.getUserID())
                    encode("", Type.STATUS, Code.S_CHANGE_A, from, to);
                else if(to == user.getUserID())
                    user.setContactStatus(from, 1, text);
                break;
            case Code.S_CHANGE_AW:
                if(from == user.getUserID())
                    encode("", Type.STATUS, Code.S_CHANGE_AW, from, to);
                else if(to == user.getUserID())
                    user.setContactStatus(from, 2, text);
                break;
            case Code.S_CHANGE_B:
                if(from == user.getUserID())
                    encode("", Type.STATUS, Code.S_CHANGE_B, from, to);
                else if(to == user.getUserID())
                    user.setContactStatus(from, 3, text);
                break;
            case Code.S_CHANGE_O:
                if(from == user.getUserID())
                    encode("", Type.STATUS, Code.S_CHANGE_O, from, to);
                else if(to == user.getUserID())
                    user.setContactStatus(from, 4, text);
                break;
            default: break;
        }
        Log.d("changeStatus", state.getLocalClassName());
        if(state.getLocalClassName().equals("Contacts"))
            Contacts.update(state);
    }

    public static String getStatus()
    {
        switch(user.getStatus())
        {
            case 1: return "Online";
            case 2: return "Busy";
            case 3: return "Offline";
            case 4: return "Away";
            default: return "Error";
        }
    }

    public static void selectContact(int id)
    {
        Log.d("selectContact", String.valueOf(id));
        try
        {
            contact = user.getContact(id);
        }
        catch(Exception e)
        {
            //
        }
    }

    public static void retrieveContacts()
    {
        contact(Code.F_GET_PENDING, user.getUserID(), 0, "");
    }

    public static void finish()
    {
        Log.d("finish", state.getLocalClassName());
        if(!state.getLocalClassName().equals("Login"))
            state.finish();
    }

    public static void changeState(Activity act)
    {
        state = act;
        if(SM == null)
            finish();
    }

    public static Activity getState() {return state;}

    public static void logout()
    {
        if(SM != null)
        {
            if(SM.listen != false && SM.retcode == 0 && user != null)
            encode("", Type.LOG_M, Code.L_OUT, user.getUserID(), 0);
            SM.listen = false;
            SM = null;
        }
        user = null;
        contact = null;
        finish();
    }

    public static String getUserName()
    {
        if(user != null)
            return user.getUserName();
        else return "No name";
    }

    public static String getContactName()
    {
        if(user != null)
            return contact.getUserName();
        else return "No name";
    }

    public static ArrayList<Contact> getUserContacts()
    {
        return user.getUserContacts();
    }



    public abstract class Type
    {
        public static final int TEXT_M = 1;
        public static final int LOG_M = 2;
        public static final int CREAT_ACC = 3; //Creating account. Send this you will get back this on success
        public static final int STATUS = 4; //send this to change status. If you receive this a friend changed status
        public static final int FRIEND = 5; //Friend requests and such
        public static final int CALL = 6; //TODO
        public static final int VID = 7; //TODO
        public static final int ERR = 8;   //Error message will be in body
        public static final int M_TYPE_MAX = 9;
    }

    public abstract class Code
    {
        public static final int TEXT_C = 1;
        public static final int L_IN = 2; //send to server to log in
        public static final int L_OUT = 3; //send to server to log out
        public static final int L_SUCCESS = 4; //receives from server on successful log in
        public static final int C_ACC_SUCCESS = 5; //Sent tou you on creating account success
        public static final int S_CHANGE_MIN = 6; //DO NOT USE THIS ONE!!!!
        public static final int S_CHANGE_A = 7; //Available status
        public static final int S_CHANGE_B = 8; //busy
        public static final int S_CHANGE_O = 9; //offline
        public static final int S_CHANGE_AW = 10;  //Away
        public static final int F_REQUEST = 11; //If you receive someone is requesting to be your friend. Else you can use to request friend
        public static final int  F_REMOVE = 12; //remove a friend from friends list. Send to server or receive (someone could have renoved you)
        public static final int F_ACCEPT = 13; //You send to server when you accept a friend request
        public static final int F_DENY = 14; //send to server when you want to deny a friend request
        public static final int  F_GET_PENDING = 15; //Get list of pending friend requests
	    /*FUTURE CODES PREDEFINED NOW*/
        //these are TODO
        public static final int INIT_CALL = 16;
        public static final int INIT_VID = 17;
        public static final int  END_CALL = 18;
        public static final int END_VID = 19;
        //MAX
        public static final int  M_CODE_MAX = 20;
    }
}
