package bgu.spl.net.srv;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReadWriteLock;
import bgu.spl.net.impl.data.Database;
import bgu.spl.net.impl.data.LoginStatus;

public class ConnectionsImpl<T> implements Connections<T> {
    // data structure to store <channels, list of users connectionId>
    private ConcurrentHashMap<Integer, Set<String>> conn2channels = new ConcurrentHashMap<>();// store the channels for
                                                                                              // every client
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> clientConnectionHand = new ConcurrentHashMap<>();// connection
                                                                                                              // handler

    private ConcurrentHashMap<Integer, ConcurrentHashMap<Integer, String>> connectionSubs = new ConcurrentHashMap<>();// no
                                                                                                                      // need
                                                                                                                      // for
                                                                                                                      // this

    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, Integer>> channel2Conn2Sub = new ConcurrentHashMap<>();// conn
                                                                                                                        // to
                                                                                                                        // channel,
                                                                                                                        // easy
                                                                                                                        // to
                                                                                                                        // send
    // send O(n) instead of
    // O(n*m) before

    // channel>>
    private AtomicInteger counter = new AtomicInteger(1);
    private Database UserDataBase = Database.getInstance();

    private ReadWriteLock lock = new java.util.concurrent.locks.ReentrantReadWriteLock();

    // subscribe to channel
    public int subscribe(String channel, int connectionId, int subId) {
        writeLock();
        try {
            if (!clientConnectionHand.containsKey(connectionId))
                // unsuccessful subscribe due not connecting
                return -1;
            if (connectionSubs.get(connectionId) != null && connectionSubs.get(connectionId).containsKey(subId)) {
                // unsuccessful subscribe due the subId already exist, already subscribed
                return -2;
            }
            channel2Conn2Sub.computeIfAbsent(channel, v -> new ConcurrentHashMap<>()).put(connectionId, subId);// save
                                                                                                               // them
                                                                                                               // as
                                                                                                               // pair
            connectionSubs.computeIfAbsent(connectionId, v -> new ConcurrentHashMap<>()).put(subId, channel);
            conn2channels.computeIfAbsent(connectionId, v -> new ConcurrentHashMap().newKeySet()).add(channel);

            return 0;// successful subscribe
        } finally {
            writeUnlock();
        }

    }

    // unsubscribe the user from the channel
    public int unSubscribe(int connectionId, int subId) {
        writeLock();
        try {
            if (!clientConnectionHand.containsKey(connectionId))
                return -1;
            ConcurrentHashMap<Integer, String> map = connectionSubs.get(connectionId);

            if (map.get(subId) == null || !map.containsKey(subId))
                return -2;// unsuccessful unsubscribe
            // O(n)->O(1) improved
            String channel = map.get(subId);
            map.remove(subId);

            if (!map.containsValue(channel)) {
                channel2Conn2Sub.get(channel).remove(connectionId);

            }
            if (channel2Conn2Sub.get(channel).isEmpty()) {
                channel2Conn2Sub.remove(channel);

            }

            return 0;// successful unsubscribe
        } finally {
            writeUnlock();
        }

    }

    // must be call it before the subscribe(..)
    public void connect(int connectionId, ConnectionHandler<T> handler) {
        // atomic the hash map will handle the cocurrency

        clientConnectionHand.put(connectionId, handler);

    }

    public int connectUser(int connectionId, String name, String pass) {
        LoginStatus state = UserDataBase.login(connectionId, name, pass);
        if (state == LoginStatus.WRONG_PASSWORD) {
            System.out.println("wrong pass");
            return -1;
        }

        else if (state == LoginStatus.ALREADY_LOGGED_IN)
            return -2;
        else {
            // ADDED_NEW_USER || LOGGED_IN_SUCCESSFULLY
            return 0;

        }

    }

    public boolean isChannelExist(String channel) {

        return channel2Conn2Sub.containsKey(channel);

    }

    // implement the interface
    @Override
    public boolean send(int connectionId, T msg) {

        if (!clientConnectionHand.containsKey(connectionId))
            return false;
        ConnectionHandler<T> handler = clientConnectionHand.get(connectionId);
        handler.send(msg);
        return true;

    }

    @Override
    public void send(String channel, T msg) {

        if (!channel2Conn2Sub.containsKey(channel)) {
            return;
        }
        // O(n*m)->O(n) improved

        ConcurrentHashMap<Integer, Integer> connTosub = channel2Conn2Sub.get(channel);
        HashMap<Integer, Integer> mapSanpShot;
        readLock();
        try {
            mapSanpShot = new HashMap<>(connTosub);
        } finally {
            readUnlock();
        }

        for (Map.Entry<Integer, Integer> element : mapSanpShot.entrySet()) {
            int connId = element.getKey();
            int subId = element.getValue();
            StringBuilder reply = new StringBuilder();

            reply.append("MESSAGE\n").append("subscription:" + subId + "\n")
                    .append("message-id:" + counter.getAndIncrement() + "\n")
                    .append("destination:" + channel + "\n").append(msg).append('\u0000').append("\n");
            send(connId, (T) (reply.toString()));

        }

    }

    @Override
    public void disconnect(int connectionId) {
        // 1. remove from clientConnectionHand4
        writeLock();
        try {
            UserDataBase.logout(connectionId);

            clientConnectionHand.remove(connectionId);
            // 2.remove from the channels
            Set<String> channelsToRemove = conn2channels.get(connectionId);

            if (channelsToRemove != null) {
                for (String channel : channelsToRemove) {
                    ConcurrentHashMap<Integer, Integer> map = channel2Conn2Sub.get(channel);
                    if (map != null) {
                        map.remove(connectionId);
                        if (map.isEmpty()) {
                            channel2Conn2Sub.remove(channel);

                        }

                    }

                }
            }

            conn2channels.remove(connectionId);
            connectionSubs.remove(connectionId);
            if (clientConnectionHand.containsKey(connectionId))
                clientConnectionHand.remove(connectionId);

        } finally {
            writeUnlock();
        }

    }


    private void writeLock() {
        // TODO: acquire write lock
        lock.writeLock().lock();

    }

    private void writeUnlock() {
        // TODO: release write lock
        lock.writeLock().unlock();
    }

    private void readLock() {
        // TODO: acquire read lock
        lock.readLock().lock();
    }

    private void readUnlock() {
        // TODO: release read lock
        lock.readLock().unlock();
    }

}
