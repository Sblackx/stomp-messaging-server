package bgu.spl.net.impl.stomp;
import java.nio.charset.StandardCharsets;
import java.util.*;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompMessageEncoderDecoder<T> implements MessageEncoderDecoder<T> {
    private final List<Byte> bytes = new LinkedList<>();

    @Override
    public T decodeNextByte(byte nextByte) {
        if (nextByte == '\u0000') {
            int len = bytes.size();
            byte[] byteToArray = new byte[len];

            for (int i = 0; i < len; i++) {
                byteToArray[i] = bytes.get(i);
            }
            String res = new String(byteToArray, StandardCharsets.UTF_8);
            bytes.clear();
            return (T) res;

        }
        bytes.add(nextByte);
        return null;

    }

    @Override
    public byte[] encode(T message) {
        String msg = (String) message;

        byte[] res = null;

        res = (msg + '\u0000').getBytes(StandardCharsets.UTF_8);

        return res;

    }

}
