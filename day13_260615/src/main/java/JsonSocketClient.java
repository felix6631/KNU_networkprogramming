//package json;

import java.io.*;
// import java.net.ServerSocket;
import java.net.Socket;
import org.json.JSONObject;

public class JsonSocketClient {
    public static void main(String args[]) {
        Socket socket;
        try {
            socket = new Socket("localhost",6000);
        } catch(Exception e) {
            e.printStackTrace();
            return;
        }
        try {
            BufferedWriter out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            JSONObject obj = new JSONObject();
            obj.put("name", "Felix Brown");
            obj.put("message", "JSON Message Testing... Say hello :)");

            out.write(obj.toString());
            out.newLine();
            out.flush();

            String respStr = in.readLine();
            JSONObject resp = new JSONObject();
            System.out.println("Reponse from server: " + resp.toString(4));

            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
