import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.URI;
import java.net.URL;
import java.net.URLConnection;

public class URLConnectionExample01 {
    public static void main(String[] args) {
        try {
            URL url = new URI("http://www.google.com").toURL();
            URLConnection conn = url.openConnection();
            conn.setConnectTimeout(5000);

            InputStream is = conn.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(is));
            String line = "";
            while((line = reader.readLine())!=null) {
                System.out.println(line);
            }
            reader.close();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
}
