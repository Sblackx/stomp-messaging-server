package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {
    // Refactoring Messaging Protocl
    //private final MessagingProtocol<T> protocol;
    private final StompMessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, StompMessagingProtocol<T> protocol) { // Refactoring Messaging Protocol
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { //just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null) {
                    protocol.process(nextMessage); // The StompMessagingProtocol untilizes Connections class to send responses to Clients
                    // T response = protocol.process(nextMessage);
                    // if (response != null) {
                    //     out.write(encdec.encode(response));
                    //     out.flush();
                    // }
                }
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }

    }

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    @Override
    public void send(T msg) {
        // The idea is to Encode the message and write it in the socket's buffered output stream, said output stream needs to be thread-safe.
        if (msg == null) return;
        if (!connected) return;

        BufferedOutputStream localOut = this.out;
        if (localOut == null) return;

        try {
            byte[] bytes = encdec.encode(msg);
            synchronized (localOut) { 
                localOut.write(bytes);
                localOut.flush();
            }
        } catch (IOException e) {
            connected = false;
            try {
                close();
            } catch (IOException ignored) {}
        }
    }
}