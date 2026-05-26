package bgu.spl.net.impl.stomp;

import java.util.*;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl<T> implements StompMessagingProtocol<T> {
    private Connections<T> conn;
    private int connectionId;
    private boolean shouldEnd;

    @Override
    public void start(int connectionId, Connections<T> connections) {// the type of the connection will be
                                                                     // ConnectionImpl<T>the class we implemented
        this.connectionId = connectionId;
        this.conn = connections;
        shouldEnd = false;
    }

    @Override
    public void process(T message) {

        String mess = ((String) (message)).replace("\u0000", "");
        // conn.send(connectionId, (T) mess);
        ;
        String[] info = mess.split("\n");
        String todo = "";

        HashMap<String, String> map = new HashMap<>();
        int j = 0;
        while (j < info.length) {
            String line = info[j].trim();
            j++;
            if (!line.isEmpty()) {
                todo = line.toUpperCase();
                break;
            }

        }
        int index = j;
        for (; index < info.length; index++) {
            String line = info[index].trim();
            if (line.isEmpty()) {
                index++;
                break;
            } else if (line.contains(":")) {
                String[] keyValue = info[index].split(":", 2);
                // String r = "len:" + keyValue.length + keyValue[0] + "\n";

                // conn.send(connectionId, (T) r);
                map.put(keyValue[0], keyValue[1]);
            }

        }

        StringBuilder msg = new StringBuilder();
        while (index < info.length) {

            msg.append(info[index]).append("\n");
            index++;
        }

        // note that every message from the client, should be replyed by (server reply)
        // the message will be included

        // need to handle errors + to make an error frame
        String reply = "";
        switch (todo) {

            case "CONNECT":
                // handle errors
                /*
                 * accept-version:1.2
                 * host:stomp.cs.bgu.ac.il
                 * login:meni
                 * passcode:films
                 */

                if (!map.containsKey("accept-version")) {
                    reply = "ERROR\n" +
                            "message: missing header\n" + "\nThe CONNECT frame is missing the 'accept-version' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);

                    disconnect();
                    return;

                } else if (!map.containsKey("host")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe CONNECT frame is missing the 'host' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                } else if (!map.containsKey("login")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe CONNECT frame is missing the 'login' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                } else if (!map.containsKey("passcode")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + 
                            "\nThe CONNECT frame is missing the 'passcode' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                }
                shouldEnd = false;
                int state = conn.connectUser(connectionId, map.get("login"), map.get("passcode"));

                if (state == -1) {
                    reply = "ERROR\n" +
                            "message:Wrong passcode\n" + "\nThe user submited wrong passcode!\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    // disconnect();
                    return;

                }
                if (state == -2) {
                    reply = "ERROR\n" +
                            "message:Already Connected\n" + "\nThe user aalready connected!\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    // disconnect();
                    return;

                }
                if (map.containsKey("receipt")) {
                    reply = "CONNECTED\n" + "version:1.2\n" + "receipt-id:" + map.get("receipt")
                            + "\n\n" + '\u0000';
                } else
                    reply = "CONNECTED\n" + "version:1.2\n" + "\n" + '\u0000';

                conn.send(connectionId, (T) reply);
                break;
            case "SUBSCRIBE":
                if (!map.containsKey("id")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe SUBSCRIBE frame is missing the 'id' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);

                    disconnect();
                    return;

                }
                if (!map.containsKey("destination")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe SUBSCRIBE frame is missing the 'destination' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);

                    disconnect();
                    return;

                }

                state = conn.subscribe(map.get("destination"), connectionId, Integer.parseInt(map.get("id")));

                if (state == 0 && map.containsKey("receipt")) {
                    reply = "RECEIPT\n" + "receipt-id:" + map.get("receipt") + "\n\n" + '\u0000';

                    conn.send(connectionId, (T) reply);
                } else if (state == -1) {
                    reply = "ERROR\n" +
                            "message:missing connect before!\n"
                            + "\nThe SUBSCRIBER doesnt have connected before\n" + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                } else {// state = -2
                    reply = "ERROR\n" +
                            "message:subscription id already used!\n"
                            + "\nthe client have same subscription id before!\n" + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                }

                break;
            case "UNSUBSCRIBE":
                if (!map.containsKey("id")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe UNSUBSCRIBE frame is missing the 'id' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                }

                state = conn.unSubscribe(connectionId, Integer.parseInt(map.get("id")));
                if (state == 0 && map.containsKey("receipt")) {
                    reply = "RECEIPT\n" + "receipt-id:" + map.get("receipt") + "\n\n" + '\u0000';

                    conn.send(connectionId, (T) reply);
                } else if (state == -1) {
                    reply = "ERROR\n" +
                            "message:missing connect before!\n"
                            + "\nThe SUBSCRIBER doesnt have connected before\n" + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                } else { // state = -2
                    reply = "ERROR\n" +
                            "message:subscription id doesnt exist!\n"
                            + "\nthe client doesnt subscribed with this id before!\n" + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                }

                break;
            case "DISCONNECT":
                if (!map.containsKey("receipt")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe DISCONNECT frame is missing the 'receipt' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                }

                reply = "RECEIPT\n" + "receipt-id:" + map.get("receipt") + "\n\n" + '\u0000';
                shouldEnd = true;
                conn.send(connectionId, (T) reply);
                disconnect();

                break;

            case "SEND":
                if (!map.containsKey("destination")) {
                    reply = "ERROR\n" +
                            "message:missing header\n" + "\nThe SEND frame is missing the 'destination' header\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;
                }
                String channel = map.get("destination");
                if (!conn.isChannelExist(channel)) {
                    String receipt = "";
                    if (map.containsKey("receipt")) {
                        receipt = "\nreceipt-id:" + map.get("receipt");
                    }
                    reply = "ERROR" +
                            receipt +
                            "\nmessage:The destination not exist\n" + "-----\n" + mess + "-----\n"
                            + "the destination does not exist!\n\n"
                            + '\u0000';
                    conn.send(connectionId, (T) reply);
                    disconnect();
                    return;

                }
                if (map.containsKey("receipt")) {
                    reply = "RECEIPT\n" + "receipt-id:" + map.get("receipt") + "\n\n" + '\u0000';
                    conn.send(connectionId, (T) reply);

                }
                System.out.println(msg.toString());
                conn.send(channel, (T) (msg.toString()));

                break;
            default:
                break;

        }

    }

    private void disconnect() {
        conn.disconnect(connectionId);
        shouldEnd = true;
    }

    @Override
    public boolean shouldTerminate() {
        return shouldEnd;
    }

}
