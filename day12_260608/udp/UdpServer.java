package udp;

import java.net.DatagramSocket;
import java.net.DatagramPacket;

public class UdpServer {
    public static void main (String[] args) throws Exception {
        DatagramSocket socket = new DatagramSocket(9000);
        byte[] buf = new byte[1024];

        System.out.println("Waiting UDP Server...");
        while (true) {
            // Rx
            DatagramPacket recvPacket = new DatagramPacket(buf, buf.length);
            socket.receive(recvPacket);
            String msg = new String(
                    recvPacket.getData(), 
                    0, 
                    recvPacket.getLength(), 
                    "UTF-8"
            );
            System.out.println("Recieve: "+msg);

            // Tx
            String resp = "Server has recieved: "+msg;
            byte[] sendBuf = resp.getBytes("UTF-8");
            DatagramPacket sendPacket = new DatagramPacket(
                    sendBuf,
                    sendBuf.length,
                    recvPacket.getAddress(),
                    recvPacket.getPort()
            );
            socket.send(sendPacket);

        }
    }
}
