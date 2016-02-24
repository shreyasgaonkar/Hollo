package nocomp.hollo;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;

public class Login extends Activity
{

    private EditText address;
    private EditText port;
    private EditText user;
    private EditText password;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        AppMain.changeState(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void startConnection(View v)
    {
        address = (EditText)findViewById(R.id.address);
        port = (EditText)findViewById(R.id.port);
        try
        {
            String add = address.getText().toString();
            Log.d("mainmenu", port.getText().toString());
            int p = Integer.parseInt(port.getText().toString());
            Log.d("mainmenu", "Parsed Address");
            AppMain.start(add, p);
        }
        catch(Exception e)
        {
            AppMain.start("bertvm.cs.uic.edu", 8000);
            Log.d("mainmenu", "Parse error");
        }
    }

    public void login(View v)
    {
        user = (EditText)findViewById(R.id.user);
        password = (EditText)findViewById(R.id.password);
        AppMain.login(user.getText().toString(), password.getText().toString());
    }

    public void createAcc(View v)
    {
        user = (EditText)findViewById(R.id.user);
        password = (EditText)findViewById(R.id.password);
        AppMain.create(user.getText().toString(), password.getText().toString());
    }

    @Override
    public void onResume()
    {
        super.onResume();
        AppMain.changeState(this);
        AppMain.logout();
    }
}
