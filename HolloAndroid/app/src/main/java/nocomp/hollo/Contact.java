package nocomp.hollo;

public class Contact
{
    private int userID;
    private int status;
    private String username;
    private boolean pending;

    public Contact(int userID, String username, int status, boolean pending)
    {
        this.userID = userID;
        this.status = status;
        this.username = username;
        this.pending = pending;
    }
    public void setStatus(int status) { this.status = status;}
    public int getStatus() {return status;}
    public int getUserID()
    {
        return userID;
    }
    public void setUserID(int userID)
    {
        this.userID = userID;
    }
    public boolean getType()
    {
        return pending;
    }
    public void setType(boolean pending)
    {
        this.pending = pending;
    }
    public void setUserName(String username) { this. username = username; }
    public String getUserName() { return username; }
    public String toString()
    {
        if(pending == false) {
            switch (status) {
                case 1:
                    return username + " - Online";
                case 2:
                    return username + " - Busy";
                case 3:
                    return username + " - Offline";
                case 4:
                    return username + " - Away";
                default:
                    return "Error";
            }
        }else return username + " - Pending";
    }
}
