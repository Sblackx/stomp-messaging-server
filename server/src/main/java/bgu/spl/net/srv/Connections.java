package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    public int subscribe(String channel, int connectionId, int subId);
    public int unSubscribe(int connectionId, int subId);
    public void connect(int connectionId, ConnectionHandler<T> handler);
    public boolean isChannelExist(String channel);
     public int connectUser(int connectionId, String name, String pass) ;
     
}
