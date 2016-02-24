package nocomp.hollo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.EditText;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.lang.String;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.nio.charset.Charset;
import java.util.ArrayDeque;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class ServerMessaging extends AsyncTask<Void, Void, Integer>
{
    private Lock lockit;
    private Context appcontext;
    private String ip;
    private int port;
    private Socket socket = null;
    public boolean listen = true;
    private DataInputStream in = null;
    private BufferedWriter out = null;
    private ArrayDeque<String> q;
    private ArrayDeque<String> qw;
    private String message = null;
    private String listened = null;
    public int retcode;
    private String temp_m = "";
    private int readlength = -1;
    private boolean published = true;

    public ServerMessaging(String ip, int port, Activity act)
    {
        this.appcontext = act.getApplicationContext();
        this.ip = ip;
        this.port = port;
        lockit = new ReentrantLock(true);
        q = new ArrayDeque<String>();
        qw = new ArrayDeque<String>();
    }

    public Integer doInBackground(Void... params)
    {
        retcode = 0;
        if(InternetConnected())
        {
            if(Connect())
            {
                while (InternetConnected() && (listen || !qw.isEmpty()))
                {
                    lockit.lock();
                    try
                    {
                        if(temp_m.length() == readlength)
                        {
                            q.addLast(temp_m);
                            readlength = -1;
                            temp_m = "";
                        }
                        if (in.available() > 0)
                        {
                            Log.d("doInBackground", "read");
                            temp_m += String.valueOf((char)in.readUnsignedByte());
                            if(temp_m.length() == 4 && readlength == -1) {
                                readlength = Integer.parseInt(temp_m.trim()) + 5 * 4;
                                Log.d("doInBackground", String.valueOf(temp_m.length()));
                            }
                        }
                        if(message == null && !qw.isEmpty())
                            message = qw.removeFirst();
                        if(listened == null && !q.isEmpty())
                            listened = q.removeFirst();
                        if ((message != null || listened != null) && published)
                        {
                            published = false;
                            if(message != null)
                            send(message);
                            publishProgress();
                        }
                    }
                    catch (Exception e)
                    {
                        retcode = -1;
                        Log.d("doInBackground", "Error");
                        e.printStackTrace();
                        break;
                    }
                    finally
                    {
                        lockit.unlock();
                    }
                }
                if(listen == false)
                retcode = 1;
                else retcode = -1;
                Close();
            }
            else retcode = 2;
        }
        return retcode;
    }

    protected void onProgressUpdate(Void... progress)
    {
        String temp = null;
        if(listened != null)
        Log.d("progress", "listened not null");
        lockit.lock();
        Log.d("progress", "updating");
        if(message != null)
            message = null;
        if(listened != null && listen)
        {
            temp = listened;
            listened = null;
        }
        published = true;
        lockit.unlock();
        if(temp != null)
        AppMain.decodeMessage(temp);
    }

    protected void onPostExecute(Integer result)
    {
        if(retcode != 1)
        {
            switch(retcode)
            {
                case 0:
                    AppMain.warning("No internet connection");
                    break;
                case -1:
                    AppMain.warning("Disconnected");
                    break;
                default:
                    AppMain.warning("Error connecting");
                    break;
            }
        }
        AppMain.logout();
    }

    private boolean InternetConnected()
    {
        ConnectivityManager connMgr = (ConnectivityManager)appcontext.getSystemService(appcontext.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
        if (networkInfo != null && networkInfo.isConnected())
            return true;
        else return false;
    }

    private boolean Connect()
    {
        try
        {
            Log.d(Integer.toString(port), ip);
            socket = new Socket();
            SocketAddress sock = new InetSocketAddress(ip, port);
            socket.connect(sock, 500);
            out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));
            in = new DataInputStream(socket.getInputStream());
            Log.d("Connect", "connected");
            return true;
        }
        catch(Exception e)
        {
            e.printStackTrace();
            Log.d("Connect", "disconnected");
            Close();
            return false;
        }
    }

    private void Close()
    {
        Log.d("Close()", "closing");
        try
        {
            out.close();
        }
        catch(Exception e)
        {
            //stream null
        }
        try
        {
            in.close();
        }
        catch(Exception ex)
        {
            //stream null
        }
        try
        {
            socket.close();
        }
        catch(Exception ex)
        {
            //socket null
        }
    }

    private String send(String message) {
        try
        {
            Log.d("send", "sending");
            out.write(message);
            out.flush();
            return "Sent";
        }
        catch(Exception e)
        {
            //e.printStackTrace();
            return "Error";
        }
    }

    public void setMessage(String message)
    {
        Log.d("setMessage", "before lock");
        lockit.lock();
        Log.d("setMessage", "After lock");
        qw.addLast(message);
        lockit.unlock();
        Log.d("setMessage", "Past");
    }

}
