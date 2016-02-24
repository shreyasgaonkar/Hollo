package nocomp.hollo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;


public class MainActivity extends Activity
{
    private Button click;
    private EditText message;
    private EditText status;
    private EditText chat;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        AppMain.changeState(this);
        message = (EditText) findViewById(R.id.message);
        chat = (EditText) findViewById(R.id.chatbox);
        AppMain.chat = chat;
        this.setTitle("Chatting with " + AppMain.getContactName());
    }

    public void sendMessage(View view)
    {
        AppMain.sendMessage(message.getText().toString());
    }

    @Override
    public void onBackPressed()
    {
        AppMain.finish();
    }
}
