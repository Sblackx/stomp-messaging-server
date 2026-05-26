package bgu.spl.net.impl.stomp;

import bgu.spl.net.impl.data.Database;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            Database.getInstance().printReport();
        }));
        Server.threadPerClient(Integer.parseInt(args[0]), () -> new StompMessagingProtocolImpl<>(),
                () -> new StompMessageEncoderDecoder<>()).serve();
        // Server.reactor(Integer.parseInt(args[0]), Integer.parseInt(args[1]), () ->
        // new StompMessagingProtocolImpl<>(),
        // () -> new StompMessageEncoderDecoder<>()).serve();

        // users and their (login + logout History) + uploads file, it's summary of what
        // happen in server

    }
}
