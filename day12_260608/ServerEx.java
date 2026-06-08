import java.io.*;
import java.net.*;
import java.util.*;

public class ServerEx {
    public static void main(String[] args) {
        ServerSocket listener = null;
        Socket socket = null;
        Scanner scanner = new Scanner(System.in);

        BufferedReader in = null;
        BufferedWriter out = null;

        try {
            listener = new ServerSocket(5550);
            System.out.println("Waiting for connection...");

            socket = listener.accept();
            System.out.println("Connected.");
            
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            while (true) {
                String inputMessage = in.readLine();
                if (inputMessage.equalsIgnoreCase("bye")) {
                    System.out.println("Client closed connection with bye");
                    break;
                }
                System.out.println("[Rx]: "+inputMessage);
                System.out.print("[Tx]>> ");

                String outputMessage = scanner.nextLine();
                out.write(outputMessage + "\n");
                out.flush();
            }
        } catch (IOException e) {
            System.out.println(e.getMessage());
        } finally {
            try {
                scanner.close();
                socket.close();
                listener.close();
            } catch (IOException e) {
                System.out.println("Error occured while chatting with client.");
            }
        }
    }
}
