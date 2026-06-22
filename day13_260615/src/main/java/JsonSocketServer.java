//package json;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import org.json.JSONObject;

public class JsonSocketServer {
    public static void main(String[] args) throws Exception {
        ServerSocket serverSocket = new ServerSocket(6000);
        System.out.println("Waiting Server...");

        try (Socket clientSocket = serverSocket.accept()) {
            BufferedReader in = new BufferedReader(
                new InputStreamReader(clientSocket.getInputStream()));
            BufferedWriter out = new BufferedWriter(
                new OutputStreamWriter(clientSocket.getOutputStream()));

            String jsonStr = in.readLine();
            JSONObject recieved = new JSONObject(jsonStr);
            System.out.println("[Server] Rx: " + recieved.toString(4));

            JSONObject response = new JSONObject();
            response.put("result","OK");
            response.put("echo_name", recieved.getString("name"));

            out.write(response.toString());
            out.newLine();
            out.flush();
        }   
        serverSocket.close();
    }
}
