import java.io.*;
import java.net.*;
import java.util.*;

public class ClientEx {
    public static void main(String[] args) {
        Socket socket = null;
        
        BufferedReader in = null;
        BufferedWriter out = null;

        Scanner scanner = new Scanner(System.in);
        
        try {
            socket = new Socket("localhost",5550);

            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            while (true) {
                System.out.print("[Tx]>> ");
                String outputMessage = scanner.nextLine();
                if (outputMessage.equalsIgnoreCase("bye")) {
                    out.write(outputMessage+"\n");
                    out.flush();
                    break;
                }
                out.write(outputMessage + "\n");
                out.flush();

                String inputMessage = in.readLine();
                System.out.println("[Rx]: "+inputMessage);
            }
        } catch (IOException e) {
            System.out.println(e.getMessage());
        } finally {
            try {
                scanner.close();
                if (socket != null) {
                    socket.close();
                }
            } catch(IOException e) {
                System.out.println("Error occured while chatting with server.");
            }
        }
    }
}
