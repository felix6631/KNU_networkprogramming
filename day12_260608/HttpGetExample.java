import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URISyntaxException;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.ProtocolException;


public class HttpGetExample {
    public static void main(String[] args) {
        URL url = null;
        try {
            url = new URI("https://jsonplaceholder.typicode.com/posts/1").toURL();
        } catch(URISyntaxException e) {
            e.printStackTrace();
        } catch(MalformedURLException e) {
            e.printStackTrace();
        }
        if (url == null) {
            return;
        }

        HttpURLConnection conn = null;
        try {
            conn = (HttpURLConnection) url.openConnection();
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (conn == null) {
            return;
        }
        try {
            conn.setRequestMethod("GET");
            conn.setConnectTimeout(5000);
            conn.setReadTimeout(5000);
        } catch(ProtocolException e) {
            e.printStackTrace();
        }

        int responseCode = 0;
        try {   
            responseCode = conn.getResponseCode();
        } catch (IOException e) {
            e.printStackTrace(); 
        }
        System.out.println("Response Code: "+responseCode);

        try (BufferedReader in = new BufferedReader(new InputStreamReader(conn.getInputStream()))) {
            String line;
            StringBuilder response = new StringBuilder();

            while((line = in.readLine()) != null) {
                response.append(line);
                System.out.println(line);
            }
            System.out.println("Response body: "+response);
        } catch (IOException e) {
            e.printStackTrace();
        }
        conn.disconnect();
        
    }
}
