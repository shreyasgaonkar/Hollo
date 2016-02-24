package nocomp.hollo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;


public class Contacts extends Activity {

    static TextView username;
    static ArrayAdapter<Contact> adapter;
    static ListView lw;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_contacts);
        AppMain.changeState(this);
        username = (TextView) findViewById(R.id.user);
        lw = (ListView) findViewById(R.id.listView);
        lw.setChoiceMode(AbsListView.CHOICE_MODE_SINGLE);
        lw.setSelector(android.R.color.holo_blue_light);
        lw.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                Contact entry = (Contact) parent.getAdapter().getItem(position);
                AppMain.selectContact(entry.getUserID());
            }
        });
        username.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                PopupMenu popup = new PopupMenu(Contacts.this, username);
                popup.getMenuInflater()
                        .inflate(R.menu.popup_menu, popup.getMenu());
                popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item)
                    {
                       switch(item.getItemId())
                       {
                           case R.id.One: AppMain.changeStatus(1); return true;
                           case R.id.Two: AppMain.changeStatus(2); return true;
                           case R.id.Three: AppMain.changeStatus(4); return true;
                           case R.id.Four: AppMain.changeStatus(3); return true;
                           default: return true;
                       }
                    }
                }); popup.show();}
        });
        update(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_contacts, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        switch(id)
        {
            case R.id.Accept: AppMain.acceptFriend(); return true;
            case R.id.Remove: AppMain.removeFriend(); return true;
            case R.id.Search: this.inputBox(); return true;
            case R.id.action_settings: return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume()
    {
        super.onResume();
        AppMain.changeState(this);
    }

    @Override
    public void onBackPressed()
    {
        AppMain.logout();
    }

    public static void update(Activity act)
    {
        String title = AppMain.getUserName() + " - Status: " + AppMain.getStatus();
        username.setText(title);
        Log.d("Update", String.valueOf(AppMain.getUserContacts().size()));
        adapter = new ArrayAdapter<Contact>(act, android.R.layout.simple_list_item_1, AppMain.getUserContacts());
        lw.setAdapter(adapter);
    }

    public void chat(View v) { AppMain.setChat();}

    private void inputBox()
    {
        AlertDialog alertDialog = new AlertDialog.Builder(this).create();
        alertDialog.setTitle("Search");
        alertDialog.setMessage("Input user name");
        final EditText input = new EditText(this);
        input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        alertDialog.setView(input);
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "Cancel", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });
        alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                AppMain.searchFriend(input.getText().toString());
                dialog.dismiss();
            }
        });
        alertDialog.show();
    }

}
