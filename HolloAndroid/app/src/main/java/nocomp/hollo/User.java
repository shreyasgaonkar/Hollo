package nocomp.hollo;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class User extends Contact
{
    private ArrayList<Contact> contacts;
    private String password = null;
    public User(int userID, String name, String password)
    {
        super(userID, name, 1, false);
        this.password = password;
        contacts = new ArrayList<Contact>();
    }

    private class NameComparator implements Comparator<Contact>
    {
        public int compare(Contact a, Contact b)
        {
            return a.getUserName().compareToIgnoreCase(b.getUserName());
        }
    }

    public void addContact(Contact con)
    {
        contacts.add(con);
        Collections.sort(contacts, new NameComparator());
    }
    public void removeContact(int id)
    {
        for(Contact c : contacts)
        {
            if(c.getUserID() == id)
            {
                contacts.remove(c);
                break;
            }
        }
    }

    public void changeContactType(int id, boolean pending)
    {
        for(Contact c : contacts)
        {
            if(c.getUserID() == id)
            {
                c.setType(pending);
                break;
            }
        }
    }

    public Contact getContact(int id)
    {
        for(Contact c : contacts)
        {
            if(c.getUserID() == id)
                return c;
        }
        return null;
    }

    public void setContactStatus(int id, int status, String text)
    {
        for(Contact c : contacts)
        {
            if(c.getUserID() == id) {
                c.setStatus(status);
                return;
            }
        }
        this.addContact(new Contact(id, text, status, false));
    }

    public ArrayList<Contact> getUserContacts()
    {
        return contacts;
    }
}
