package udp;

import java.net.DatagramSocket;
import java.net.DatagramPacket;
import java.net.InetAddress;

public class UdpClient {
    public static void main(String[] args) throws Exception {
        DatagramSocket socket = new DatagramSocket();
        InetAddress addr = InetAddress.getByName("127.0.0.1");
        int port = 9000;

        // Tx
        String msg = "Hello UDP Server";
        byte[] buf = msg.getBytes("UTF-8");
        DatagramPacket sendPacket = new DatagramPacket(buf, buf.length, addr, port);
        socket.send(sendPacket);

        // Rx
        byte[] recvBuf = new byte[1024];
        DatagramPacket recvPacket = new DatagramPacket(recvBuf, recvBuf.length);
        socket.receive(recvPacket);
        String response = new String(
                recvPacket.getData(),
                0,
                recvPacket.getLength(),
                "UTF-8"
        );
        System.out.println("Server response: "+response);
        socket.close();
    }
}

